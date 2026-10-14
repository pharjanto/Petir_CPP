"""Records Petir's evaluation of labelled positions for the Texel fitter.

Reads a labelled set (label_stockfish.py output; a file still being written is
fine, its last incomplete line is skipped) and writes into OUTDIR:
  positions.tsv  one position per line: game, ply, result and Stockfish score
                 (both from White's view; the score in centipawns, or #N for
                 mate in N), FEN
  trace.txt      eval_trace's line for each position, in the same order: the
                 evaluation and the coefficient of each weight
  weights.tsv    the weights the trace was made at (eval_trace --list)
Positions that eval_trace cannot read are left out of both files.

--weights FILE traces at the values in FILE (a fit's output) instead of the
engine's own, for a second fit that starts from the first. --jobs N runs N
eval_trace processes on parts of the set.

Usage: trace_labels.py EVAL_TRACE LABELS OUTDIR [--depth D] [--jobs N]
                       [--weights FILE] [--step N]
"""
import argparse
import json
import subprocess
import sys
import time
from pathlib import Path


def labelled_rows(path, depth):
    """Yields (game, ply, result, score text, fen) for each complete row."""
    with open(path, encoding='utf-8') as rows:
        for line in rows:
            if not line.endswith('\n'):
                break
            row = json.loads(line)
            entry = row['scores'][str(depth or row['labelDepth'])]
            score = f"#{entry['mate']}" if 'mate' in entry else str(entry['cp'])
            yield row['game'], row['ply'], row['result'], score, row['fen']


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('eval_trace')
    parser.add_argument('labels')
    parser.add_argument('outdir')
    parser.add_argument('--depth', type=int, help='Stockfish depth to use (default: the label depth)')
    parser.add_argument('--jobs', type=int, default=1)
    parser.add_argument('--weights')
    parser.add_argument('--step', type=int, default=16)
    args = parser.parse_args()

    out = Path(args.outdir)
    out.mkdir(parents=True, exist_ok=True)
    rows = list(labelled_rows(args.labels, args.depth))
    jobs = max(1, min(args.jobs, len(rows)))
    exe = str(Path(args.eval_trace).resolve())
    extra = ['--weights', str(Path(args.weights).resolve())] if args.weights else []

    subprocess.run([exe, '--list', str(out / 'weights.tsv')] + extra, check=True)
    start = time.monotonic()
    parts = []
    for job in range(jobs):
        part = rows[job * len(rows) // jobs:(job + 1) * len(rows) // jobs]
        fens = out / f'part{job}.fen'
        fens.write_text(''.join(row[4] + '\n' for row in part), encoding='utf-8', newline='\n')
        trace = out / f'part{job}.trace'
        command = [exe, str(fens), str(trace), '--step', str(args.step)] + extra
        parts.append((part, fens, trace, subprocess.Popen(command, stderr=subprocess.DEVNULL)))
    failed = [job for job, (_, _, _, process) in enumerate(parts) if process.wait() != 0]
    if failed:
        sys.exit(f'eval_trace failed on part(s) {failed}')
    seconds = time.monotonic() - start

    kept = errors = 0
    with open(out / 'positions.tsv', 'w', encoding='utf-8', newline='\n') as positions, \
         open(out / 'trace.txt', 'w', encoding='utf-8', newline='\n') as traces:
        for part, fens, trace, _ in parts:
            lines = trace.read_text(encoding='utf-8').splitlines()
            if len(lines) != len(part):
                sys.exit(f'{trace}: {len(lines)} lines for {len(part)} positions')
            for (game, ply, result, score, fen), line in zip(part, lines):
                if line == 'error':
                    errors += 1
                    continue
                positions.write(f'{game}\t{ply}\t{result}\t{score}\t{fen}\n')
                traces.write(line + '\n')
                kept += 1
            fens.unlink()
            trace.unlink()
    print(f'{kept:,} positions traced in {seconds:.1f} s on {jobs} process(es); {errors} unreadable')


if __name__ == '__main__':
    main()
