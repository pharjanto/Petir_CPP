# Candidate-pawn checks

This optional CMake project builds `evalshow` against the engine's real source.
It compares the actual cache contents with fresh pawn calculations and injects
an independently computed Python term into a second full evaluation. Both
evaluations use identical phase gates, draw scaling and rounding. The Python
checker also requires the term to have no effect outside pawn endings (kings and
pawns only).

From the worktree root, with MSVC and a Python interpreter available:

```powershell
cmake -S tools/candidate_pawns -B out-pawn-check -A x64
cmake --build out-pawn-check --config Release --parallel 2
python -I -B tools/candidate_pawns/candcheck.py PATH_TO_PYTHON_CHESS tools/candidate_pawns/targeted.fen out-pawn-check/Release/evalshow.exe
```

Set `-DPETIR_CHECK_BOUNDS=ON` when configuring to check Pascal array bounds.
The final positional argument to the Python command is the checker executable,
not a baseline engine. Every input is replayed after later pawn keys to check
eviction and reuse. The checker exits nonzero on any mismatch.

The engine's ordinary `--selftest` separately contains explicit expected chess
classifications, full-evaluation/scaling checks and 5,000 pawn-key cache/mirror
checks. The Python model checks implementation consistency; these explicit
fixtures supply the expected chess behavior.

The injection override exists only in `evalshow`, not the released engine.
Match tests are still required to assess playing strength. The conservative
helper allocation, passed-helper discount, blockade factor and narrow
three-pawn breakthrough bonus are evaluation heuristics, not exact endgame
probes.
