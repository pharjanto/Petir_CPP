# lazy_margin: lazy-eval margin statistics in search

Use these tools before changing the lazy-evaluation margin in `src/ueval.cpp`.
They measure, in real searches, how often the lazy exit fires and how often it
is wrong. They then replay the recorded calls under other margin schemes,
without rebuilding the engine for each scheme.

**lazystat** is the full UCI engine built from the new tree. Its `evaluate()`
gives search exactly the new tree's result, so the search is unchanged. For
every null-window call that reaches the lazy checkpoint, it records:

- the partial score at the checkpoint and the full score, for both the new
  eval and a second eval (by default 08c0f11, before Texel tuning);
- whether the lazy exit fired, and whether it was wrong.

A lazy exit is wrong when the lazy result and the full score fall on different
sides of alpha.

It saves histograms of rest = full − partial, by phase class, trade bucket,
gamePhase and queens. It also saves a sample of records: (alpha, partial, full,
phase class, trades, gamePhase, queens) for every call whose partial score is at
least 100 outside the window.

**analyze** reads a run directory and prints:

- the totals;
- rest quantiles next to the current margin;
- a replay of the records under margin schemes given on the command line.

## Build

`prepare_trees.py` exports `src/` of two commits and patches the copies, never
the shared `src/`:

- the lazy probe goes into both `ueval.cpp` files;
- the second tree's clashing inline names get an `Old` suffix;
- the new tree's `main.cpp` gets the ucinewgame/go hooks for game-like counting.

The probe looks for the 4ce0793 lazy block,
`if ((pas::add(pas::add(termScore, <base>), lazyMargin) < alpha))`, and stops if
it is not there.

```sh
PY=C:/Users/USER/AppData/Local/Temp/petir-protocol-python-3.12.10/python.exe
$PY -I tools/lazy_margin/prepare_trees.py build/<work>/trees --new HEAD   # --second 08c0f11 by default
cmake -S tools/lazy_margin -B build/<work>/lazy -G "Visual Studio 17 2022" -A x64 -DLAZY_TREES=C:/chess/Petir_CPP/build/<work>/trees
cmake --build build/<work>/lazy --config Release -- -m
```

To compare two variants of the same tree, for example one with a changed
`ueval.cpp`, use `--second-src DIR --same-material` and configure with
`-DLAZY_SAME_MATERIAL=ON`. Without `LAZY_SAME_MATERIAL`, the harness recomputes
`materialScore` at the 08c0f11 piece values for the second eval.

## Run

Start runs only when at least two cores are idle (see `MEMORY.md`).

```sh
# Game-like: ucinewgame, search 2 plies earlier, then the position (counted).
$PY -I tools/lazy_margin/lazy_driver.py build/<work>/lazy/Release/lazystat.exe tools/eval_diff/bench-games.json build/<work>/run-gl --jobs 10
# Fresh: one search per position after ucinewgame.
$PY -I tools/lazy_margin/lazy_driver.py build/<work>/lazy/Release/lazystat.exe tools/eval_diff/bench-games.json build/<work>/run-fresh --jobs 10 --fresh
# Analyze: RECORD_SAMPLE must match --sample (default 4), then the schemes.
build/<work>/lazy/Release/analyze.exe build/<work>/run-gl 4 current base:200 baseE:200:200:400 taperQ:220:480:240
```

`--depth-offset` (default +2) is added to each position's bench depth.

## Replay schemes

`trades` is `lazyMarginForTrades` (0 to −120). The phase classes are those of
4ce0793: endgame means both sides have 12 or less non-pawn material; early
endgame means either side has under 16. gamePhase is N/B 1, R 2, Q 4, capped
at 24. Prefix any spec with `o:` to replay it on the second eval.

| Spec | Margin |
|---|---|
| `current` | 280 + trades, +200 early endgame, +600 endgame (4ce0793) |
| `base:B` | `current` with 280 replaced by B |
| `baseE:B:EE:EG` | B + trades, +EE early endgame, +EG endgame |
| `mg:X` | `current`, with the middlegame base X instead of 280 |
| `mgQ:X:Q` | middlegame X (+Q with a queen on the board); other classes as `current` |
| `mgQT:X:Q` | `mgQ` plus trades in the middlegame |
| `mgQE:X:Q:EE:EG` | middlegame X (+Q with a queen); EE + trades early endgame; EG + trades endgame |
| `flat:X` | X everywhere |
| `flatT:X` | X + trades |
| `taper:A:B` | B + (A − B) × gamePhase / 24 |
| `taperT:A:B` | `taper` + trades |
| `taperQ:A:B:Q` | `taper` + Q with a queen on the board |

Output per scheme:

- exits as a % of null-window calls that reach the checkpoint;
- wrong exits, and wrong exits per million of those calls;
- exits and wrong exits by phase class (mg, ee, eg);
- `sevMed`, the median |full − alpha| of the wrong exits.

## Results (2026-10-14/15, 4ce0793)

Game-like run on the 360 bench positions:

| Scheme | Exits | Wrong per million |
|---|---|---|
| 08c0f11 eval, its own margin | 39.84% | 316.4 |
| 4ce0793 (`current`) | 41.51% | 2,378.8 |

At 10+0.1 against 4ce0793, five margins were tested. None gained Elo, and the
original margin was kept.

| Margin | Games | Elo | Note |
|---|---|---|---|
| `taperQ:220:480:240` | 934 | −16.8 | 84 wrong per million |
| `taperQ:90:500:240` | 2,557 | −2.9 | |
| `mgQE:140:200:480:720` | 2,574 | −2.7 | |
| `taperQ:160:480:240` | 2,801 | 0.0 | stopped |
| `baseE:200:200:400` | 1,138 | +0.3 | stopped |

Fewer wrong exits did not turn into Elo, and the wider margins cost 0.01–0.05
plies of mean depth (`tools/pgn_depth.py`).

## Match helpers (tools/)

- `pgn_sprt.py FILE.pgn CANDIDATE`: score, Elo with 95% interval, and the
  SPRT [0, 10] LLR of the finished games so far.
- `pgn_depth.py FILE.pgn`: mean search depth per engine, by move-number band.
- `pgn_phases.py FILE.pgn CANDIDATE [--pylib DIR]`: for each material class
  (queens on, middlegame, endgame), the expected-score drift, the mean depth
  and the decisive turns. Needs python-chess.
