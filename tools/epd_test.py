"""Runs UCI engines on EPD test suites and compares how many positions each solves.

A position is solved when the engine's final move is one of its "bm" moves and
none of its "am" moves. The time (and depth) to solution is when the engine's
principal variation started with a correct move and kept one until the end.

Suites are names from epd_suites.py (fetched into build/suites/NAME.json),
JSON files in that format, or .epd files (converted on the fly, which needs
python-chess; see epd_suites.py).

Several engines run on the same positions in the same worker, one after the
other, in alternating order, so each sees the same machine load. --jobs N runs N
workers in parallel; with --pin K, worker J is pinned to logical core K + 2J (one
per physical core on a machine with two threads per core). Under fixed time
(--movetime) the results depend on machine load: leave cores idle, and compare
engines from the same run. Fixed depth (--depth) is reproducible.

Usage: epd_test.py ENGINE [ENGINE ..] --suites S[,S..]
                   (--movetime MS | --depth N | --nodes N)
                   [--jobs N] [--pin K] [--hash MB] [--option NAME=VALUE ..]
                   [--limit N] [--timeout S] [--json OUT] [--report OUT]
ENGINE is an executable path, or LABEL=PATH.
"""
import argparse
import ctypes
import json
import queue
import subprocess
import sys
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import epd_suites  # noqa: E402


class Engine:
    def __init__(self, exe, hash_mb, options, core=None):
        self.proc = subprocess.Popen([str(exe)], cwd=Path(exe).parent, stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                     text=True, bufsize=1)
        if core is not None and sys.platform == 'win32':
            api = ctypes.WinDLL('kernel32', use_last_error=True)
            api.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
            api.SetProcessAffinityMask(int(self.proc._handle), 1 << core)
        self.lines = queue.Queue()
        threading.Thread(target=self._reader, daemon=True).start()
        self.send('uci')
        names = set()
        while (line := self.get(30)) != 'uciok':
            if line.startswith('option name '):
                names.add(line[len('option name '):].split(' type ')[0].strip())
        settings = {'Hash': str(hash_mb)}
        if 'OwnBook' in names:
            settings['OwnBook'] = 'false'
        settings.update(options)
        for name, value in settings.items():
            if name in names:
                self.send(f'setoption name {name} value {value}')

    def _reader(self):
        for line in self.proc.stdout:
            self.lines.put(line.rstrip())
        self.lines.put(None)

    def send(self, command):
        self.proc.stdin.write(command + '\n'); self.proc.stdin.flush()

    def get(self, timeout):
        line = self.lines.get(timeout=timeout)
        if line is None:
            raise RuntimeError('engine exited')
        return line

    def search(self, fen, go, timeout):
        """Returns the bestmove and every principal variation's (time ms, depth, first move)."""
        self.send('ucinewgame')
        self.send('position fen ' + fen)
        self.send('isready')
        while self.get(60) != 'readyok':
            pass
        start = time.monotonic()
        self.send('go ' + go)
        deadline, stopped, pvs, last = start + timeout, False, [], {}
        while True:
            try:
                line = self.get(max(0.001, deadline - time.monotonic()))
            except queue.Empty:
                if stopped:
                    raise RuntimeError('no bestmove after stop')
                self.send('stop'); stopped, deadline = True, time.monotonic() + 10
                continue
            words = line.split()
            if words[:1] == ['info'] and 'pv' in words and 'string' not in words:
                value = lambda key: words[words.index(key) + 1] if key in words else None
                ms = int(value('time')) if value('time') else int(1000 * (time.monotonic() - start))
                move = words[words.index('pv') + 1] if words.index('pv') + 1 < len(words) else None
                if move:
                    pvs.append((ms, int(value('depth') or 0), move))
                last = {key: value(key) for key in ('depth', 'nodes')}
                if 'score' in words:
                    last['score'] = ' '.join(words[words.index('score') + 1:words.index('score') + 3])
            elif words[:1] == ['bestmove']:
                elapsed = int(1000 * (time.monotonic() - start))
                return words[1] if len(words) > 1 else '', pvs, last, elapsed

    def close(self):
        try:
            self.send('quit'); self.proc.wait(timeout=5)
        except Exception:
            self.proc.kill()


def correct(move, position):
    return (not position['bm'] or move in position['bm']) and move not in position['am']


def judge(position, bestmove, pvs, last, elapsed):
    solved = correct(bestmove, position)
    solve_ms = solve_depth = None
    if solved:
        # Start of the final run of principal variations with a correct first move.
        first = len(pvs)
        while first > 0 and correct(pvs[first - 1][2], position):
            first -= 1
        solve_ms, solve_depth = (pvs[first][0], pvs[first][1]) if first < len(pvs) else (elapsed, None)
    return {'move': bestmove, 'solved': solved, 'solve_ms': solve_ms, 'solve_depth': solve_depth,
            'depth': int(last.get('depth') or 0), 'nodes': int(last.get('nodes') or 0),
            'score': last.get('score', ''), 'ms': elapsed}


def load_suite(spec, python_chess):
    path = Path(spec)
    if path.suffix.lower() == '.epd':
        positions, problems = epd_suites.convert(path.read_text(encoding='utf-8', errors='replace'),
                                                 epd_suites.import_chess(python_chess))
        for problem in problems:
            print(f'{spec}: {problem}', file=sys.stderr)
        return path.stem, positions
    if path.suffix.lower() != '.json':
        path = epd_suites.SUITES_DIR / f'{spec}.json'
    if not path.exists():
        sys.exit(f'{spec}: not found; fetch it with: epd_suites.py fetch {spec}')
    suite = json.loads(path.read_text(encoding='utf-8'))
    return suite['name'], suite['positions']


def report(results, labels, suites, settings):
    out = [f'# EPD test: {settings}', '',
           'Solved: final move is a "bm" move and no "am" move. Time and depth to solution: when the',
           'principal variation started with a correct move and kept one to the end. Mean time is over',
           'positions every engine solved, so the engines are compared on the same positions.', '',
           '| Suite | Positions | ' + ' | '.join(f'{label} solved' for label in labels)
           + ' | ' + ' | '.join(f'{label} mean time to solution' for label in labels) + ' |',
           '|---|---|' + '---|' * (2 * len(labels))]
    totals = {label: 0 for label in labels}
    differences = []
    for suite, positions in suites:
        rows = [results[(suite, index)] for index in range(len(positions))]
        solved = {label: sum(row[label]['solved'] for row in rows) for label in labels}
        common = [row for row in rows if all(row[label]['solved'] for label in labels)]
        mean = {label: (sum(row[label]['solve_ms'] for row in common) / len(common) / 1000 if common else 0)
                for label in labels}
        for label in labels:
            totals[label] += solved[label]
        out.append(f'| {suite} | {len(rows)} | ' + ' | '.join(str(solved[label]) for label in labels) + ' | '
                   + ' | '.join(f'{mean[label]:.2f} s ({len(common)})' for label in labels) + ' |')
        for index, row in enumerate(rows):
            if len({row[label]['solved'] for label in labels}) > 1:
                differences.append((suite, positions[index], row))
    count = sum(len(positions) for _, positions in suites)
    out.append(f'| **all** | {count} | ' + ' | '.join(f'**{totals[label]}**' for label in labels)
               + ' |' + ' |' * len(labels))
    if len(labels) > 1:
        out += ['', f'## Positions solved by some engines but not all ({len(differences)})', '',
                '| Suite | Id | Expected | ' + ' | '.join(labels) + ' |', '|---|---|---|' + '---|' * len(labels)]
        for suite, position, row in differences:
            expected = ' '.join(position['bm']) + (' (avoid ' + ' '.join(position['am']) + ')' if position['am'] else '')
            cells = []
            for label in labels:
                result = row[label]
                cells.append(f'{result["move"]} ok, {result["solve_ms"] / 1000:.2f} s d{result["solve_depth"]}'
                             if result['solved'] else f'{result["move"]} (d{result["depth"]}, {result["score"]})')
            out.append(f'| {suite} | {position["id"]} | {expected} | ' + ' | '.join(cells) + ' |')
    return '\n'.join(out) + '\n'


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('engines', nargs='+')
    parser.add_argument('--suites', required=True)
    limit = parser.add_mutually_exclusive_group(required=True)
    limit.add_argument('--movetime', type=int, help='milliseconds per position')
    limit.add_argument('--depth', type=int)
    limit.add_argument('--nodes', type=int)
    parser.add_argument('--jobs', type=int, default=1)
    parser.add_argument('--pin', type=int, help='pin worker J to logical core PIN + 2J')
    parser.add_argument('--hash', type=int, default=64)
    parser.add_argument('--option', action='append', default=[], help='NAME=VALUE, sent to every engine')
    parser.add_argument('--limit', type=int, help='first N positions of each suite')
    parser.add_argument('--timeout', type=float, help='seconds before "stop" (default: movetime x 3 + 10, else 600)')
    parser.add_argument('--python-chess', type=Path, default=epd_suites.ROOT / 'build' / 'kpk-audit' / 'python-chess-master')
    parser.add_argument('--json', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()

    engines = {}
    for spec in args.engines:
        label, _, path = spec.rpartition('=') if '=' in spec else (Path(spec).stem, '', spec)
        engines[label] = Path(path).resolve()
    labels = list(engines)
    options = dict(option.split('=', 1) for option in args.option)
    go = (f'movetime {args.movetime}' if args.movetime else f'depth {args.depth}' if args.depth else f'nodes {args.nodes}')
    timeout = args.timeout or (args.movetime * 3 / 1000 + 10 if args.movetime else 600)
    suites = [load_suite(spec, args.python_chess) for spec in args.suites.split(',')]
    if args.limit:
        suites = [(name, positions[:args.limit]) for name, positions in suites]

    work = queue.Queue()
    order = 0
    for suite, positions in suites:
        for index, position in enumerate(positions):
            work.put((order, suite, index, position)); order += 1
    results, lock, errors = {}, threading.Lock(), []
    total, done = order, [0]

    def worker(number):
        core = None if args.pin is None else args.pin + 2 * number
        try:
            running = {label: Engine(engines[label], args.hash, options, core) for label in labels}
        except Exception as error:
            errors.append(f'worker {number}: {error}'); return
        try:
            while True:
                try:
                    order, suite, index, position = work.get_nowait()
                except queue.Empty:
                    return
                row = {}
                turn = labels[order % len(labels):] + labels[:order % len(labels)]
                for label in turn:
                    try:
                        row[label] = judge(position, *running[label].search(position['fen'], go, timeout))
                    except Exception as error:
                        errors.append(f'{label} {suite} {position["id"]}: {error}')
                        running[label].close()
                        running[label] = Engine(engines[label], args.hash, options, core)
                        row[label] = {'move': '', 'solved': False, 'solve_ms': None, 'solve_depth': None,
                                      'depth': 0, 'nodes': 0, 'score': 'error', 'ms': 0}
                with lock:
                    results[(suite, index)] = row
                    done[0] += 1
                    if done[0] % 25 == 0 or done[0] == total:
                        print(f'{done[0]}/{total} positions', file=sys.stderr, flush=True)
        finally:
            for engine in running.values():
                engine.close()

    threads = [threading.Thread(target=worker, args=(number,)) for number in range(args.jobs)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()
    for error in errors:
        print('error: ' + error, file=sys.stderr)
    if len(results) != total:
        sys.exit(f'only {len(results)} of {total} positions finished')

    settings = f'{go}, hash {args.hash} MB, {args.jobs} jobs' + (f', pinned from core {args.pin}' if args.pin is not None else '')
    text = report(results, labels, suites, settings)
    print(text)
    if args.report:
        args.report.write_text(text, encoding='utf-8')
    if args.json:
        data = {'settings': settings, 'engines': {label: str(path) for label, path in engines.items()},
                'positions': [{'suite': suite, **position, 'results': results[(suite, index)]}
                              for suite, positions in suites for index, position in enumerate(positions)]}
        args.json.write_text(json.dumps(data, indent=1), encoding='utf-8')


if __name__ == '__main__':
    main()
