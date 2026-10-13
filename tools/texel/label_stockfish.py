"""Labels sampled positions with a UCI engine's search score (Stockfish).

Each position gets one search to --depth with a fresh hash (ucinewgame), one
thread, so the label does not depend on which worker ran it or in what order.
The final score of every completed depth is kept, from White's view: a run to
depth 16 also gives the depth-10, -12 and -14 labels, with the nodes and time
spent up to that depth. Single-threaded iterative deepening from a fresh hash
reaches the same depth-d result as a separate search to depth d (check it with
a --depth d run on a few positions).

Results are appended as JSON lines while the run goes, so an interrupted run
continues where it stopped when started again with the same OUT:
  {"fen", "result", "game", "ply", ..., "scores": {"10": {"cp"|"mate", "nodes", "ms"}, ..}}

--jobs N runs N engines in parallel; with --pin K, worker J is pinned to
logical core K + 2J (one per physical core with two threads per core).

Usage: label_stockfish.py ENGINE POSITIONS OUT --depth D [--jobs N] [--pin K]
                          [--hash MB] [--limit N]
"""
import argparse
import json
import queue
import sys
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from epd_test import Engine  # noqa: E402


def search_scores(engine, fen, depth, timeout):
    """Returns {depth: {'cp' or 'mate': score, 'nodes', 'ms'}} from the side to move's view."""
    engine.send('ucinewgame')
    engine.send('position fen ' + fen)
    engine.send('isready')
    while engine.get(60) != 'readyok':
        pass
    engine.send(f'go depth {depth}')
    scores = {}
    while True:
        words = engine.get(timeout).split()
        if words[:1] == ['bestmove']:
            return scores
        if words[:1] != ['info'] or 'score' not in words or 'depth' not in words or 'string' in words:
            continue
        value = lambda key: words[words.index(key) + 1] if key in words else None
        if value('multipv') not in (None, '1'):
            continue
        at = words.index('score')
        entry = {words[at + 1]: int(words[at + 2]), 'nodes': int(value('nodes') or 0), 'ms': int(value('time') or 0)}
        if at + 3 < len(words) and words[at + 3] in ('lowerbound', 'upperbound'):
            entry['bound'] = words[at + 3]
        # The exact line of a depth comes after its bound lines, so the last one wins.
        scores[int(value('depth'))] = entry


def white_view(scores, white_to_move):
    if white_to_move:
        return scores
    flipped = {}
    for depth, entry in scores.items():
        entry = dict(entry)
        for kind in ('cp', 'mate'):
            if kind in entry:
                entry[kind] = -entry[kind]
        flipped[depth] = entry
    return flipped


def worker(number, args, todo, done):
    core = None if args.pin is None else args.pin + 2 * number
    engine = Engine(args.engine, args.hash, {'Threads': '1'}, core)
    try:
        while True:
            try:
                row = todo.get_nowait()
            except queue.Empty:
                return
            scores = search_scores(engine, row['fen'], args.depth, args.timeout)
            row['scores'] = {str(depth): entry
                             for depth, entry in sorted(white_view(scores, row['fen'].split()[1] == 'w').items())}
            row['labelDepth'] = args.depth
            done.put(row)
    except Exception as error:  # report and let the other workers finish
        done.put({'error': f'worker {number}: {error!r}'})
    finally:
        engine.close()


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('engine')
    parser.add_argument('positions')
    parser.add_argument('out')
    parser.add_argument('--depth', type=int, required=True)
    parser.add_argument('--jobs', type=int, default=1)
    parser.add_argument('--pin', type=int, help='pin worker J to logical core PIN + 2J')
    parser.add_argument('--hash', type=int, default=16)
    parser.add_argument('--limit', type=int, help='label only the first N positions')
    parser.add_argument('--timeout', type=float, default=600, help='seconds without engine output before giving up')
    args = parser.parse_args()

    out = Path(args.out)
    finished = set()
    if out.exists():
        with open(out, encoding='utf-8') as rows:
            finished = {json.loads(line)['fen'] for line in rows if line.strip()}
    with open(args.positions, encoding='utf-8') as rows:
        positions = [json.loads(line) for line in rows if line.strip()]
    if args.limit:
        positions = positions[:args.limit]
    todo = queue.Queue()
    for row in positions:
        if row['fen'] not in finished:
            todo.put(row)
    total = todo.qsize()
    print(f'{len(positions):,} positions, {len(positions) - total:,} already labelled, {total:,} to go, '
          f'depth {args.depth}, {args.jobs} jobs', flush=True)

    done = queue.Queue()
    threads = [threading.Thread(target=worker, args=(number, args, todo, done), daemon=True)
               for number in range(args.jobs)]
    for thread in threads:
        thread.start()
    start, written, errors = time.monotonic(), 0, 0
    with open(out, 'a', encoding='utf-8', newline='\n') as sink:
        while written + errors < total and any(thread.is_alive() for thread in threads) or not done.empty():
            try:
                row = done.get(timeout=1)
            except queue.Empty:
                continue
            if 'error' in row:
                errors += 1
                print(row['error'], file=sys.stderr, flush=True)
                continue
            sink.write(json.dumps(row) + '\n')
            sink.flush()
            written += 1
            if written % 100 == 0 or written == total:
                rate = written / max(1e-9, time.monotonic() - start)
                print(f'  {written:,}/{total:,} labelled, {rate * 3600:,.0f} positions/hour', flush=True)
    print(f'done: {written:,} labelled, {errors} worker errors, {time.monotonic() - start:,.0f} s')
    return 1 if errors or written < total else 0


if __name__ == '__main__':
    sys.exit(main())
