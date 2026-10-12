"""Drive the evaldiff harness (search_harness.cpp) over the 360-position bench and
the order_stats.py regression suite, in parallel shards, and sum the results.

The harness aborts on the first evaluation difference, so a shard that does not
finish, or prints MISMATCH, is a failure.

Usage: diff_driver.py EVALDIFF_EXE [--jobs N] [--depth-offset D] [--hash MB]
                      [--positions FILE.json] [--out DIR]
FILE.json holds [name, group, "startpos moves ..." or "fen ...", depth] entries,
like bench-games.json next to this script.
"""
import argparse
import importlib.util
import json
import re
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent


def positions(path):
    if path:
        return [tuple(p) for p in json.loads(Path(path).read_text())]
    bench = json.loads((HERE / 'bench-games.json').read_text())
    spec = importlib.util.spec_from_file_location('order_stats', HERE.parent / 'order_stats.py')
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return [tuple(p) for p in bench] + [tuple(p) for p in module.SUITE]


def run_shard(args, work):
    """One engine session over `work`; prints one line per position."""
    engine = subprocess.Popen([args.exe], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        engine.stdin.write(line + '\n')
        engine.stdin.flush()

    def wait_for(prefix):
        while True:
            line = engine.stdout.readline()
            if not line:
                raise SystemExit(f'engine exited (exit code {engine.wait()})')
            if line.startswith(prefix):
                return line

    send('uci'); wait_for('uciok')
    send(f'setoption name Hash value {args.hash}')
    send('setoption name OwnBook value false')
    for name, group, command, depth in work:
        send('ucinewgame')
        send(f'position {command}')
        send('isready'); wait_for('readyok')
        send(f'go depth {max(1, depth + args.depth_offset)}')
        print(name, wait_for('bestmove').strip(), flush=True)
    send('quit')
    engine.wait()
    print(f'DONE {len(work)} positions', flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('exe')
    parser.add_argument('--jobs', type=int, default=10, help='parallel shards (leave two cores idle)')
    parser.add_argument('--depth-offset', type=int, default=0)
    parser.add_argument('--hash', type=int, default=64)
    parser.add_argument('--positions', help='JSON position list instead of the bench and the regression suite')
    parser.add_argument('--out', default='eval-diff-run', help='directory for the shard logs')
    parser.add_argument('--shard', help=argparse.SUPPRESS)  # internal: K/N, run one shard in this process
    args = parser.parse_args()
    args.exe = str(Path(args.exe).resolve())
    work = positions(args.positions)
    if args.shard:
        k, n = map(int, args.shard.split('/'))
        run_shard(args, work[k - 1::n])
        return
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    procs = []
    for k in range(1, args.jobs + 1):
        command = [sys.executable, '-I', __file__, args.exe, '--shard', f'{k}/{args.jobs}', '--depth-offset',
                   str(args.depth_offset), '--hash', str(args.hash)]
        if args.positions:
            command += ['--positions', str(Path(args.positions).resolve())]
        procs.append(subprocess.Popen(command, stdout=open(out / f's{k}.out', 'w'), stderr=open(out / f's{k}.err', 'w')))
    for proc in procs:
        proc.wait()
    totals, failures, done = {}, [], 0
    for k in range(1, args.jobs + 1):
        text_out = (out / f's{k}.out').read_text()
        text_err = (out / f's{k}.err').read_text()
        finished = re.search(r'DONE (\d+) positions', text_out)
        done += int(finished.group(1)) if finished else 0
        if not finished or 'MISMATCH' in text_err or 'DIFF SUMMARY' not in text_err:
            failures.append(k)
        for line in text_err.splitlines():
            if line.startswith(('DIFF SUMMARY', 'COVERAGE')):
                for key, value in re.findall(r'(\w+)=(\d+)', line):
                    totals[key] = totals.get(key, 0) + int(value)
    print(f'{done}/{len(work)} positions, depth offset {args.depth_offset:+d}, {args.jobs} shards')
    print(' '.join(f'{key}={value:,}' for key, value in totals.items()))
    for k in failures:
        print(f'FAIL shard {k}: see {out / f"s{k}.err"}')
        for line in (out / f's{k}.err').read_text().splitlines():
            if line.startswith('MISMATCH'):
                print('  ' + line)
    print('PASS' if not failures and done == len(work) else 'FAIL')
    sys.exit(1 if failures else 0)


if __name__ == '__main__':
    main()
