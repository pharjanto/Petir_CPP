"""Runs lazystat over a position list and writes the run directory for analyze.

Game-like (default): for each position, ucinewgame, search the position WARMUP
plies earlier, then the position itself without ucinewgame (as order_stats.py
--warmup does). With LAZYSTAT_GAMELIKE=1 the harness counts only the second
search. --fresh: ucinewgame and one search per position, all counted.

Usage: lazy_driver.py EXE POSITIONS.json OUT_DIR [--jobs N] [--depth-offset D] [--warmup W] [--sample K] [--fresh]
POSITIONS.json is a list of [name, group, position command, depth], for example
tools/eval_diff/bench-games.json.
"""
import argparse, json, os, subprocess, sys
from pathlib import Path


def shard(exe, work, offset, warmup):
    engine = subprocess.Popen([exe], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True, bufsize=1)

    def send(line):
        engine.stdin.write(line + '\n'); engine.stdin.flush()

    def wait_for(prefix):
        while True:
            line = engine.stdout.readline()
            if not line:
                raise SystemExit(f'engine exited ({engine.wait()})')
            if line.startswith(prefix):
                return line

    send('uci'); wait_for('uciok')
    send('setoption name Hash value 16'); send('setoption name OwnBook value false')
    for name, group, command, depth in work:
        depth = max(1, depth + offset)
        moves = command.split()
        send('ucinewgame')
        if warmup:
            send('position ' + ' '.join(moves[:len(moves) - warmup]))
            send('isready'); wait_for('readyok')
            send(f'go depth {depth}'); wait_for('bestmove')
        send('position ' + command)
        send('isready'); wait_for('readyok')
        send(f'go depth {depth}')
        print(name, wait_for('bestmove').strip(), flush=True)
    send('quit'); engine.wait()
    print(f'DONE {len(work)}', flush=True)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('exe'); ap.add_argument('positions'); ap.add_argument('out')
    ap.add_argument('--jobs', type=int, default=10); ap.add_argument('--depth-offset', type=int, default=2)
    ap.add_argument('--warmup', type=int, default=2); ap.add_argument('--sample', default='4')
    ap.add_argument('--fresh', action='store_true'); ap.add_argument('--shard')
    a = ap.parse_args()
    if a.fresh:
        a.warmup = 0
    work = [tuple(p) for p in json.loads(Path(a.positions).read_text())]
    if a.shard:
        k, n = map(int, a.shard.split('/'))
        return shard(a.exe, work[k - 1::n], a.depth_offset, a.warmup)
    out = Path(a.out).resolve(); out.mkdir(parents=True, exist_ok=True)
    env = dict(os.environ, LAZYSTAT_OUT=str(out), LAZYSTAT_SAMPLE=a.sample)
    env.pop('LAZYSTAT_GAMELIKE', None)
    if not a.fresh:
        env['LAZYSTAT_GAMELIKE'] = '1'
    procs = [subprocess.Popen([sys.executable, '-I', __file__, a.exe, a.positions, a.out, '--shard', f'{k}/{a.jobs}',
                               '--depth-offset', str(a.depth_offset), '--warmup', str(a.warmup)],
                              env=env, stdout=open(out / f's{k}.out', 'w'), stderr=subprocess.STDOUT)
             for k in range(1, a.jobs + 1)]
    done = 0
    for k, p in enumerate(procs, 1):
        p.wait()
        text = (out / f's{k}.out').read_text()
        done += int(text.split('DONE ')[1]) if 'DONE ' in text else 0
    mode = 'fresh' if a.fresh else f'game-like warmup {a.warmup}'
    print(f'{done}/{len(work)} positions, {mode}, depth offset {a.depth_offset:+d}, {a.jobs} shards')


if __name__ == '__main__':
    main()
