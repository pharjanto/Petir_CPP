# Protect equal-depth exact hash entries — 2026-10-20

Developed on branch `hash-preserve-exact`, based on `a2bc9a04dedd62b688661cca756d1ecafb117b97` (published timeReserve + 15 ms overhead build). The user approved publishing the exact-entry preference to main on 2026-10-20; no performance or strength improvement is claimed.

The baseline produced 3,445 exact-to-bound overwrites out of 2,355,866 equal-depth same-position updates (0.146%), or 169,265,050 total writes (0.002%). Events occurred in 474/720 searches. Of these baseline events, 2,649 have the matching counter and current generation required by the protection rule. Fresh-table nodes improve 0.411%; the single-pass timing improvement of 0.703% is small. The unrelated carried-table sample worsens nodes by 1.179%, but serves only as a populated-table replacement stress test: it does not model consecutive positions in actual play. Retaining the change is a preference for preserving more informative bounds, not an established performance gain or a correction of an invalid bound.

All three CTest suites passed in each experimental build: normal candidate (5,589 core checks), instrumented candidate (5,751), and instrumented unchanged baseline (5,751). The tests cover both destination slots and colors, exact/upper/lower transitions, current/old entries and matching/different counters. Saved measurement invariants and all executable checksums were independently verified after collection.

The measurement baseline preserves the existing search and replacement rules. Instrumentation observes the actual destination of each store before mutation, in both bucket slots. A new store into an empty slot counts as a write; an eviction of a different key is not a same-position update. Bucket-demotion copies are excluded from write counts.

An overwrite event means: same full key, same stored depth (quarter plies), existing exact bound, new upper or lower bound. All ages and counter contexts count in the broad statistic. The candidate protects only events with the same halfmove counter and current generation, retaining the entire old entry, including its move. Older entries and changed counters update normally. No shallower-move updates or other hash experiments are included.

360 positions, depth 12, Hash 64 MiB, fresh and carried tables, 6 workers pinned to separate physical cores (logical CPUs 2,4,...). Baseline and normal candidate alternate run order. Both are independently repeated by instrumented builds and required to match all ordinary UCI output after stripping timing fields. Only normal Release builds supply timings. One timed pass per mode; reported time is the sum of engine search times.

## Frequency on unchanged baseline

| Table | All writes | Same-key equal-depth writes | Exact→bound events | % of equal-depth same-key writes | % of all writes | Protectable events | Protectable % of equal-depth same-key writes |
|---|---:|---:|---:|---:|---:|---:|---:|
| fresh | 83,926,714 | 1,119,296 | 1,720 | 0.154% | 0.002% | 1,302 | 0.116% |
| carry | 85,338,336 | 1,236,570 | 1,725 | 0.139% | 0.002% | 1,347 | 0.109% |

| Table | First / second slot | Upper / lower incoming bound | Current / previous / older generation | Same / different counter |
|---|---|---|---|---|
| fresh | 1,690 / 30 | 865 / 855 | 1,720 / 0 / 0 | 1,302 / 418 |
| carry | 1,631 / 94 | 870 / 855 | 1,723 / 0 / 2 | 1,349 / 376 |

## Fixed-depth benchmark

| Table | Baseline nodes | Protected nodes | Node change | Baseline seconds | Protected seconds | Time change | Candidate prevented writes |
|---|---:|---:|---:|---:|---:|---:|---:|
| fresh | 868,202,064 | 864,636,233 | -0.411% | 143.705 | 142.695 | -0.703% | 1,364 |
| carry | 881,078,610 | 891,468,148 | +1.179% | 146.944 | 148.149 | +0.820% | 1,460 |

Baseline opportunities and candidate prevented writes come from different search trees; their totals need not match.

| Phase | Mode | Baseline exact→bound events | % of equal-depth same-key writes | Node change | Time change |
|---|---|---:|---:|---:|---:|
| late | fresh | 389 | 0.127% | -0.038% | -1.297% |
| late | carry | 444 | 0.149% | +7.852% | +7.593% |
| middlegame | fresh | 530 | 0.160% | -0.337% | -0.065% |
| middlegame | carry | 546 | 0.144% | +0.456% | -0.247% |
| opening | fresh | 801 | 0.166% | -0.634% | -0.781% |
| opening | carry | 735 | 0.131% | -1.353% | -1.450% |

## Sample positions with the most overwrite events

| Position | Mode | Exact→bound events | Protectable | % of equal-depth same-key writes |
|---|---|---:|---:|---:|
| opening-074 | carry | 85 | 81 | 0.066% |
| middlegame-037 | fresh | 46 | 31 | 0.983% |
| late-089 | carry | 41 | 30 | 1.036% |
| middlegame-037 | carry | 37 | 33 | 0.548% |
| opening-078 | fresh | 34 | 28 | 0.924% |
| middlegame-045 | carry | 31 | 31 | 1.865% |
| opening-052 | carry | 30 | 27 | 0.720% |
| opening-041 | fresh | 30 | 28 | 0.339% |
| opening-038 | fresh | 29 | 25 | 0.172% |
| opening-074 | fresh | 29 | 25 | 0.118% |
| opening-016 | carry | 28 | 25 | 0.313% |
| late-064 | carry | 27 | 9 | 0.442% |

All 720 baseline and 720 candidate searches match their respective instrumented builds. Best moves changed in 59/720 searches. Each raw record includes the position command/FEN and counters.

No strength match was started; these results do not establish Elo. Raw results and executable checksums: `build/hash-preserve-exact/results/results.json` and `BUILD_MANIFEST.json`.
