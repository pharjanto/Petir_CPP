# Conservative halfmove-counter TT relaxation

Branch: `hash-counter-relax`, based on measurement commit `c8d7407` and engine
base `0092b95`. Main is unchanged.

For a matching board key with sufficient stored depth and a usable bound, allow
different halfmove counters only when the cached score is outside the mate range
and:

```text
max(storedCounter, currentCounter) + ceil(storedDepthInPlies) + 16 < 80
```

The TT stores quarter plies, so the conversion is `(entry.depth + 3) / 4`.
The guard uses the stored depth, even when the current request is shallower.
Scores with absolute value >=19900 still require exact counters. The
`avoidNullMove` hint always requires exact counters. Matching-counter behavior,
entry size, replacement policy and book/repetition keys are unchanged.

The 16-ply margin is experimental. Extensions and quiescence make it a heuristic,
not a guarantee that every descendant avoids counter-dependent evaluation.
Relaxed TT entries can also be reused in later searches; the guard does not track
the entire ancestry of a cached bound. Playing strength needs a separate match.

Validation: all three CTest suites pass in Release with diagnostics off and on.
Core regression: 5,439 checks off, 5,601 checks on, zero failures. Coverage includes
both colors, all bound types, both directions of counter mismatch, rounded depth,
the strict threshold boundary, deep stored results, mate scores, null restrictions,
second-slot probes and independent diagnostic classification. The protocol suite
retains the original counter-0/counter-99 cached-score reproduction for both colors.

## Results

| Sample | Mode | Node change | Time change | Relaxed cutoffs | Changed best moves |
|---|---|---:|---:|---:|---:|
| bench (360) | fresh | -3.32% | -2.82% | 2,091,502 | 72 |
| bench (360) | carry | -7.83% | -6.33% | 2,013,705 | 83 |
| games (1120) | fresh | -7.43% | -5.20% | 5,921,351 | 203 |
| games (1120) | carry | -10.44% | -7.93% | 5,981,621 | 246 |

All **2,960** diagnostic candidate searches matched the diagnostics-disabled candidate. Classifier disagreements: **zero**. Baseline search differences are expected.

No playing-strength match has been run for this candidate. The margin remains experimental.

## Measurement method

Hash=64 MiB, fixed depth 12. The original 360-position bench and the first 20
games in `C:/chess/results/lc_vs_base.pgn` provide 1,120 LC turns after the opening.
Each is run with fresh and carried hash, for 2,960 searches per engine.
Six workers use separate physical cores, with one active engine per worker.
Baseline/candidate timing order alternates by search. The candidate is timed
without diagnostics; every diagnostic candidate search must match its ordinary
UCI output exactly, ignoring time/nps/hashfull. The baseline is the unmodified
`0092b95` build with matching compiler and Release settings.

This is one timing pass on a shared machine. Node changes are reproducible for
these sessions; time changes are approximate and depend on position mix. Changed
best moves at fixed depth are observations, not evidence of better or worse play.

Raw local artifacts: [per-search results](../../relax-results/results.json),
[full diagnostic report](../../relax-results/REPORT.md),
[replayed positions](../../relax-results/game-positions.json).

Reproduce from this worktree after building the three executables:

```powershell
& 'C:/Users/USER/AppData/Local/Temp/petir-protocol-python-3.12.10/python.exe' -I tools/hash_counter_stats.py `
  ../relax-stats-build/Release/petir64_hashrelax_stats.exe `
  --baseline ../base-build/Release/petir64.exe `
  --timing-engine ../relax-build/Release/petir64_hashrelax.exe `
  --allow-search-changes --base-commit 0092b95 --hash 64 --jobs 6 --depth-offset 2 `
  --pgn C:/chess/results/lc_vs_base.pgn --player LC --games 20 --game-depth 12 `
  --python-chess C:/chess/Petir_CPP/build/kpk-audit/python-chess-master `
  --out ../relax-results
```

Build with MSVC x64 Release and `PETIR_HASH_STATS=ON` for the diagnostic executable;
use `OFF` for the ordinary candidate. The output names are set through
`PETIR_OUTPUT_NAME`. The baseline build is from the separate `base-tree` worktree.

## Detailed counters


Non-mate scores allow differing counters when max(counter) + ceil(stored depth in plies) + 16 < 80. Exact counters are still required for mate-range scores and avoidNullMove. Only non-PV main-search probes contribute to the counts. Search changes are expected.

Base: `0092b95`. Hash: 64 MiB. Workers: 6.

| Dataset | Hash mode | Searches | Probes | Key hits | Counter mismatches | Accepted cutoffs | Counter-blocked cutoffs | Blocked share |
|---|---|---|---|---|---|---|---|---|
| bench | carry | 360 | 125,343,868 | 33,617,904 | 3,605,654 | 8,736,452 | 25,681 | 0.29% |
| bench | fresh | 360 | 124,303,360 | 33,867,567 | 3,733,894 | 8,876,903 | 31,442 | 0.35% |
| games | carry | 1120 | 177,923,061 | 63,062,819 | 12,216,201 | 19,462,947 | 1,031,865 | 5.03% |
| games | fresh | 1120 | 221,642,331 | 70,960,844 | 11,808,523 | 19,649,244 | 1,015,092 | 4.91% |

## bench, carry

- Nodes searched: 881,078,610.
- Key matches rejected by counter: 10.73%.
- Counter mismatches also too shallow: 1,423,549; deep enough but non-cutting bound: 142,719.
- Blocked cutoffs / all probes: 0.02%.
- PV main probes excluded: 1,423,357; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 3,605,654 | 25,681 | 100.00% | 11,968 | 9,366 | 4,347 |
| Either 80-99, neither >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 20 |
| Upper | 2,614 |
| Lower | 23,047 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 15,258 |
| 2 | 7,066 |
| 3 | 2,044 |
| 4-5 | 1,221 |
| 6-8 | 92 |
| 9+ | 0 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 25,676 |
| Previous search | 0 |
| Older | 5 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| late | 120 | 3,400,162 | 17,943 | 0.52% |
| middlegame | 120 | 2,563,266 | 6,585 | 0.26% |
| opening | 120 | 2,773,024 | 1,153 | 0.04% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| late-109 | 7,348 | 28.61% | 6.96% |
| late-017 | 2,271 | 8.84% | 2.94% |
| late-035 | 1,518 | 5.91% | 1.75% |
| middlegame-057 | 695 | 2.71% | 1.10% |
| late-029 | 682 | 2.66% | 1.99% |

## bench, fresh

- Nodes searched: 868,202,064.
- Key matches rejected by counter: 11.02%.
- Counter mismatches also too shallow: 1,454,493; deep enough but non-cutting bound: 156,457.
- Blocked cutoffs / all probes: 0.03%.
- PV main probes excluded: 1,211,701; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 3,733,894 | 31,442 | 100.00% | 14,943 | 11,342 | 5,157 |
| Either 80-99, neither >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 51 |
| Upper | 3,711 |
| Lower | 27,680 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 18,644 |
| 2 | 8,550 |
| 3 | 2,428 |
| 4-5 | 1,671 |
| 6-8 | 149 |
| 9+ | 0 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 31,442 |
| Previous search | 0 |
| Older | 0 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| late | 120 | 3,592,778 | 23,034 | 0.64% |
| middlegame | 120 | 2,510,378 | 7,017 | 0.28% |
| opening | 120 | 2,773,747 | 1,391 | 0.05% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| late-109 | 11,948 | 38.00% | 8.12% |
| late-017 | 2,502 | 7.96% | 4.20% |
| late-035 | 1,089 | 3.46% | 1.69% |
| late-096 | 982 | 3.12% | 3.34% |
| middlegame-117 | 954 | 3.03% | 2.77% |

## games, carry

- Nodes searched: 1,053,054,759.
- Key matches rejected by counter: 19.37%.
- Counter mismatches also too shallow: 4,519,151; deep enough but non-cutting bound: 683,564.
- Blocked cutoffs / all probes: 0.58%.
- PV main probes excluded: 1,684,395; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 11,664,627 | 657,922 | 63.76% | 222,559 | 171,221 | 264,142 |
| Either 80-99, neither >=100 | 551,574 | 373,943 | 36.24% | 53,234 | 80,333 | 240,376 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 28,136 |
| Upper | 201,201 |
| Lower | 802,528 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 581,810 |
| 2 | 273,109 |
| 3 | 111,746 |
| 4-5 | 59,773 |
| 6-8 | 5,370 |
| 9+ | 57 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 884,798 |
| Previous search | 90,817 |
| Older | 56,250 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| opening | 170 | 3,725,330 | 2,113 | 0.06% |
| middlegame | 282 | 5,464,049 | 85,382 | 1.54% |
| endgame | 668 | 10,273,568 | 944,370 | 8.42% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| game-005-ply-198 | 175,250 | 16.98% | 73.64% |
| game-005-ply-200 | 72,384 | 7.01% | 66.98% |
| game-005-ply-202 | 64,849 | 6.28% | 69.49% |
| game-005-ply-184 | 62,829 | 6.09% | 69.28% |
| game-005-ply-196 | 60,117 | 5.83% | 71.41% |

## games, fresh

- Nodes searched: 1,329,000,074.
- Key matches rejected by counter: 16.64%.
- Counter mismatches also too shallow: 4,219,091; deep enough but non-cutting bound: 652,989.
- Blocked cutoffs / all probes: 0.46%.
- PV main probes excluded: 2,215,876; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 11,348,080 | 677,060 | 66.70% | 272,778 | 178,459 | 225,823 |
| Either 80-99, neither >=100 | 460,443 | 338,032 | 33.30% | 63,436 | 71,498 | 203,098 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 25,882 |
| Upper | 193,451 |
| Lower | 795,759 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 556,258 |
| 2 | 276,430 |
| 3 | 116,109 |
| 4-5 | 60,428 |
| 6-8 | 5,849 |
| 9+ | 18 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 1,015,092 |
| Previous search | 0 |
| Older | 0 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| opening | 170 | 3,464,501 | 3,068 | 0.09% |
| middlegame | 282 | 5,459,233 | 131,418 | 2.35% |
| endgame | 668 | 10,725,510 | 880,606 | 7.59% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| game-005-ply-198 | 154,806 | 15.25% | 74.07% |
| game-005-ply-202 | 90,063 | 8.87% | 72.08% |
| game-005-ply-196 | 64,532 | 6.36% | 71.50% |
| game-005-ply-180 | 58,903 | 5.80% | 62.22% |
| game-005-ply-194 | 53,217 | 5.24% | 70.91% |


## Relaxed cutoffs and search cost

| Dataset | Mode | Relaxed cutoffs | Base nodes | Candidate nodes | Node change | Base ms | Candidate ms | Time change | Different best moves |
|---|---|---|---|---|---|---|---|---|---|
| bench | carry | 2,013,705 | 955901843 | 881078610 | -7.83% | 158353 | 148337 | -6.33% | 83 |
| bench | fresh | 2,091,502 | 898026445 | 868202064 | -3.32% | 145220 | 141123 | -2.82% | 72 |
| games | carry | 5,981,621 | 1175862363 | 1053054759 | -10.44% | 205800 | 189478 | -7.93% | 246 |
| games | fresh | 5,921,351 | 1435602950 | 1329000074 | -7.43% | 252364 | 239236 | -5.20% | 203 |

Timing uses a separate diagnostics-disabled candidate; diagnostics parity is required on every search. Fixed-depth differences do not establish playing strength.
## Validation and interpretation

- Identical baseline/instrumented searches: 22 / 2,960. Every ordinary UCI info/bestmove line is compared except timing fields.
- Counts include all iterative-deepening iterations and aspiration/PVS re-searches.
- Counter bands describe the stored/current node counters. Both below 80 does not guarantee descendants stay below the evaluation fade: extensions and quiescence can continue.
- Timings use diagnostics-disabled engines, with alternating run order; concurrency and a single pass still limit precision.

- Game sample: first 20 games from `C:\chess\results\lc_vs_base.pgn`, player `LC`, searching every own turn after 16 opening plies at fixed depth 12. Actual played moves are replayed regardless of the analysis move.
- Replay root halfmove counters: 0..86. Each game starts a new session history; carried hash persists between the selected player's turns.

## Run metadata

```json
{
  "base_commit": "0092b95",
  "engine": "C:\\chess\\Petir_CPP\\build\\hash-counter\\relax-stats-build\\Release\\petir64_hashrelax_stats.exe",
  "baseline": "C:\\chess\\Petir_CPP\\build\\hash-counter\\base-build\\Release\\petir64.exe",
  "timing_engine": "C:\\chess\\Petir_CPP\\build\\hash-counter\\relax-build\\Release\\petir64_hashrelax.exe",
  "hash_mb": 64,
  "jobs": 6,
  "first_core": 2,
  "bench_positions": "C:\\chess\\Petir_CPP\\build\\hash-counter\\relax-tree\\tools\\eval_diff\\bench-games.json",
  "bench_depth_offset": 2,
  "bench_depth_range": [
    12,
    12
  ],
  "modes": [
    "fresh",
    "carry"
  ],
  "games": {
    "pgn": "C:\\chess\\results\\lc_vs_base.pgn",
    "games": 20,
    "player": "LC",
    "book_plies": 16,
    "depth": 12,
    "positions": 1120,
    "min_counter": 0,
    "max_counter": 86
  }
}
```
