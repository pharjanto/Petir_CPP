"""Compares two order_stats.py --json runs of the same suite: node counts and move-ordering metrics.

Usage: order_compare.py BASE.json[,..] CANDIDATE.json[,..] [--groups G,..] [--detail-groups G,..] [--report OUT]

Each side may list several JSON files, for a suite searched in shards.

Node counts at a fixed depth say how much work each build needed; the ordering
metrics say why. Single-position differences are mostly tie-break noise, so read
the totals and the geometric mean rather than any one row.
"""
import argparse
import json
import math
import statistics
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from order_stats import DEEP, DEPTHS, cuts, first_cuts, pct, table, total  # noqa: E402

QUIET, KILLERS = 6, range(2, 6)


def load(paths):
    return {r['name']: r for path in paths.split(',') for r in json.loads(Path(path).read_text(encoding='utf-8'))
            if r['variant'] == 'original'}


def change(a, b):
    return f'{100 * (b - a) / a:+.1f}%' if a else '-'


def geomean_change(pairs):
    """Geometric-mean node change and its 95% confidence range."""
    logs = [math.log(c['nodes'] / b['nodes']) for b, c in pairs if b['nodes'] and c['nodes']]
    if not logs:
        return '-', '-'
    mean = sum(logs) / len(logs)
    if len(logs) < 2:
        return f'{100 * (math.exp(mean) - 1):+.1f}%', '-'
    half = 1.96 * statistics.stdev(logs) / math.sqrt(len(logs))
    return (f'{100 * (math.exp(mean) - 1):+.1f}%',
            f'{100 * (math.exp(mean - half) - 1):+.1f}% to {100 * (math.exp(mean + half) - 1):+.1f}%')


def node_rows(base, cand, groups):
    rows = []
    for label, names in [(g, [n for n in base if base[n]['group'] == g]) for g in groups] + \
                        [('**all**', [n for n in base if base[n]['group'] in groups])]:
        pairs = [(base[n], cand[n]) for n in names if n in cand]
        if not pairs:
            continue
        a, b = sum(p['nodes'] for p, _ in pairs), sum(q['nodes'] for _, q in pairs)
        fewer = sum(1 for p, q in pairs if q['nodes'] < p['nodes'])
        more = sum(1 for p, q in pairs if q['nodes'] > p['nodes'])
        same_best = sum(1 for p, q in pairs if p['bestmove'] == q['bestmove'])
        rows.append([label, len(pairs), f'{a:,}', f'{b:,}', change(a, b), *geomean_change(pairs),
                     f'{fewer} / {more}', f'{same_best}/{len(pairs)}'])
    return rows


def metrics(c):
    """Ordering metrics as (label, value as a fraction or None)."""
    def frac(part, whole):
        return part / whole if whole else None
    n = cuts(c)
    deep_cuts = sum(sum(c['cut_phase'][d]) for d in DEEP)
    deep_wasted = sum(c['cut_wasted'][d] for d in DEEP)
    deep_nodes = sum(c['cut_nodes'][d] for d in DEEP)
    quiet_cuts = sum(c['cut_phase'][d][QUIET] for d in range(len(DEPTHS)))
    quiet_tried = sum(c['tried_phase'][d][QUIET] for d in range(len(DEPTHS)))
    rank_cut = [w + b for w, b in zip(*c['quiet_rank'])]
    rank_tried = [w + b for w, b in zip(*c['quiet_tried_rank'])]
    nohash = [sum(c['cut_by_hash'][d][0]) for d in DEEP]
    nohash_first = [c['cut_by_hash'][d][0][1] for d in DEEP]
    killer_cut = sum(c['cut_phase'][d][k] for d in range(len(DEPTHS)) for k in KILLERS)
    killer_tried = sum(c['tried_phase'][d][k] for d in range(len(DEPTHS)) for k in KILLERS)
    return [
        ('First-move cutoff rate', frac(first_cuts(c), n)),
        ('Average cutoff move number', sum(c['cut_move_sum']) / n if n else None),
        ('Cutoffs at move 6 or later', frac(sum(sum(row[5:]) for row in c['cut_index']), n)),
        ('Wasted share, all cut nodes', frac(sum(c['cut_wasted']), sum(c['cut_nodes']))),
        ('Wasted share, cut nodes with 4+ plies left', frac(deep_wasted, deep_nodes)),
        ('  of which in nodes cut by a quiet move', frac(sum(c['phase_wasted'][d][QUIET] for d in DEEP), deep_wasted)),
        ('Quiet move: share of cutoffs', frac(quiet_cuts, n)),
        ('Quiet move: hit rate, any position', frac(quiet_cuts, quiet_tried)),
        ('Quiet move: hit rate at list position 1', frac(rank_cut[0], rank_tried[0])),
        ('Quiet cutoffs at list position 6 or later', frac(sum(rank_cut[4:]), sum(rank_cut))),
        ('Quiet cutoffs past position 20 (unsorted)', frac(rank_cut[6], sum(rank_cut))),
        ('Killer hit rate', frac(killer_cut, killer_tried)),
        ('No hash move, 4+ plies: first-move rate', frac(sum(nohash_first), sum(nohash))),
        ('PV nodes: best move searched first', frac(c['pv_best_index'][0], sum(c['pv_best_index']))),
        ('Reduced moves re-searched after failing high', frac(c['lmr_research'], c['reduced'])),
        ('Root: final best move was searched first', frac(c['root_best_index'][0], sum(c['root_best_index']))),
    ]


def show(label, v):
    if v is None:
        return '-'
    return f'{v:.2f}' if label.startswith('Average') else f'{100 * v:.1f}%'


def compare(base_files, cand_files, groups, detail_groups):
    base, cand = load(base_files), load(cand_files)
    groups = groups or list(dict.fromkeys(r['group'] for r in base.values()))
    out = [f'# Move ordering: {cand_files} against {base_files}', '',
           '## Nodes at fixed depth', '',
           'Geometric mean: the typical per-position change, so one big position cannot dominate it; '
           'its 95% range comes from the spread between positions. '
           'Fewer / more: positions where the candidate searched fewer or more nodes.', '',
           table(['Group', 'Positions', 'Base nodes', 'Candidate nodes', 'Change', 'Geometric mean',
                  '95% range', 'Fewer / more', 'Same best move'], node_rows(base, cand, groups)), '']
    names = [n for n in base if base[n]['group'] in detail_groups and n in cand]
    if names:
        mb = metrics(total([base[n] for n in names]))
        mc = metrics(total([cand[n] for n in names]))
        out += [f'## Ordering metrics ({", ".join(detail_groups)})', '',
                table(['Metric', 'Base', 'Candidate'],
                      [[label, show(label, a), show(label, b)] for (label, a), (_, b) in zip(mb, mc)]), '']
    out += ['## Per position', '',
            table(['Position', 'Group', 'Depth', 'Base nodes', 'Candidate nodes', 'Change',
                   'Base best', 'Candidate best', 'Base score', 'Candidate score'],
                  [[n, base[n]['group'], base[n]['depth'], f'{base[n]["nodes"]:,}', f'{cand[n]["nodes"]:,}',
                    change(base[n]['nodes'], cand[n]['nodes']), base[n]['bestmove'], cand[n]['bestmove'],
                    base[n]['score'], cand[n]['score']]
                   for n in base if base[n]['group'] in groups and n in cand]), '']
    return '\n'.join(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('base')
    parser.add_argument('candidate')
    parser.add_argument('--groups', help='groups to compare (default: all)')
    parser.add_argument('--detail-groups', default='middlegame,tactical')
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    text = compare(args.base, args.candidate, args.groups and args.groups.split(','), args.detail_groups.split(','))
    if args.report:
        args.report.write_text(text + '\n', encoding='utf-8')
    print(text)


if __name__ == '__main__':
    main()
