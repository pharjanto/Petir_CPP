# Halfmove-counter hash rejection measurement

Completed on 2026-10-19 in branch `hash-counter-measure`, based on `0092b95`.
The measurements retain the exact halfmove-counter check and all existing hash
decisions. Diagnostics are opt-in through `PETIR_HASH_STATS`; normal builds
compile out the observations and the extra probe output parameter.

- The carried-game replay blocked **39.66%**
  of otherwise eligible cutoff opportunities: **12,630,884 blocked** versus
  **19,215,617 accepted**. Blocked opportunities were
  **6.32%** of all non-PV main-search probes.
- **97.48%** of the carried-game blocked
  opportunities had both counters below 80. **87.27%**
  requested at most two whole plies. Their eligibility does not establish safe
  reuse or a corresponding percentage gain in time or playing strength.
- The game workload contains 170 opening, 282 middlegame and 668 endgame turns.
  Carried-hash blocked shares were 20.67%, 24.23% and 49.41%, respectively.
  The overall rate pools probes and is weighted toward the endgames in this sample.
  Games 2 and 5 together contribute 41.98% of all
  carried-game blocked opportunities; the median individual-game blocked share
  is 30.90%.
- Every ordinary UCI info/bestmove line matched the unmodified baseline after
  removing timing fields: **2,960 / 2,960 searches**. Diagnostic classifier
  disagreements: **zero** throughout.
- Release CTest: **3/3 passed** with diagnostics enabled and **3/3 passed**
  with them disabled. Core checks: **5,471** enabled (including 144 additional
  diagnostic checks), **5,327** disabled, zero failures. The added checks cover
  both sides, all three bound types, the second bucket slot, shallow/non-cutting
  entries, missing keys, PV exclusion, counter bands and unchanged table bytes.

The next decision is whether to design a separately tested relaxation for low
counters. This experiment only measures the current restriction. The predominance
of shallow opportunities makes a time-to-depth comparison essential for any
future candidate; the counts alone cannot predict the benefit.

Implementation and reproduction: [diagnostic guide](../tools/hash_counter_stats.md).
Local artifacts: [raw per-search results](../../results/results.json),
[replayed positions](../../results/game-positions.json),
[build manifest](../../BUILD_MANIFEST.json),
[enabled CTest log](../../stats-ctest.log), [disabled CTest log](../../off-ctest.log).
These artifacts are in the ignored build directory and are not committed.

Base: `0092b95`. Hash: **64 MiB**. Workers: 6; each pinned to a separate physical core.

The exact counter check remains enabled. Only non-PV main-search probes count below. Root and PV extraction probes are omitted; PV main-search probes are counted separately and excluded. A blocked cutoff has a matching key, sufficient stored depth, and an exact or alpha/beta-compatible bound. The counter is its only failed eligibility condition.

Blocked share = blocked / (accepted + blocked). These are observed opportunities along the unchanged search tree, rather than a prediction of nodes, time, or Elo saved.

| Dataset | Hash mode | Searches | Probes | Key hits | Counter mismatches | Accepted cutoffs | Counter-blocked cutoffs | Blocked share |
|---|---|---|---|---|---|---|---|---|
| bench | carry | 360 | 133,525,555 | 38,054,064 | 5,812,892 | 8,322,993 | 3,805,060 | 31.37% |
| bench | fresh | 360 | 128,377,149 | 37,564,108 | 5,879,125 | 8,021,080 | 3,838,001 | 32.36% |
| games | carry | 1120 | 199,726,597 | 80,245,345 | 20,302,395 | 19,215,617 | 12,630,884 | 39.66% |
| games | fresh | 1120 | 240,109,171 | 86,669,424 | 20,777,947 | 18,592,143 | 13,638,095 | 42.31% |

## bench, carry

- Nodes searched: 955,901,843.
- Key matches rejected by counter: 15.28%.
- Counter mismatches also too shallow: 1,844,965; deep enough but non-cutting bound: 162,867.
- Blocked cutoffs / all probes: 2.85%.
- PV main probes excluded: 1,397,230; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 5,812,892 | 3,805,060 | 100.00% | 1,607,380 | 1,618,670 | 579,010 |
| Either 80-99, neither >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 22,835 |
| Upper | 928,209 |
| Lower | 2,854,016 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 2,563,330 |
| 2 | 895,051 |
| 3 | 216,955 |
| 4-5 | 115,177 |
| 6-8 | 14,311 |
| 9+ | 236 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 3,802,060 |
| Previous search | 46 |
| Older | 2,954 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| late | 120 | 3,180,032 | 2,263,830 | 41.58% |
| middlegame | 120 | 2,484,281 | 851,462 | 25.53% |
| opening | 120 | 2,658,680 | 689,768 | 20.60% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| late-022 | 1,057,024 | 27.78% | 56.03% |
| late-052 | 119,754 | 3.15% | 35.32% |
| middlegame-057 | 97,150 | 2.55% | 40.30% |
| middlegame-016 | 45,465 | 1.19% | 24.57% |
| late-109 | 41,378 | 1.09% | 37.44% |

## bench, fresh

- Nodes searched: 898,026,445.
- Key matches rejected by counter: 15.65%.
- Counter mismatches also too shallow: 1,870,646; deep enough but non-cutting bound: 170,478.
- Blocked cutoffs / all probes: 2.99%.
- PV main probes excluded: 1,198,366; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 5,879,125 | 3,838,001 | 100.00% | 1,642,811 | 1,610,591 | 584,599 |
| Either 80-99, neither >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 24,753 |
| Upper | 948,841 |
| Lower | 2,864,407 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 2,582,150 |
| 2 | 890,858 |
| 3 | 225,646 |
| 4-5 | 123,562 |
| 6-8 | 15,556 |
| 9+ | 229 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 3,838,001 |
| Previous search | 0 |
| Older | 0 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| late | 120 | 3,191,202 | 2,327,255 | 42.17% |
| middlegame | 120 | 2,373,680 | 849,805 | 26.36% |
| opening | 120 | 2,456,198 | 660,941 | 21.20% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| late-022 | 999,648 | 26.05% | 59.94% |
| late-052 | 122,420 | 3.19% | 36.04% |
| late-109 | 81,243 | 2.12% | 37.69% |
| late-097 | 51,368 | 1.34% | 30.60% |
| middlegame-057 | 49,049 | 1.28% | 39.79% |

## games, carry

- Nodes searched: 1,175,862,363.
- Key matches rejected by counter: 25.30%.
- Counter mismatches also too shallow: 6,771,025; deep enough but non-cutting bound: 900,486.
- Blocked cutoffs / all probes: 6.32%.
- PV main probes excluded: 1,820,336; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 19,827,518 | 12,312,017 | 97.48% | 5,196,843 | 3,947,502 | 3,167,672 |
| Either 80-99, neither >=100 | 474,877 | 318,867 | 2.52% | 49,806 | 71,043 | 198,018 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 189,197 |
| Upper | 2,677,713 |
| Lower | 9,763,974 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 8,084,882 |
| 2 | 2,937,820 |
| 3 | 1,013,882 |
| 4-5 | 535,454 |
| 6-8 | 57,986 |
| 9+ | 860 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 10,049,574 |
| Previous search | 1,696,939 |
| Older | 884,371 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| opening | 170 | 3,682,690 | 959,553 | 20.67% |
| middlegame | 282 | 5,326,479 | 1,703,080 | 24.23% |
| endgame | 668 | 10,206,448 | 9,968,251 | 49.41% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| game-002-ply-167 | 216,193 | 1.71% | 57.38% |
| game-005-ply-198 | 148,821 | 1.18% | 73.52% |
| game-002-ply-163 | 110,959 | 0.88% | 54.17% |
| game-002-ply-223 | 95,496 | 0.76% | 69.43% |
| game-007-ply-038 | 92,683 | 0.73% | 33.08% |

## games, fresh

- Nodes searched: 1,435,602,950.
- Key matches rejected by counter: 23.97%.
- Counter mismatches also too shallow: 6,229,628; deep enough but non-cutting bound: 910,224.
- Blocked cutoffs / all probes: 5.68%.
- PV main probes excluded: 2,284,551; classifier disagreements with the real probe: 0.

| Counter band | All mismatches | Blocked cutoffs | Share of blocked | Difference 1 | Difference 2 | Difference 3+ |
|---|---|---|---|---|---|---|
| Both <80 | 20,338,340 | 13,304,992 | 97.56% | 5,782,087 | 4,401,682 | 3,121,223 |
| Either 80-99, neither >=100 | 439,607 | 333,103 | 2.44% | 65,834 | 67,922 | 199,347 |
| Either >=100 | 0 | 0 | 0.00% | 0 | 0 | 0 |

| Bound | Blocked cutoffs |
|---|---|
| Exact | 216,906 |
| Upper | 2,859,177 |
| Lower | 10,562,012 |

| Requested whole plies | Blocked cutoffs |
|---|---|
| 0-1 | 8,585,570 |
| 2 | 3,252,486 |
| 3 | 1,145,729 |
| 4-5 | 590,548 |
| 6-8 | 63,152 |
| 9+ | 610 |

| Entry generation | Blocked cutoffs |
|---|---|
| Current search | 13,638,095 |
| Previous search | 0 |
| Older | 0 |

| Position phase | Searches | Accepted cutoffs | Blocked cutoffs | Blocked share |
|---|---|---|---|---|
| opening | 170 | 3,196,502 | 945,706 | 22.83% |
| middlegame | 282 | 4,991,928 | 1,866,161 | 27.21% |
| endgame | 668 | 10,403,713 | 10,826,228 | 51.00% |

| Largest contributors | Blocked cutoffs | Share of this dataset | Blocked share |
|---|---|---|---|
| game-002-ply-263 | 170,111 | 1.25% | 70.61% |
| game-002-ply-167 | 155,509 | 1.14% | 58.31% |
| game-005-ply-198 | 136,676 | 1.00% | 76.39% |
| game-002-ply-163 | 107,750 | 0.79% | 51.00% |
| game-002-ply-151 | 104,399 | 0.77% | 60.80% |

## Validation and interpretation

- Identical baseline/instrumented searches: 2,960 / 2,960. Every ordinary UCI info/bestmove line is compared except timing fields.
- Counts include all iterative-deepening iterations and aspiration/PVS re-searches.
- Counter bands describe the stored/current node counters. Both below 80 does not guarantee descendants stay below the evaluation fade: extensions and quiescence can continue.
- The diagnostic build adds counting overhead. Its timings are not a speed comparison.

- Game sample: first 20 games from `C:\chess\results\lc_vs_base.pgn`, player `LC`, searching every own turn after 16 opening plies at fixed depth 12. Actual played moves are replayed regardless of the analysis move.
- Replay root halfmove counters: 0..86. Each game starts a new session history; carried hash persists between the selected player's turns.

## Run metadata

```json
{
  "base_commit": "0092b95",
  "engine": "C:\\chess\\Petir_CPP\\build\\hash-counter\\stats-build\\Release\\petir64_hashcounter.exe",
  "baseline": "C:\\chess\\Petir_CPP\\build\\hash-counter\\base-build\\Release\\petir64.exe",
  "hash_mb": 64,
  "jobs": 6,
  "first_core": 2,
  "bench_positions": "C:\\chess\\Petir_CPP\\build\\hash-counter\\tree\\tools\\eval_diff\\bench-games.json",
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
