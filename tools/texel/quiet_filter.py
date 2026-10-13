"""Keeps the sampled positions that are quiet for Petir.

A position is quiet when Petir's quiescence search with the full window returns
its static evaluation: no capture, promotion or (with --checks) checking move
changes the score. The tuner compares the static evaluation with the label, so
a position whose score a capture would change only adds noise. Also dropped:
positions whose score comes from an exact rule that no weight can change (KPK,
material draws, theoretical draws), the rook + minor against rook ending without
pawns (the evaluation divides its score by 8), positions in check and FENs
Petir cannot read.

The scores come from quiet_eval (tools/texel/qfilter), built from the source
tree whose evaluation is tuned. Every kept row gets Petir's scores from White's
view: "petir": {"eval", "qs", "qsChecks"}.

Use captures only (the default). In the pilot, --checks dropped another 7% of
positions, mostly for small shifts (median 15 cp; only 4 of 73 over 100 cp)
rather than tactics, and those positions were the more
decided ones, so dropping them would skew the set.

Usage: quiet_filter.py QUIET_EVAL_EXE IN OUT [--checks] [--rejected FILE] [--jobs N]
"""
import argparse
import collections
import json
import subprocess
import sys
import tempfile
import time
from pathlib import Path

DROPPED_RULES = {'kpk': 'exact rule (KPK)', 'materialDraw': 'exact rule (material draw)',
                 'theoreticalDraw': 'exact rule (theoretical draw)', 'likelyDraw': 'rook + minor against rook'}


def petir_scores(exe, fens, jobs):
    """Runs quiet_eval on the FENs in JOBS slices at once and returns its output lines in order."""
    size = (len(fens) + jobs - 1) // jobs
    slices = [fens[start:start + size] for start in range(0, len(fens), size)]
    with tempfile.TemporaryDirectory(prefix='quiet_filter_') as work:
        work = Path(work)
        processes = []
        for number, part in enumerate(slices):
            (work / f'{number}.fen').write_text(''.join(fen + '\n' for fen in part), encoding='utf-8')
            processes.append(subprocess.Popen([exe, str(work / f'{number}.fen'), str(work / f'{number}.tsv')],
                                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL))
        lines = []
        for number, process in enumerate(processes):
            if process.wait() != 0:
                raise SystemExit(f'quiet_eval failed on slice {number} (exit code {process.returncode})')
            part = (work / f'{number}.tsv').read_text(encoding='utf-8').splitlines()
            if len(part) != len(slices[number]):
                raise SystemExit(f'quiet_eval gave {len(part)} lines for {len(slices[number])} positions')
            lines += part
    return lines


def verdict(line, checks):
    """Returns (reason or None, scores) for one quiet_eval output line."""
    fields = line.split('\t')
    if fields[0] == 'error':
        return 'unreadable FEN', None
    evaluation, captures, with_checks = map(int, fields[:3])
    rule, in_check = fields[3], fields[4] == '1'
    scores = {'eval': evaluation, 'qs': captures, 'qsChecks': with_checks}
    if in_check:
        return 'in check', scores
    if rule in DROPPED_RULES:
        return DROPPED_RULES[rule], scores
    if captures != evaluation:
        return 'quiescence changes the score', scores
    if checks and with_checks != evaluation:
        return 'a check changes the score', scores
    return None, scores


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('exe')
    parser.add_argument('positions')
    parser.add_argument('out')
    parser.add_argument('--checks', action='store_true',
                        help='also require that checks at the first ply leave the score unchanged')
    parser.add_argument('--rejected', help='write the dropped rows here, with their reason')
    parser.add_argument('--jobs', type=int, default=1)
    args = parser.parse_args()

    with open(args.positions, encoding='utf-8') as rows:
        positions = [json.loads(line) for line in rows if line.strip()]
    start = time.monotonic()
    lines = petir_scores(str(Path(args.exe).resolve()), [row['fen'] for row in positions], max(1, args.jobs))
    seconds = time.monotonic() - start

    counts = collections.Counter()
    rejected = open(args.rejected, 'w', encoding='utf-8', newline='\n') if args.rejected else None
    with open(args.out, 'w', encoding='utf-8', newline='\n') as out:
        for row, line in zip(positions, lines):
            reason, scores = verdict(line, args.checks)
            if scores:
                row['petir'] = scores
            if reason is None:
                counts['kept'] += 1
                out.write(json.dumps(row) + '\n')
            else:
                counts[reason] += 1
                if rejected:
                    row['rejected'] = reason
                    rejected.write(json.dumps(row) + '\n')
    if rejected:
        rejected.close()
    print(f'{len(positions):,} positions scored in {seconds:.1f} s')
    for reason, count in counts.most_common():
        print(f'{reason:>32}: {count:>9,}  {count / len(positions):6.1%}')


if __name__ == '__main__':
    sys.exit(main())
