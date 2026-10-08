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

Re-run locally:

```sh
ctest --preset linux-release
./build/linux-release/petir --selftest
```

Or on Windows:

```powershell
ctest --preset windows-x64
.\build\windows-x64\Release\petir.exe --selftest
```
