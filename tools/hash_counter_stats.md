# Halfmove-counter hash diagnostics

Build with `-DPETIR_HASH_STATS=ON` to enable read-only observations of the
entry selected by `probeHash`. The normal build compiles out the hooks and the
extra probe output parameter. The exact halfmove-counter eligibility check,
replacement policy, scores, moves and 16-byte table entries are unchanged.

After a search finishes, send `hashstats`. It prints one JSON line and clears
the counters. Starting a search also resets them. `hashstats` joins the worker
before reading the counters; it should be sent after `bestmove` so it does not
interrupt the search being measured.

Only non-PV `mainSearch` probes contribute to the primary counters. Root,
fallback and PV-extraction probes do not request diagnostic output. Main-search
PV probes contribute only to `pv_probes_excluded` because a TT hit cannot end
those searches. Quiescence currently has no TT probes.

- `counter_mismatches`: full key matches whose stored/current counters differ.
- `depth_rejected_mismatches`: mismatches that also fail the stored-depth check.
- `bound_rejected_mismatches`: remaining mismatches whose bound cannot cut off.
- `counter_blocked_cutoffs`: remaining mismatches. Depth and bound would permit
  a cutoff if the counters matched.
- `accepted_cutoffs`: real successful probes at non-PV main-search nodes.
- `decision_mismatches`: disagreements between the diagnostic classifier and
  the real probe. This must be zero.

The blocked share is `counter_blocked_cutoffs / (accepted_cutoffs +
counter_blocked_cutoffs)`. It is an eligibility statistic on the current search
tree, rather than an estimate of speed or playing strength. A skipped cutoff
could save very different amounts of work, and allowing it would change later
searches and table contents.

Array labels are documented alongside the fields in `src/hash_stats.hpp`.
Counter bands use the larger of the stored/current counters: below 80, 80-99,
and 100+. In particular, two counters below 80 do not establish safe score
reuse; descendants may enter evaluation fading or the draw limit. Requested
depths are converted from Petir's fractional-ply units to whole plies only for
the report; eligibility uses exactly the existing compressed-depth comparison.

The driver compares all ordinary UCI info/bestmove lines with `--baseline`,
ignoring only time/nps/hashfull. It validates counter partitions on every
search and writes per-shard JSON, combined raw results and a Markdown report.
Each worker runs one active engine at a time, pinned to a physical core on
Windows. Check machine load and leave at least two physical cores idle before
starting it.

Example from PowerShell, with paths adjusted to the build directories:

```powershell
& $pythonPath -I tools/hash_counter_stats.py $statsExe `
  --baseline $baseExe --base-commit 0092b95 --hash 64 --jobs 6 `
  --depth-offset 2 --pgn C:/chess/results/lc_vs_base.pgn --player LC `
  --games 20 --game-depth 12 `
  --python-chess C:/chess/Petir_CPP/build/kpk-audit/python-chess-master `
  --out C:/chess/Petir_CPP/build/hash-counter/results
```

The default 360-position bench uses strided sessions. Fresh mode sends
`ucinewgame` for every position; carry mode keeps entries and ordering history
within each shard. These bench positions are independent samples, rather than
consecutive moves of one game.

The optional PGN sample takes the first N games and analyzes the selected
player's turns after the opening plies. It replays the actual recorded moves,
so counters, repetitions and castling history are preserved. Carried mode
keeps entries between that player's turns and resets for the next game. These
are fixed-depth replays of existing games, rather than new clocked matches or
a reproduction of the original game's exact searches.
