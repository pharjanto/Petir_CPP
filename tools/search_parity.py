"""Search parity: two engines must print identical UCI output, apart from time, nps
and hashfull, on every bench position. Use it for changes that must not alter the
search (speed-ups, refactors).

Each shard runs both engines over its positions in one session per engine, twice:
"fresh" sends ucinewgame before every position, "carry" keeps the hash between
positions. Prints PASS when every shard matches.

Usage: search_parity.py BASE_EXE CANDIDATE_EXE [--depth-offset 2] [--jobs 11]
                        [--positions FILE.json] [--hash 16]
Default positions: tools/eval_diff/bench-games.json (360 positions).
"""
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

DEFAULT_POSITIONS = Path(__file__).resolve().parent / 'eval_diff' / 'bench-games.json'
STRIP = re.compile(r' (time|nps|hashfull) \d+')


def run(exe, suite, offset, mode, hash_mb):
    proc = subprocess.Popen([exe], cwd=Path(exe).parent, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, text=True)
    out = []
    proc.stdin.write(f'uci\nsetoption name OwnBook value false\nsetoption name Hash value {hash_mb}\nucinewgame\nisready\n')
    proc.stdin.flush()
    for name, group, pos, depth in suite:
        if mode == 'fresh':
            proc.stdin.write('ucinewgame\n')
        proc.stdin.write(f'position {pos}\ngo depth {depth + offset}\n')
        proc.stdin.flush()
        for line in proc.stdout:
            if line.startswith(('info', 'bestmove')):
                out.append(name + ' ' + STRIP.sub('', line.strip()))
            if line.startswith('bestmove'):
                break
    proc.stdin.write('quit\n')
    proc.stdin.flush()
    proc.wait(timeout=10)
    return out


def shard(args):
    suite = json.loads(Path(args.positions).read_text())[args.shard - 1::args.jobs]
    a = run(args.base, suite, args.depth_offset, args.mode, args.hash)
    b = run(args.candidate, suite, args.depth_offset, args.mode, args.hash)
    diff = [(x, y) for x, y in zip(a, b) if x != y]
    ok = not diff and len(a) == len(b)
    print(f'shard {args.shard}/{args.jobs} {args.mode}: {len(suite)} positions, {len(a)}/{len(b)} lines, '
          f'{len(diff)} differ' + ('' if ok else ' FAIL'))
    for x, y in diff[:3]:
        print('  base:', x, '\n  cand:', y)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('base')
    parser.add_argument('candidate')
    parser.add_argument('--depth-offset', type=int, default=2)
    parser.add_argument('--jobs', type=int, default=11, help='parallel shards (leave two cores idle)')
    parser.add_argument('--positions', default=str(DEFAULT_POSITIONS))
    parser.add_argument('--hash', type=int, default=16)
    parser.add_argument('--shard', type=int, help=argparse.SUPPRESS)  # internal
    parser.add_argument('--mode', choices=['fresh', 'carry'], help=argparse.SUPPRESS)  # internal
    args = parser.parse_args()
    args.base, args.candidate = str(Path(args.base).resolve()), str(Path(args.candidate).resolve())
    if args.shard:
        shard(args)
        return
    lines = []
    for mode in ('fresh', 'carry'):
        procs = [subprocess.Popen([sys.executable, '-I', __file__, args.base, args.candidate, '--depth-offset',
                                   str(args.depth_offset), '--jobs', str(args.jobs), '--positions', args.positions,
                                   '--hash', str(args.hash), '--shard', str(k), '--mode', mode],
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
                 for k in range(1, args.jobs + 1)]
        for proc in procs:
            lines += proc.communicate()[0].splitlines()
    print('\n'.join(lines))
    passed = sum(1 for line in lines if line.startswith('shard') and line.endswith(' 0 differ'))
    print(f'{passed} of {2 * args.jobs} shards identical, depth offset {args.depth_offset:+d}')
    print('PASS' if passed == 2 * args.jobs else 'FAIL')
    sys.exit(0 if passed == 2 * args.jobs else 1)


if __name__ == '__main__':
    main()
