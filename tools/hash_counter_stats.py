"""Measure TT cutoffs blocked only by the halfmove counter, without relaxing it.

Requires a PETIR_HASH_STATS build. With --baseline, compares every ordinary UCI
info/bestmove line after removing timing fields. Bench positions use strided
sessions, as search_parity.py does. PGNs are replayed on the selected player's
turns, preserving the actual moves, counters, castling history and repetitions;
the replay searches to --game-depth, rather than reproducing its original clock.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import ctypes
import json
from pathlib import Path
import queue
import re
import subprocess
import sys
import threading
import time

STRIP_TIME = re.compile(r' (time|nps|hashfull) \d+')
DEFAULT_SUITE = Path(__file__).resolve().parent / 'eval_diff' / 'bench-games.json'


class Engine:
    def __init__(self, exe, hash_mb, core):
        self.proc = subprocess.Popen([str(exe)], cwd=exe.parent, stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                     text=True, bufsize=1)
        self.lines = queue.Queue()
        threading.Thread(target=self._reader, daemon=True).start()
        if sys.platform == 'win32':
            api = ctypes.WinDLL('kernel32', use_last_error=True)
            api.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
            if not api.SetProcessAffinityMask(int(self.proc._handle), 1 << core):
                self.close()
                raise ctypes.WinError(ctypes.get_last_error())
        self.send('uci')
        self.until(lambda line: line == 'uciok')
        self.send('setoption name OwnBook value false')
        self.send(f'setoption name Hash value {hash_mb}')
        self.send('ucinewgame')
        self.send('isready')
        self.until(lambda line: line == 'readyok')

    def _reader(self):
        for line in self.proc.stdout:
            self.lines.put(line.rstrip())
        self.lines.put(None)

    def send(self, command):
        self.proc.stdin.write(command + '\n')
        self.proc.stdin.flush()

    def until(self, wanted, timeout=900):
        deadline = time.monotonic() + timeout
        lines = []
        while True:
            line = self.lines.get(timeout=max(0.001, deadline - time.monotonic()))
            if line is None:
                raise RuntimeError(f'engine exited with {self.proc.poll()}')
            if line.startswith(('Error', 'info string')):
                raise RuntimeError(line)
            lines.append(line)
            if wanted(line):
                return lines

    def search(self, position, reset, stats):
        if reset:
            self.send('ucinewgame')
        self.send('position ' + position['position'])
        self.send('isready')
        self.until(lambda line: line == 'readyok')
        self.send(f'go depth {position["depth"]}')
        lines = self.until(lambda line: line.startswith('bestmove '))
        normal = [STRIP_TIME.sub('', line) for line in lines
                  if line.startswith(('info ', 'bestmove '))]
        node_line = next(line for line in reversed(lines) if line.startswith('info nodes '))
        words = node_line.split()
        result = {'nodes': int(words[2]), 'engine_ms': int(words[words.index('time') + 1]),
                  'bestmove': lines[-1].split()[1], 'uci': normal}
        if stats:
            self.send('hashstats')
            line = self.until(lambda line: line.startswith('hashstats '))[-1]
            result['stats'] = json.loads(line.removeprefix('hashstats '))
            validate(result['stats'])
        return result

    def close(self):
        try:
            if self.proc.poll() is None:
                self.send('quit')
                self.proc.wait(timeout=10)
        finally:
            if self.proc.poll() is None:
                self.proc.kill()
                self.proc.wait(timeout=5)
            for stream in (self.proc.stdin, self.proc.stdout):
                if stream:
                    stream.close()


def validate(c):
    assert c['decision_mismatches'] == 0, c
    assert c['key_hits'] == c['counter_matches'] + c['counter_mismatches'] <= c['probes'], c
    assert c['counter_mismatches'] == (c['depth_rejected_mismatches']
                                      + c['bound_rejected_mismatches']
                                      + c['counter_blocked_cutoffs']), c
    assert c['accepted_cutoffs'] <= c['counter_matches'], c
    for field in ('blocked_bands', 'blocked_differences', 'blocked_bounds', 'blocked_depths', 'blocked_ages'):
        assert sum(c[field]) == c['counter_blocked_cutoffs'], (field, c)
    assert c['blocked_bands'] == [sum(row) for row in c['blocked_bands_by_difference']], c
    assert sum(c['mismatch_bands']) == c['counter_mismatches'], c


def combine(a, b):
    if isinstance(a, dict):
        return {key: combine(a[key], b[key]) for key in a}
    if isinstance(a, list):
        return [combine(x, y) for x, y in zip(a, b)]
    return a + b


def total(rows):
    out = rows[0]['stats']
    for row in rows[1:]:
        out = combine(out, row['stats'])
    validate(out)
    return out


def pct(a, b):
    return f'{100 * a / b:.2f}%' if b else '-'


def table(headers, rows):
    return '\n'.join(['| ' + ' | '.join(headers) + ' |',
                       '|' + '|'.join('---' for _ in headers) + '|']
                      + ['| ' + ' | '.join(map(str, row)) + ' |' for row in rows])


def report(rows, metadata):
    out = ['# Halfmove-counter hash rejection measurement', '',
           f'Base: `{metadata["base_commit"]}`. Hash: **{metadata["hash_mb"]} MiB**. '
           f'Workers: {metadata["jobs"]}; each pinned to a separate physical core.', '',
           'The exact counter check remains enabled. Only non-PV main-search probes count below. '
           'Root and PV extraction probes are omitted; PV main-search probes are counted separately '
           'and excluded. A blocked cutoff has a matching key, sufficient stored depth, and an exact '
           'or alpha/beta-compatible bound. The counter is its only failed eligibility condition.', '',
           'Blocked share = blocked / (accepted + blocked). These are observed opportunities along '
           'the unchanged search tree, rather than a prediction of nodes, time, or Elo saved.', '']
    summary = []
    sets = list(dict.fromkeys((r['dataset'], r['mode']) for r in rows))
    for dataset, mode in sets:
        group = [r for r in rows if (r['dataset'], r['mode']) == (dataset, mode)]
        c = total(group)
        summary.append([dataset, mode, len(group), f'{c["probes"]:,}', f'{c["key_hits"]:,}',
                        f'{c["counter_mismatches"]:,}', f'{c["accepted_cutoffs"]:,}',
                        f'{c["counter_blocked_cutoffs"]:,}',
                        pct(c['counter_blocked_cutoffs'], c['accepted_cutoffs'] + c['counter_blocked_cutoffs'])])
    out += [table(['Dataset', 'Hash mode', 'Searches', 'Probes', 'Key hits', 'Counter mismatches',
                   'Accepted cutoffs', 'Counter-blocked cutoffs', 'Blocked share'], summary), '']
    for dataset, mode in sets:
        group = [r for r in rows if (r['dataset'], r['mode']) == (dataset, mode)]
        c = total(group)
        out += [f'## {dataset}, {mode}', '',
                f'- Nodes searched: {sum(r["nodes"] for r in group):,}.',
                f'- Key matches rejected by counter: {pct(c["counter_mismatches"], c["key_hits"])}.',
                f'- Counter mismatches also too shallow: {c["depth_rejected_mismatches"]:,}; '
                f'deep enough but non-cutting bound: {c["bound_rejected_mismatches"]:,}.',
                f'- Blocked cutoffs / all probes: {pct(c["counter_blocked_cutoffs"], c["probes"])}.',
                f'- PV main probes excluded: {c["pv_probes_excluded"]:,}; '
                f'classifier disagreements with the real probe: {c["decision_mismatches"]}.', '']
        out += [table(['Counter band', 'All mismatches', 'Blocked cutoffs', 'Share of blocked',
                       'Difference 1', 'Difference 2', 'Difference 3+'],
                      [[label, f'{c["mismatch_bands"][i]:,}', f'{c["blocked_bands"][i]:,}',
                        pct(c['blocked_bands'][i], c['counter_blocked_cutoffs']),
                        *[f'{x:,}' for x in c['blocked_bands_by_difference'][i]]]
                       for i, label in enumerate(['Both <80', 'Either 80-99, neither >=100', 'Either >=100'])]), '',
                table(['Bound', 'Blocked cutoffs'], [[label, f'{c["blocked_bounds"][i]:,}']
                       for i, label in enumerate(['Exact', 'Upper', 'Lower'])]), '',
                table(['Requested whole plies', 'Blocked cutoffs'], [[label, f'{c["blocked_depths"][i]:,}']
                       for i, label in enumerate(['0-1', '2', '3', '4-5', '6-8', '9+'])]), '',
                table(['Entry generation', 'Blocked cutoffs'], [[label, f'{c["blocked_ages"][i]:,}']
                       for i, label in enumerate(['Current search', 'Previous search', 'Older'])]), '']
        phase_rows = []
        for phase in dict.fromkeys(r['group'] for r in group):
            phase_group = [r for r in group if r['group'] == phase]
            p = total(phase_group)
            phase_rows.append([phase, len(phase_group), f'{p["accepted_cutoffs"]:,}',
                               f'{p["counter_blocked_cutoffs"]:,}',
                               pct(p['counter_blocked_cutoffs'], p['accepted_cutoffs'] + p['counter_blocked_cutoffs'])])
        out += [table(['Position phase', 'Searches', 'Accepted cutoffs', 'Blocked cutoffs', 'Blocked share'], phase_rows), '']
        out += [table(['Largest contributors', 'Blocked cutoffs', 'Share of this dataset', 'Blocked share'],
                      [[r['name'], f'{r["stats"]["counter_blocked_cutoffs"]:,}',
                        pct(r['stats']['counter_blocked_cutoffs'], c['counter_blocked_cutoffs']),
                        pct(r['stats']['counter_blocked_cutoffs'],
                            r['stats']['accepted_cutoffs'] + r['stats']['counter_blocked_cutoffs'])]
                       for r in sorted(group, key=lambda r: r['stats']['counter_blocked_cutoffs'], reverse=True)[:5]]), '']
    compared = [r for r in rows if 'baseline_identical' in r]
    out += ['## Validation and interpretation', '',
            f'- Identical baseline/instrumented searches: {sum(r["baseline_identical"] for r in compared):,}'
            f' / {len(compared):,}. Every ordinary UCI info/bestmove line is compared except timing fields.',
            '- Counts include all iterative-deepening iterations and aspiration/PVS re-searches.',
            '- Counter bands describe the stored/current node counters. Both below 80 does not '
            'guarantee descendants stay below the evaluation fade: extensions and quiescence can continue.',
            '- The diagnostic build adds counting overhead. Its timings are not a speed comparison.', '']
    if 'games' in metadata:
        g = metadata['games']
        out += [f'- Game sample: first {g["games"]} games from `{g["pgn"]}`, '
                f'player `{g["player"]}`, searching every own turn after {g["book_plies"]} opening plies '
                f'at fixed depth {g["depth"]}. Actual played moves are replayed regardless of the analysis move.',
                f'- Replay root halfmove counters: {g["min_counter"]}..{g["max_counter"]}. '
                'Each game starts a new session history; carried hash persists between the selected player\'s turns.', '']
    out += ['## Run metadata', '', '```json', json.dumps(metadata, indent=2), '```', '']
    return '\n'.join(out)


def load_games(args):
    if args.python_chess:
        sys.path.insert(0, str(args.python_chess.resolve()))
    import chess.pgn
    games = []
    with args.pgn.open(encoding='utf-8-sig', errors='replace') as stream:
        for number in range(1, args.games + 1):
            game = chess.pgn.read_game(stream)
            if game is None:
                break
            if game.errors:
                raise RuntimeError(f'PGN game {number}: {game.errors}')
            if game.headers.get('White') == args.player:
                color = chess.WHITE
            elif game.headers.get('Black') == args.player:
                color = chess.BLACK
            else:
                raise ValueError(f'game {number} does not contain {args.player}')
            board = game.board()
            prefix = 'startpos' if board.fen() == chess.STARTING_FEN else 'fen ' + board.fen()
            moves, positions = [], []
            for ply, move in enumerate(game.mainline_moves()):
                if ply >= args.book_plies and board.turn == color:
                    material = [sum(len(board.pieces(piece, side)) * value
                                    for piece, value in [(chess.KNIGHT, 3), (chess.BISHOP, 3),
                                                         (chess.ROOK, 5), (chess.QUEEN, 9)])
                                for side in (chess.WHITE, chess.BLACK)]
                    phase = 'endgame' if max(material) <= 12 else 'opening' if ply <= 32 else 'middlegame'
                    positions.append({'name': f'game-{number:03d}-ply-{ply:03d}', 'group': phase,
                                      'position': prefix + (' moves ' + ' '.join(moves) if moves else ''),
                                      'depth': args.game_depth, 'game': number, 'ply': ply,
                                      'halfmove_counter': board.halfmove_clock})
                moves.append(move.uci())
                board.push(move)
            if positions:
                games.append(positions)
    return games


def run_task(args, dataset, mode, shard, positions):
    engines = []
    core = args.first_core + 2 * shard
    rows = []
    try:
        engine = Engine(args.engine, args.hash, core)
        engines.append(engine)
        baseline = Engine(args.baseline, args.hash, core) if args.baseline else None
        if baseline:
            engines.append(baseline)
        previous_game = None
        for position in positions:
            reset = mode == 'fresh' or position.get('game') != previous_game
            base = baseline.search(position, reset, False) if baseline else None
            row = engine.search(position, reset, True)
            if base:
                row['baseline_identical'] = row['uci'] == base['uci']
                if not row['baseline_identical']:
                    raise AssertionError(f'{dataset}/{mode}/{position["name"]}: UCI differs\n'
                                         f'base={base["uci"]}\nstats={row["uci"]}')
                row['baseline_ms'] = base['engine_ms']
            row.update(position, dataset=dataset, mode=mode, shard=shard + 1)
            rows.append(row)
            previous_game = position.get('game')
    finally:
        for engine in engines:
            engine.close()
    output = args.out / f'{dataset}-{mode}-{shard + 1}.json'
    output.write_text(json.dumps(rows, indent=1) + '\n', encoding='utf-8')
    c = total(rows)
    print(f'{dataset} {mode} shard {shard + 1}/{args.jobs}: {len(rows)} searches, '
          f'{c["counter_blocked_cutoffs"]:,} blocked, '
          f'{pct(c["counter_blocked_cutoffs"], c["accepted_cutoffs"] + c["counter_blocked_cutoffs"])} share, '
          f'parity {"PASS" if baseline else "not checked"}', flush=True)
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('engine', type=Path)
    parser.add_argument('--baseline', type=Path)
    parser.add_argument('--base-commit', default='unknown')
    parser.add_argument('--positions', type=Path, default=DEFAULT_SUITE)
    parser.add_argument('--depth-offset', type=int, default=2)
    parser.add_argument('--modes', default='fresh,carry')
    parser.add_argument('--hash', type=int, default=64)
    parser.add_argument('--jobs', type=int, default=6)
    parser.add_argument('--first-core', type=int, default=2)
    parser.add_argument('--pgn', type=Path)
    parser.add_argument('--player', default='LC')
    parser.add_argument('--games', type=int, default=20)
    parser.add_argument('--game-depth', type=int, default=12)
    parser.add_argument('--book-plies', type=int, default=16)
    parser.add_argument('--python-chess', type=Path)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    if args.jobs < 1 or args.jobs > 10:
        parser.error('--jobs must be 1..10, leaving at least two physical cores idle')
    modes = args.modes.split(',')
    if any(mode not in ('fresh', 'carry') for mode in modes):
        parser.error('--modes must contain fresh and/or carry')
    args.engine = args.engine.resolve()
    if args.baseline:
        args.baseline = args.baseline.resolve()
    args.out.mkdir(parents=True, exist_ok=True)
    suite = [{'name': name, 'group': group, 'position': position, 'depth': depth + args.depth_offset}
             for name, group, position, depth in json.loads(args.positions.read_text(encoding='utf-8'))]
    metadata = {'base_commit': args.base_commit, 'engine': str(args.engine),
                'baseline': str(args.baseline) if args.baseline else None,
                'hash_mb': args.hash, 'jobs': args.jobs, 'first_core': args.first_core,
                'bench_positions': str(args.positions.resolve()), 'bench_depth_offset': args.depth_offset,
                'bench_depth_range': [min(p['depth'] for p in suite), max(p['depth'] for p in suite)],
                'modes': modes}
    tasks = [('bench', mode, shard, suite[shard::args.jobs])
             for mode in modes for shard in range(args.jobs) if suite[shard::args.jobs]]
    if args.pgn:
        games = load_games(args)
        positions = [p for game in games for p in game]
        if not positions:
            parser.error('no game positions selected')
        metadata['games'] = {'pgn': str(args.pgn.resolve()), 'games': len(games), 'player': args.player,
                             'book_plies': args.book_plies, 'depth': args.game_depth,
                             'positions': len(positions),
                             'min_counter': min(p['halfmove_counter'] for p in positions),
                             'max_counter': max(p['halfmove_counter'] for p in positions)}
        (args.out / 'game-positions.json').write_text(json.dumps(positions, indent=1) + '\n', encoding='utf-8')
        tasks += [('games', mode, shard, [p for game in games[shard::args.jobs] for p in game])
                  for mode in modes for shard in range(args.jobs) if games[shard::args.jobs]]
    print(json.dumps(metadata, indent=2), flush=True)
    rows = []
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(run_task, args, *task) for task in tasks]
        for future in as_completed(futures):
            rows += future.result()
    rows.sort(key=lambda r: (r['dataset'], r['mode'], r['name']))
    (args.out / 'results.json').write_text(json.dumps({'metadata': metadata, 'results': rows}, indent=1) + '\n', encoding='utf-8')
    text = report(rows, metadata)
    (args.out / 'REPORT.md').write_text(text.rstrip() + '\n', encoding='utf-8')
    print(text, flush=True)


if __name__ == '__main__':
    main()
