# eval_diff: exact-score tests for evaluation changes

Use these tests when a change to `src/ueval.cpp` must leave every evaluation
score unchanged, for example a speed-up or a readability refactor. Both
harnesses compile the candidate's `ueval.cpp` and the baseline's `ueval.cpp`
into one program and compare them call by call. They stop at the first
difference.

- **evaldiff** (`search_harness.cpp`) is the full UCI engine built from the
  candidate tree. Every `evaluate()` call that search makes also runs the
  baseline evaluation. Each call additionally compares both sides, both
  perspectives, a full window, nine null windows (lazy exits), and the public
  helpers (`connectedPassedPawns`, `passedPawnActivity`,
  `endgamePieceActivity`, `isMaterialDraw`, `isTheoreticalDraw`,
  `rookPawnScale`, `lazyMarginAdjustment`, ...) on every relevant square.
- **evalrandom** (`random_harness.cpp`) covers boards that search does not
  reach. It plays random games from ten seed positions and builds random
  endgame material and random pawn structures with pieces. It also adds the
  board after every pseudo-legal capture by either side, including king
  captures, which leave a stale king bitboard. Every board is also tried with
  other fifty-move counters and castling states. A few captures by the side
  not to move are en passant captures that use the other side's en passant
  file. They also leave bitboards that disagree with the board array. Search
  never makes them, but they are tested too, so `stale` can be slightly
  higher than `kingCaptures`.

Mutation check (2026-10-12): changing one connected-passer bonus by 1 made both
harnesses report a mismatch.

## Build

Use two separate source trees, never the shared `src/`, because other sessions
edit it. A git worktree at the base commit works as the baseline.

```sh
git worktree add --detach build/<work>/base-tree HEAD   # baseline
# candidate: your own worktree or copy with the changed ueval.cpp
cmake -S tools/eval_diff -B build/<work>/eval-diff -G "Visual Studio 17 2022" -A x64 \
      -DCANDIDATE_SRC=C:/chess/Petir_CPP/build/<work>/cand-tree/src \
      -DBASELINE_SRC=C:/chess/Petir_CPP/build/<work>/base-tree/src
cmake --build build/<work>/eval-diff --config Release -- -m
```

Add `-DEVAL_DIFF_CHECK_BOUNDS=ON` in a second build directory for Pascal
array bounds checks.

The candidate's `ueval.cpp` has to keep the public functions that the harnesses
compare (see the declarations at the top of each harness). It may add new ones.

## Run

Start runs only when at least two cores are idle (see `MEMORY.md`).

```sh
PY=C:/Users/USER/AppData/Local/Temp/petir-protocol-python-3.12.10/python.exe
# In search: 360 bench positions plus the order_stats.py suite, at bench depth.
$PY -I tools/eval_diff/diff_driver.py build/<work>/eval-diff/Release/evaldiff.exe --jobs 10 --out build/<work>/diffrun
# Random boards: GAMES ENDGAME_ROUNDS PAWN_BOARDS SEED. Use several seeds.
build/<work>/eval-diff/Release/evalrandom.exe 300 1000 60000 20261012
```

- The driver prints `PASS` or `FAIL`, the total number of calls and
  comparisons, and coverage counts. The coverage counts show how many calls
  reached connected passers, endgame piece activity, passers in endgames,
  isolated and blocked pawns, pawnless boards, minor-only material and stale
  boards. A failing shard's `.err` file holds the `MISMATCH` line and the
  board. `--depth-offset -3` gives a quick run of a few seconds; the default
  depth takes about 10 minutes with 10 jobs.
- `evalrandom` prints `PASS positions=... stale=... kingCaptures=...` and
  exits with code 0. On a mismatch it prints the board and FEN and exits
  with code 1.
- `bench-games.json` is a copy of the 360-position bench
  (`build/order-stats/bench-games.json`). Pass `--positions FILE` to use a
  different list.

These harnesses only test evaluation scores. Also run the search parity check
(360 positions at depth +2, every UCI line identical except timing), the self
test (`petir64 --selftest`) and the timing comparison. `MEMORY.md` lists those
tools.
