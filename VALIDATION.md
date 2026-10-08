# Validation of the C++ port

Validation performed on 2026-10-08, Linux x86-64, GCC 13.3, C++20. The native CMake Release build and CTest suite pass. The core self-test performs **214 checks with zero failures**. The UCI/WinBoard integration test passes separately.

| Position / check | Depth | Expected / observed |
|---|---:|---:|
| Start position | 1 | 20 |
| Start position | 2 | 400 |
| Start position | 3 | 8,902 |
| Start position | 4 | 197,281 |
| Start position | 5 | 4,865,609 |
| Kiwipete | 1 | 48 |
| Kiwipete | 2 | 2,039 |
| Kiwipete | 3 | 97,862 |
| Rook/en passant ending | 4 | 43,238 |
| Starting hash | — | 3,423,812,783,342,919,194 |

Kiwipete FEN: `r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1`.

Rook/en passant ending FEN: `8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1`.

The tests also cover:

- Incremental material, all piece bitboards, combined and rotated occupancies, and hash keys against rebuilding the board across a deterministic sequence of 180 legal moves.
- Castling and en passant state updates.
- Input/make handling of all four promotions. Bishop promotion's original missing bishop-count update is an explicitly retained limitation, not silently fixed.
- Bare-king evaluation, repetition insertion/deletion, a legal depth-5 search result, and preservation of the caller's board.
- Compatibility of the unchanged original opening books, including a legal initial book reply, and packed book-weight updates after a hash collision.
- UCI handshake, readiness, hash resizing, opening book on/off, a multi-move position, checkmate/no legal move, FEN en passant/counters, timed search, `stop`, infinite search, and ponder hit.
- WinBoard negotiation, forced moves, undo, ping, search, setboard, and checkmate reporting.

The core and protocol tests also pass with GCC **AddressSanitizer and UndefinedBehaviorSanitizer**, checked Pascal array bounds, and `UBSAN_OPTIONS=halt_on_error=1`. LeakSanitizer was disabled (`ASAN_OPTIONS=detect_leaks=0`) because the execution environment uses tracing that blocks its process inspection. These runs check exercised paths; they do not prove every possible chess position safe.

The Windows binary is a **PE32+ x86-64 console executable**, successfully cross-compiled using Clang/Zig with the same checked bounds and arithmetic settings. It was not executed on Windows. The included MSVC/VS Code configuration was not built with MSVC in this environment.

A running Delphi executable was not available for side-by-side comparison. There is no verified claim of identical move choice, node count, score, speed, or Elo against the Delphi executable. The port preserves the source algorithms and constants, but deterministic initialization of previously unassigned variables, portability guards, compiler arithmetic/rounding, and changed timing can affect particular searches. See `PORTING_NOTES.md`.

## Windows performance comparison

Local MSVC 19.44 x64 Release measurements on 2026-10-08, with bounds checks disabled, 64 MiB Hash, opening books disabled, fresh processes, and fixed depth 12. Medians of three runs with alternating executable order:

| Position | Legacy bit scan | Native bit scan |
|---|---:|---:|
| Start position | 409 ms | 379 ms |
| Kiwipete | 1,878 ms | 1,762 ms |

Node counts, scores, best moves, and principal variations matched for these positions and the rook ending above. The ending completed too quickly for reliable timing. The native scan passed the core regression suite, including exhaustive 16-bit block comparisons and the empty-board case. These measurements compare C++ builds, not Delphi performance, and do not establish gains for every position.

Link-time optimization was also tested separately at depth 11 and increased search time in the opening and middlegame samples, so it remains disabled by default.

## Windows array-access investigation

The promoted-queen position `kn6/pp1Q4/1Q4Q1/4Q3/Q5Q1/2Q5/1Q3Q2/K2Q4 w - - 0 1` reproduced an access at move-list index 101 (original capacity 100). After enlarging the list, evaluation exposed material-scale index 90 (original maximum 41). Both defects were fixed, and the mirrored black position was added to regression coverage. Move-list checks now remain active even with general bounds checks disabled. Black quiet-move generation no longer writes through an integer pointer across record boundaries. Material-scale and king-pressure lookups saturate at their last defined entries.

The expanded core suite passes with general bounds checks on and off, and with MSVC AddressSanitizer plus bounds checks. A 60-ply self-play comparison at fixed depth 8 with books disabled produced identical node counts, scores, and best moves in checked and unchecked builds without a bounds exception. These tests cover the exercised positions; a user's specific failure still requires its FEN or game to reproduce.

Mate-in-one defense regression tests cover back-rank threats for both colors at fixed depth 6. A deterministic 32-game move-generation audit exercised 244 checked positions; the specialized evasion lists matched full capture/quiet generation followed by making each candidate and verifying king safety. The audit passed with general bounds checks on and off. This checks the evasion generator, not a proof that the search never misses a mate threat.

## PGN missed-mate reproduction and fix

Round 1 of the user's PetirCPP game ended with `68.Qa2 Rxh6#`. The reconstructed position before White's move is `7k/1q5p/5r1P/4Np2/R2p1P2/2bP2PK/1p6/1Q6 w - - 4 68`. The port chose another move permitting the same mate in a fresh depth-6 search, with a score of -97 cp. Its main and quiescence searches incorrectly bound the king value to `usee`'s implementation-private 99; root search similarly used `movgen`'s private 99. Delphi callers must use `header`'s exported 20000. Other foreign references to private piece values in search/tools were corrected as part of the same visibility defect, and the translator now restricts cross-unit lookup to interface exports.

The game-position defense and three terminal mate-score tests failed before the binding correction and pass after it. The rebuilt engine chooses `g3g4` at fixed depth 15 and at a one-second UCI limit, allowing the king to escape after `Rh6+`. The expanded core suite passes in performance, checked Release, and MSVC AddressSanitizer builds. These assertions cover avoiding immediate mate, not saving an already materially losing position.

## NPS after removing search move-list clearing

The corrected engine was copied as a baseline before this optimization. Main search no longer zeroes its two 2 KiB move lists at every call, and quiescence no longer zeroes its 2 KiB move list. Generators populate both fields of each counted entry before the search uses it. Full-buffer capacity checks remain enabled. Additional tests compare generator results with initial buffers filled with different byte patterns, and the suite passes in performance, checked, and MSVC AddressSanitizer builds.

Local fixed-depth-12 comparisons with books disabled, 64 MiB Hash, fresh processes, alternating executable order, and three runs per executable yielded the following median times. The confirmation run took place without concurrent build work:

| Position | Baseline | On-demand move lists | NPS increase |
|---|---:|---:|---:|
| Start position | 517 ms | 482 ms | 7.3% |
| Kiwipete | 3,263 ms | 3,047 ms | 7.1% |
| PGN mate defense | 964 ms | 888 ms | 8.6% |

Node counts, scores, best moves, and principal variations matched for all compared searches, including the shorter rook-ending sample. The rook-ending timing was excluded from the percentage claims because it completed in tens of milliseconds. A separate trial replacing legacy population counts with `std::popcount` produced no consistent gain on this MSVC build and was reverted.

Re-run locally:

```sh
ctest --preset linux-release
./build/linux-release/petir --selftest
```

Or on Windows:

```powershell
ctest --preset windows-x64
.\build\windows-x64\Release\petir64.exe --selftest
```

## Windows port/interface regressions

The port/interface core suite performed **251 checks with zero failures** on 2026-10-08. Both CTest tests passed in the default MSVC x64 performance Release build, bounds-checked Release, and MSVC AddressSanitizer with bounds checks enabled.

New coverage verifies shallow depth limits, combined depth/clock limits, restoration of the last completed move after interruption, repeated time-control periods, complete FEN output, and strict insufficient-material classification. Protocol tests exercise WinBoard clock/status messages during search, result cancellation without changing the board, a late low-clock update, draw claims, undo after repetition, zero UCI clocks, and transactional position input.

On Windows, a child-process memory limit forces a real hash-allocation failure. The engine subsequently clears the retained table, completes another search, and successfully resizes it. This test passes in performance and bounds-checked builds. Only this deliberately induced allocation failure is omitted under MSVC AddressSanitizer, whose allocator aborts before C++ can catch `std::bad_alloc`; the remaining protocol tests run there.

The supplied game's position before `41.h4` was replayed in four fresh WinBoard searches with books disabled and a three-second limit. One run received no clock messages; the other three received `time` and `otim` immediately after `go`. All four completed depth 19 and chose `g2f3` (`41.Kf3`), with score -106 cp. Previously, those clock messages immediately stopped the search and produced a different move. This demonstrates the corrected interface behavior for this replay; it does not establish the cause of every decision in the recorded game.

An alternating three-run, fixed-depth-12 comparison against the previous performance executable produced matching node counts, scores, and best moves for all four positions below. The completed-iteration PV display can have a different tail from the previous live root report. Median timings show no meaningful performance regression in these samples; concurrent Arena play limits timing precision.

| Position | Previous performance build | Interface fixes |
|---|---:|---:|
| Start position | 490 ms | 481 ms |
| Kiwipete | 3,049 ms | 3,053 ms |
| Rook ending | 35 ms | 34 ms |
| PGN mate defense | 907 ms | 900 ms |

## Depth-1 searches with cache reuse

On 2026-10-09, iterative deepening was changed to start at depth 1. Search preparation retains previous transposition-table entries and ages their replacement priority, with the original move-ordering history decay restored. Killers reset between searches, and game repetition history is preserved. Opening-book replies still bypass iterative deepening.

The expanded core suite passes **257 checks with zero failures**. It verifies that a previously usable exact hash result survives search preparation and is aged for replacement, that move-ordering history decays and killers reset, that repetition counts are preserved, and that explicit `Clear Hash` removes the retained result. UCI tests cover every depth from 1 to the requested limit with an empty table, then repeat searches without `ucinewgame` or `Clear Hash`: moves and scores match while node counts decrease in both default and MTD(f) modes. WinBoard tests likewise reuse results after undo, with matching moves and scores and fewer nodes. Cached iterations can complete without intermediate PV reports; the final report still honors the requested depth. Core and protocol tests pass in performance, bounds-checked Release, and MSVC AddressSanitizer builds, with the same intentional OOM-test exclusion under AddressSanitizer described above.

The earlier timing measurements used the original depth-4/5 start and do not benchmark this search-control change. No playing-strength improvement is claimed without match testing.

## Castling cutoffs and halfmove-aware hash results

On 2026-10-09, a regression forced a legal castle to be searched first in a root beta cutoff. The previous bounds-checked engine stopped with `Pascal array index 65 outside 0..63`. The root history update now obtains the king from its pre-move source square rather than treating the special destination code as a board square. Forced king- and queen-side castling cutoffs pass for both colors, including history/killer updates and preservation of the input board.

The halfmove issue was reproduced with `7k/8/8/8/8/8/8/KR6 w - - 0 1` at depth 3 with books disabled. Its score was +539 cp. Searching the same board at halfmove counter 99 with the previous hash retained incorrectly returned +539 immediately with zero nodes; clearing the hash produced 0 cp. The corrected engine searches the counter-99 position and returns 0 cp with either retained or cleared hash entries. The mirrored black position and reuse in the reverse direction are covered by protocol regressions.

Each hash entry now stores its halfmove counter in the former unused byte. Tests cover exact, upper-bound, and lower-bound score reuse with matching and differing counters for both sides, counter values 0, 79, 99, and 255, score-dependent null-move restrictions, and PV extraction. Counter mismatches retain only the board's move-ordering hint. The 16-byte entry size, default 64 MiB table, original board hash, book keys, and repetition keys are preserved.

The expanded suite passes **327 core checks with zero failures**, plus the protocol regressions, in the performance Release, bounds-checked Release, and MSVC AddressSanitizer builds. The intentional AddressSanitizer OOM-test exclusion described above still applies. These changes address the castling cutoff access and cached-score counter mismatch; the inherited halfmove-field width and other search-rule limitations remain as documented in `PORTING_NOTES.md`.

## En passant evasions and quiescence stalemate

On 2026-10-09, explicit fixtures reproduced illegal en passant evasions for both colors. In `4r2k/8/8/3pP3/4K3/8/8/8 w - d6 0 1`, capturing the checking pawn with `e5d6` exposes the king to the rook. The original evasion generator admitted the move, and quiescence left its destination variable at zero while in check, bypassing the post-move en passant safety check. The mirrored black fixture reproduced the same defect. Separate fixtures showed that removing the illegal en passant candidate can reveal actual checkmate.

Both evasion generators now verify the en passant pawn and target, make candidates on copies, and check king safety before including them. Tests compare their raw evasion lists with independent full capture/quiet generation followed by king-safety filtering. They cover pinned captures, file-a/file-h boundaries, unrelated en passant markers, legal captures of checking pawns, and legal en passant interpositions against bishop checks, for both colors. Quiescence also decodes the destination on checked nodes. The checkmate fixtures now return -19998 cp at ply 2.

Quiescence now tests for a legal move before stand pat, including cached evaluations and the depth-63 fallback. Eight stalemate fixtures cover kings alone, a pinned bishop with a pseudo-legal capture, blocked pawns, and pinned pawns with empty forward squares. Each returns the draw score with checks enabled, in capture-only quiescence, with a cached evaluation that would otherwise cause a beta cutoff, and at the depth limit. Other fixtures ensure that a legal quiet move, promotion, or en passant capture prevents a false draw. A deterministic audit compares the quiescence probe with independently filtered legal moves in **4,575 nonchecked positions**; probes preserve search node counts. The existing evasion audit still covers 244 checked positions.

The suite passes **440 core checks with zero failures**, plus UCI/WinBoard protocol regressions, in performance Release, bounds-checked Release, and MSVC AddressSanitizer builds. The intentional AddressSanitizer OOM-test exclusion described above still applies.

The additional legality check has a runtime cost. A local fixed-depth-12 comparison used fresh processes, disabled books, 64 MiB Hash, alternating executable order, three runs per executable, and no concurrent build work:

| Position | Before these fixes, median | Corrected, median |
|---|---:|---:|
| Start position | 570 ms | 576 ms |
| Kiwipete | 2,797 ms | 2,909 ms |
| PGN mate defense | 1,331 ms | 1,440 ms |

Best moves and scores matched in these samples. Node counts matched for the first two positions; the PGN position changed from 7,949,107 to 7,949,444 nodes. The short rook-ending sample (45 versus 46 ms) was excluded from percentage comparisons. These timings describe this machine and sample; they do not establish playing-strength improvement.
