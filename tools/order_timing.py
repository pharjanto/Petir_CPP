"""Times two builds on the order_stats.py suite at fixed depth: time to depth, nodes and speed.

Usage: order_timing.py BASE_EXE CANDIDATE_EXE [--depth-offset N] [--groups G,..] [--positions FILE]
                       [--shard K/N] [--trials N] [--core K] [--json OUT]

Each search runs in a fresh engine pinned to one core, and the two builds take
turns going first, so slow drift on the machine hits both alike. Run it on an
otherwise idle machine. Times are the engine's own figures for the search.
"""
import argparse
import ctypes
import json
import statistics
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from order_stats import SUITE, table  # noqa: E402


def search(exe, position, depth, core):
    proc = subprocess.Popen([str(exe)], cwd=Path(exe).parent, stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    if sys.platform == 'win32':
        api = ctypes.WinDLL('kernel32', use_last_error=True)
        api.SetProcessAffinityMask.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
        assert api.SetProcessAffinityMask(int(proc._handle), 1 << core)
    commands = ['uci', 'setoption name OwnBook value false', 'setoption name Hash value 16', 'ucinewgame',
                'position ' + position, 'isready', f'go depth {depth}']
    proc.stdin.write('\n'.join(commands) + '\n'); proc.stdin.flush()
    nodes = ms = None
    for line in proc.stdout:
        words = line.split()
        if words[:2] == ['info', 'nodes'] and 'time' in words:
            nodes, ms = int(words[2]), int(words[words.index('time') + 1])
        if words and words[0] == 'bestmove':
            break
    proc.stdin.write('quit\n'); proc.stdin.flush()
    proc.wait(timeout=10)
    assert nodes is not None, (exe, position)
    return nodes, ms


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('base', type=Path)
    parser.add_argument('candidate', type=Path)
    parser.add_argument('--depth-offset', type=int, default=0)
    parser.add_argument('--groups', help='groups to time (default: middlegame,tactical, or all of --positions)')
    parser.add_argument('--positions', type=Path, help='JSON suite in the order_stats.py --positions format')
    parser.add_argument('--trials', type=int, default=3)
    parser.add_argument('--core', type=int, default=4)
    parser.add_argument('--shard', default='1/1', help='K/N: time positions K, K+N, K+2N, ...')
    parser.add_argument('--json', type=Path, help='save nodes and trial times, for combining shards')
    args = parser.parse_args()
    source = json.loads(args.positions.read_text(encoding='utf-8')) if args.positions else SUITE
    if args.groups:
        groups = args.groups.split(',')
    else:
        groups = [g for _, g, _, _ in source] if args.positions else ['middlegame', 'tactical']
    suite = [(name, pos, depth + args.depth_offset) for name, group, pos, depth in source if group in groups]
    shard, shards = map(int, args.shard.split('/'))
    suite = suite[shard - 1::shards]
    builds = {'base': args.base, 'candidate': args.candidate}
    times = {b: [] for b in builds}
    nodes = {}
    for trial in range(args.trials):
        total = {b: 0 for b in builds}
        for i, (name, position, depth) in enumerate(suite):
            order = list(builds) if (trial + i) % 2 == 0 else list(reversed(builds))
            for b in order:
                n, ms = search(builds[b], position, depth, args.core)
                assert nodes.setdefault((b, name), n) == n, f'{b} {name}: node count changed between trials'
                total[b] += ms
        for b in builds:
            times[b].append(total[b])
        print(f'trial {trial + 1}: ' + ', '.join(f'{b} {total[b] / 1000:.2f} s' for b in builds), flush=True)
    rows = []
    for b in builds:
        n = sum(nodes[(b, name)] for name, _, _ in suite)
        t = statistics.median(times[b])
        rows.append([b, f'{n:,}', f'{t / 1000:.2f}', f'{n / t / 1000:.2f}M' if t else '-',
                     ', '.join(f'{x / 1000:.2f}' for x in times[b])])
    nb, nc = (sum(nodes[(b, name)] for name, _, _ in suite) for b in builds)
    tb, tc = (statistics.median(times[b]) for b in builds)
    print(f'\n{len(suite)} positions ({",".join(dict.fromkeys(groups))}), suite depths {args.depth_offset:+d}, '
          f'{args.trials} trials, median shown.\n')
    print(table(['Build', 'Nodes', 'Seconds', 'Nodes per second', 'Each trial (s)'], rows))
    print(f'\nCandidate against base: nodes {100 * (nc - nb) / nb:+.1f}%, time {100 * (tc - tb) / tb:+.1f}%, '
          f'speed {100 * ((nc / tc) / (nb / tb) - 1):+.1f}%.')
    if args.json:
        args.json.write_text(json.dumps({'positions': len(suite), 'times_ms': times,
                                         'nodes': {b: sum(nodes[(b, name)] for name, _, _ in suite) for b in builds}}),
                             encoding='utf-8')


if __name__ == '__main__':
    main()
