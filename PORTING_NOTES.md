# Faithfulness and compatibility notes

This is a source-level port of the active Petir 1 engine, not a new chess engine. The search and evaluation were translated unit by unit. The separate engine interface replaces VCL forms, Windows timer events, and Delphi threads. The original GUI and inactive experimental evaluation/move-generation variants are retained in the original uploaded archive, but are not part of the C++ engine build.

## Semantics retained

- `Integer`/`Longint` remain 32-bit, `Byte` 8-bit, `Word` 16-bit, `Shortint` signed 8-bit, and `Smallint` signed 16-bit, on a 64-bit process.
- Int64 bitboards and keys use unsigned 64-bit storage to preserve bit patterns, modular multiplication, and logical right shifts without signed overflow. No pointer is narrowed to an integer.
- The original arbitrary array bounds, including negative piece indices and 1-based move lists, are retained by `pas::Array`. Strings use Pascal's 1-based indexing.
- Delphi overflow-check-disabled integer arithmetic is represented by explicit modular helpers. GCC/Clang builds also disable strict aliasing because the original bit-scan/popcount routines inspect 64-bit values through 16-bit pointers.
- Boolean expressions retain the original short-circuit behavior and Pascal operator precedence. Records and positions retain value-copy semantics at the same call sites.
- The fractional-ply scale, all search extensions/reductions, aspiration windows, null move, futility conditions, SEE, killers/history, root move order, and full classical evaluator are unchanged.
- The material and piece-square values, including asymmetries between colors, are retained. No tuning, bitboard redesign, pruning consolidation, new search heuristics, or strength fixes were applied.
- Delphi unit visibility is preserved when binding symbols: implementation-private piece values in `movgen` and `usee` do not shadow `header`'s centipawn values in callers. The initial translator violated this rule, causing mate scores around -99 instead of -20000 and incorrect capture/threat scaling. Search, root, and tools bindings and the translator lookup have been corrected.
- Packed book records remain 10 bytes (`uint64 key`, `uint16 weight`). The TT record remains packed, 16 bytes, with the original 16-bit move field.
- Hash initialization reproduces Delphi's LCG, seed 99999, unused-square RNG consumption, and **right-to-left** evaluation of the three Random calls in each key expression. The starting hash is `3423812783342919194`, matching the original source's book comment and supplied book keys. Opening move selection remains weighted and randomized after hash initialization.
- `lastbitp` retains its unusual original behavior: it searches the highest occupied 16-bit block and takes that block's *least* set bit. It was not replaced with a conventional most-significant-bit scan.
- `firstbitp` uses C++20 `std::countr_zero` instead of the legacy 16-bit lookup. It preserves the least-set-bit index and the original zero result for an empty board; the self-test compares all 65,535 nonzero word values in each of the four blocks.
- The translation uses the original build definitions `INDEBUGA`, `CEKMOVEa`, and `TESTING`, plus unit-local directives. The inactive `stat`, `calleval`, and `CEKMOVE` branches remain inactive.

## Explicit compatibility changes

| Change | Reason / effect |
|---|---|
| Initialize C++ local scalars and return values to zero | Delphi leaves some locals and some early-return values unassigned. Reproducing stack garbage would create C++ undefined behavior. This can change affected paths, notably checked-node quiescence locals; there is no defined Delphi value to preserve. |
| Stop quiescence at cancellation | Lets the console worker acknowledge `stop` and fixed-time deadlines. This affects interrupted searches; the adapter returns a legal fallback when no root move completed. |
| At depth 63, return the existing evaluation | Prevents the original 64-ply arrays from being indexed outside their range. Normal searches below this boundary keep the original logic. |
| Optional Pascal array checks, disabled by default for performance | `PETIR_CHECK_BOUNDS=ON` detects an overflow rather than corrupting memory. With checks enabled, the console reports an error and returns a legal fallback if an original capacity is exceeded. |
| Move lists enlarged to 256 with an unconditional capacity check; black quiet moves use record indexing | Positions with promoted pieces can exceed the original 100 moves. Raw pointer writes bypassed Pascal bounds checks. The capacity check remains active in performance builds. |
| Material-scale and king-pressure evaluation indices saturate at the last table entry | Promotions can exceed the material tables' index 41; high king pressure can exceed index 63. Values within the original ranges are unchanged. |
| Main and quiescence search move lists are filled on demand rather than zeroed at entry | Generators assign both move and score for each counted entry before ordering reads it. Scalar initialization and unconditional move-list capacity guards are retained. Regression tests compare generator output from different initial byte patterns. |
| Standard-library worker, atomic stop/time counters, monotonic timer | Replaces VCL threads and the 250 ms timer. Search remains single-threaded. Modern thread scheduling and hard `movetime` limits can change the last completed iteration. |
| Console UCI/WinBoard adapter | Replaces GUI actions. FEN input is validated and restores en passant and counters; legacy `readnotation` remains in the translated core. Protocol move input is checked using the original generators. |
| Depth-1 start, completed-iteration results, and explicit depth limits | Each search begins at depth 1. Hash entries are retained and aged between moves, with the original history decay and killer reset. Game repetition history is preserved. Depth limits, including limits combined with clocks, are honored. An interrupted iteration cannot replace the last completed move, score, or PV. |
| WinBoard search lifecycle, period clocks, and game results | Routine clock/status commands leave search running; position changes and `force` discard its result. Move counts wrap for each time-control period. The adapter claims actual-game draws independently of the core's heuristic draw evaluation. |
| Transactional hash resizing and position input | Allocation or input failure preserves the previous usable table or position. Diagnostic FEN output includes all six fields. |
| Portable book/PGN file operations and paths | Removes form and Windows-path dependencies; book bytes and learning logic are unchanged. Missing files/paths are reported by the console where appropriate. |
| Omit GUI-only diagnostics and `TStringList` startup/finalization | Removes desktop captions and the original `C:\anu.txt` write. The translated diagnostic routines remain available but inactive. |

The console reconstructs repetition entries with the original **0/1 side indices**; search uses the same `2-giliran` convention. A new game does not pre-insert its initial position, matching the original GUI initialization.

## Original limitations intentionally retained

These are future improvement candidates, not changes made in this version:

- The generator searches queen and knight promotions only. The maker can accept bishop/rook promotions from an opponent, as the original did. Standard perft positions that require all four generated promotions can therefore differ from published chess totals.
- Bishop promotion does not increment `ngajahputih`/`ngajahhitam`. The original missing bishop-count update is retained and tested explicitly.
- The TT's 16-bit move field can truncate the rook-promotion encoding, which needs bit 16. The field was not widened in this faithful version.
- Repetition-table deletion still uses the original open-addressing behavior without tombstones.
- Mate scores retain their original TT encoding and root conventions; no ply normalization was added.
- The original `IsValid` pawn-file behavior, SEE treatment of pins/kings, check-evasion logic, and search pruning quirks are retained.
- Book candidates remain limited to 20 entries. Optional bounds checks detect exceeding that capacity. Move lists now have 256 entries and an unconditional capacity check.
- MTD(f) retains its original byte-sized depth parameter and is off by default.
- Node counters remain 32-bit, and the halfmove counter remains an 8-bit field.
- The legacy PGN learner is not a general PGN parser.

Because the Delphi application could not be compiled/run here, this is **not a claim of proven bit-for-bit search parity** against a running Delphi build. The original source, translation tooling, structural review, book-key compatibility, perft results, board-state invariants, and functional tests are provided to make that comparison practical.
