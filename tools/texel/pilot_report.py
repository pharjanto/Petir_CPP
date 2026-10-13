"""Reports on a labelling pilot: cost per depth, fitted sigmoid, agreement with
the deepest depth, and the mirror and separate-search checks.

Scores are from White's view. A score s maps to an expected score for White
with sigma(s) = 1 / (1 + 10^(-K s / 400)); K is fitted per depth to the game
results by least squares. Mate scores count as 1 or 0.

Usage: pilot_report.py LABELS [--mirror MIRROR_LABELS] [--check CHECK_LABELS]
                       [--jobs N] [--worst N]
"""
import argparse
import json
import math
import statistics


def load(path):
    with open(path, encoding='utf-8') as rows:
        return [json.loads(line) for line in rows if line.strip()]


def expected(entry, k):
    if 'mate' in entry:
        return 1.0 if entry['mate'] > 0 else 0.0
    return 1.0 / (1.0 + 10.0 ** (-k * entry['cp'] / 400.0))


def error(rows, depth, k):
    return statistics.fmean((row['result'] - expected(row['scores'][depth], k)) ** 2 for row in rows)


def fit_k(rows, depth, low=0.05, high=4.0):
    """Golden-section search for the K that minimises the squared error."""
    ratio = (math.sqrt(5) - 1) / 2
    a, b = low, high
    c, d = b - ratio * (b - a), a + ratio * (b - a)
    for _ in range(60):
        if error(rows, depth, c) < error(rows, depth, d):
            b = d
        else:
            a = c
        c, d = b - ratio * (b - a), a + ratio * (b - a)
    return (a + b) / 2


def clipped(entry, cap=1000):
    if 'mate' in entry:
        return cap if entry['mate'] > 0 else -cap
    return max(-cap, min(cap, entry['cp']))


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('labels')
    parser.add_argument('--mirror')
    parser.add_argument('--check')
    parser.add_argument('--jobs', type=int, default=10, help='workers assumed for the positions/hour estimate')
    parser.add_argument('--worst', type=int, default=10)
    args = parser.parse_args()

    rows = load(args.labels)
    reference = str(max(int(depth) for depth in rows[0]['scores']))
    depths = [d for d in ('8', '10', '12', '14', '16', '18', reference) if all(d in row['scores'] for row in rows)]
    depths = sorted(set(depths), key=int)
    results = [row['result'] for row in rows]
    mean_result = statistics.fmean(results)
    print(f'{len(rows):,} positions; results: White {sum(r == 1 for r in results)}, '
          f'draw {sum(r == 0.5 for r in results)}, Black {sum(r == 0 for r in results)}; reference depth {reference}')
    print(f'squared error of the constant prediction {mean_result:.3f}: '
          f'{statistics.fmean((r - mean_result) ** 2 for r in results):.4f}')
    mates = sum('mate' in row['scores'][reference] for row in rows)
    print(f'mate scores at depth {reference}: {mates}')

    k_reference = fit_k(rows, reference)
    print(f'\n{"depth":>5} {"median ms":>9} {"mean ms":>8} {"mean nodes":>11} {"pos/hour":>9} '
          f'{"K":>6} {"sq.err":>7} {"|dcp| vs ref":>12} {"|dE| vs ref":>11} {">0.05":>6} {">0.10":>6}')
    for depth in depths:
        ms = [row['scores'][depth]['ms'] for row in rows]
        nodes = [row['scores'][depth]['nodes'] for row in rows]
        k = fit_k(rows, depth)
        diff_cp = [abs(clipped(row['scores'][depth]) - clipped(row['scores'][reference])) for row in rows]
        diff_e = [abs(expected(row['scores'][depth], k_reference) - expected(row['scores'][reference], k_reference))
                  for row in rows]
        per_hour = args.jobs * 3600 * 1000 / max(1, statistics.fmean(ms))
        print(f'{depth:>5} {statistics.median(ms):>9.0f} {statistics.fmean(ms):>8.0f} {statistics.fmean(nodes):>11,.0f} '
              f'{per_hour:>9,.0f} {k:>6.3f} {error(rows, depth, k):>7.4f} {statistics.fmean(diff_cp):>12.1f} '
              f'{statistics.fmean(diff_e):>11.4f} {sum(d > 0.05 for d in diff_e) / len(rows):>6.1%} '
              f'{sum(d > 0.10 for d in diff_e) / len(rows):>6.1%}')
    print('(|dcp| clips scores to +-1000; |dE| uses the reference K; pos/hour assumes', args.jobs, 'workers)')

    if args.mirror:
        mirrored = {(row['game'], row['ply']): row for row in load(args.mirror)}
        print(f'\nmirror check ({len(mirrored):,} colour-flipped positions, White-view scores should be negated):')
        for depth in depths:
            sums, exact, sign_flips = [], 0, 0
            for row in rows:
                other = mirrored[(row['game'], row['ply'])]['scores'][depth]
                mine = row['scores'][depth]
                total = clipped(mine) + clipped(other)
                sums.append(abs(total))
                exact += total == 0 and ('mate' in mine) == ('mate' in other)
                a, b = clipped(mine), -clipped(other)
                sign_flips += (a > 20 and b < -20) or (a < -20 and b > 20)
            print(f'  depth {depth:>2}: exactly negated {exact / len(rows):.1%}, mean |s + s_mirror| '
                  f'{statistics.fmean(sums):.1f} cp, max {max(sums)}, opposite signs beyond 20 cp: {sign_flips}')

    if args.check:
        checked = load(args.check)
        by_fen = {row['fen']: row for row in rows}
        same = 0
        for row in checked:
            depth = str(row['labelDepth'])
            mine, deep = row['scores'][depth], by_fen[row['fen']]['scores'][depth]
            same += {k: v for k, v in mine.items() if k != 'ms'} == {k: v for k, v in deep.items() if k != 'ms'}
        print(f'\nseparate-search check: {same}/{len(checked)} positions give the same depth-'
              f'{checked[0]["labelDepth"]} score and node count as the deep search')

    print(f'\nlargest disagreements between the result and the depth-{reference} score (K {k_reference:.3f}):')
    worst = sorted(rows, key=lambda row: -abs(row['result'] - expected(row['scores'][reference], k_reference)))
    for row in worst[:args.worst]:
        entry = row['scores'][reference]
        score = f"mate {entry['mate']}" if 'mate' in entry else f"{entry['cp']:+d}"
        print(f"  result {row['result']:.1f}  sf {score:>7}  ply {row['ply']:>3}  {row['white']} ({row['whiteElo']}) - "
              f"{row['black']} ({row['blackElo']})  {row['fen']}")

    elos = sorted(min(row['whiteElo'] or 0, row['blackElo'] or 0) for row in rows)
    print('\nlower Elo of the two engines, percentiles 10/25/50/75/90: '
          + ' / '.join(str(elos[int(p * (len(elos) - 1))]) for p in (0.1, 0.25, 0.5, 0.75, 0.9)))


if __name__ == '__main__':
    main()
