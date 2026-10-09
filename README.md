# Petir 1 — faithful C++20 engine port

This project ports the **active chess engine core** from your Delphi source to native 64-bit C++. It retains Petir's classical search, evaluation constants, move ordering, rotated bitboards, repetition logic, and binary opening books. The console adapter adds modern time management and the compatibility fixes described in `PORTING_NOTES.md`. Playing strength requires match testing. The VCL chessboard and desktop dialogs are outside this engine-only port.

The source is editable C++, with no Pascal compiler or Python dependency at runtime. Python is optional for protocol tests and translation provenance tooling.

## Open and build in VS Code on Windows

1. Extract the archive and open the `Petir_CPP` folder in VS Code.
2. Install the **C/C++** and **CMake Tools** extensions. The project recommends both.
3. Install **Visual Studio 2022 Build Tools** with **Desktop development with C++**, the x64 compiler, Windows SDK, and CMake tools. The supplied Windows preset targets that toolchain. If you use another compiler version, create/select its x64 CMake preset instead.
4. Run **CMake: Select Configure Preset**, select `windows-x64`, then **CMake: Configure** and **CMake: Build**.
5. The native build is `build/windows-x64/Release/petir64.exe`. Run `--selftest`, or load that executable in a chess GUI as a UCI engine.

You can also build from a terminal in the project folder:

```powershell
cmake --preset windows-x64
cmake --build --preset windows-x64
ctest --preset windows-x64
.\build\windows-x64\Release\petir64.exe --selftest
```

Builds disable Pascal array bounds checks by default for performance. To enable them for troubleshooting, configure with `cmake --preset windows-x64 -DPETIR_CHECK_BOUNDS=ON` (or use the equivalent option with your Linux preset). Reconfiguring with the standard preset restores the performance default.

The included debugger configurations run the self-test. Choose a Debug build configuration to step through the engine with useful symbols. You can change the launch arguments to `--perft` or `--eval`.

Official setup references: [VS Code CMake](https://code.visualstudio.com/docs/cpp/cmake-quickstart) and [Microsoft C++ in VS Code](https://code.visualstudio.com/docs/cpp/config-msvc).

## Included executables

- `bin/windows-x64/petir.exe`: Windows x64 console executable, cross-compiled with Clang/Zig. **Compilation was verified; execution on Windows was not available here.** It uses the Windows Universal CRT.
- `bin/linux-x64/petir`: Linux x86-64 executable, built and tested with GCC 13.3 on Ubuntu 24.04. Rebuild on another Linux distribution if its system libraries differ.

Each executable has its opening books in an adjacent `books` folder. The engine is a console program: double-clicking it does not open a chessboard. Add it to a chess GUI as a **UCI engine**, or run it in a terminal.

## Build on Linux

Use a 64-bit C++20 compiler and CMake 3.21 or later for the presets:

```sh
cmake --preset linux-release
cmake --build --preset linux-release
ctest --preset linux-release
./build/linux-release/petir --selftest
```

A conventional `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release` build also works with CMake 3.20 or later. Only the C++ standard library and system threading library are required.

## Use the engine

The default protocol is **UCI**. For example, run the executable and enter:

```text
uci
isready
setoption name OwnBook value false
position startpos moves e2e4 e7e5
go depth 6
```

Wait for `bestmove`. Send `stop` to interrupt search and `quit` to exit. Options include `Hash`, `OwnBook`, `Contempt`, `MTDf`, `Move Overhead`, and `Clear Hash`.

Supported UCI search controls: `depth`, `movetime`, `wtime`, `btime`, `winc`, `binc`, `movestogo`, `infinite`, `ponder`, `ponderhit`, and `stop`. `searchmoves`, `nodes`, and `mate` limits are not implemented. Each new search starts iterative deepening at depth 1 and searches root moves at every depth, using the cached root move for ordering. Hash score cutoffs remain enabled below the root. Hash entries are retained between moves and aged for replacement using a generation tag, while move-ordering history uses the original decay and killers reset. Use `Clear Hash` for a search without prior hash entries; a new game also resets move-ordering history. Game history is retained for repetition detection; opening-book replies still bypass search. Depth limits are respected, including requests combined with clocks. Interrupted searches retain the last completed iteration's move and report its score and PV before `bestmove`. Replacing a position, starting a new game, or quitting discards the pending result.

The default build handles clocks and increments in 64-bit monotonic milliseconds. `Move Overhead` reserves **50 ms by default** (range 0–5000), including for fixed `movetime`. Search preparation counts against the deadline. A soft target and iteration-duration estimate control whether to deepen; direct deadline checks in search enforce a hard ceiling that score drops and move changes cannot extend. When the clock is too low for useful search, the engine immediately returns a validated legal root move. If no iteration completes, the final report supplies a depth-0 static evaluation of the current position and a one-move PV matching the returned move; WinBoard respects `post`/`nopost`. The allocator also banks overhead for future moves in games without increment. Configure the reserve through `setoption name Move Overhead value 100` in UCI or `option Move Overhead=100` in WinBoard if your GUI needs a larger margin. `go ponder` starts its timed budget on `ponderhit`, which continues the existing search at its current depth; depth-only and infinite searches have no clock deadline.

For **WinBoard**, send `xboard` and `protover 2`. Supported commands include `new`, `force`, `go`, coordinate `usermove`, `setboard`, `time`, `otim`, `level`, `st`, `sd`, `ping`, `post`, `nopost`, `undo`, `remove`, `?`, and `quit`. WinBoard `hard`/`easy` are accepted, but automatic WinBoard pondering is not implemented; UCI pondering is available. SAN and analyze mode are not advertised.

Clock updates, posting controls, and unsupported WinBoard commands leave the search running. A late `time` update can tighten the active clock deadline while retaining the overhead reserve. `?` returns a move; `force`, `new`, `setboard`, `result`, undo, and replacement moves discard an active search. WinBoard reports checkmate, stalemate, insufficient mating material, threefold repetition, and the 50-move rule. Period timing wraps the move count for each new period without losing the game's fullmove number. Hash resizing retains the previous usable table if allocation fails. The diagnostic `d` command prints all six FEN fields.

Cached hash scores require the same halfmove counter as the current position. A matching board with a different counter can supply a move-ordering hint, but cannot supply a score cutoff or a score-dependent pruning flag. Hash entries remain 16 bytes, so the default table remains 64 MiB.

En passant check evasions must leave the king safe, including when the capture blocks a sliding-piece check. Quiescence recognizes stalemate before returning a static or cached evaluation.

The positional quiescence restart is disabled by default (`PETIR_Q_POSITIONAL_EXTENSION=OFF`), including in all supplied presets. Check evasions and the other quiescence search behavior remain active. To enable the positional restart for a comparison, configure a separate build directory with `-DPETIR_Q_POSITIONAL_EXTENSION=ON` and optionally choose a distinct executable name with `-DPETIR_OUTPUT_NAME=petir64-qextension`. The supplied presets explicitly set the option to `OFF`, including when reconfiguring an existing build directory.

The 4.39 history-based quiet-move reduction is always enabled in the C++ engine. The reduction build option and comparison presets have been removed. The standard Windows preset produces `build/windows-x64/Release/petir64.exe`.

Stage-1-only lazy evaluation is the default for all C++ builds, including the standard `windows-x64` and Linux presets. The first lazy checkpoint and its margin are unchanged; stages 2 and 3 have been removed, so evaluation completes all remaining terms whenever stage 1 does not return. Rebuild the standard Windows preset to use this evaluation in `build/windows-x64/Release/petir64.exe`.

The optional `windows-x64-stage1` preset builds the same evaluation under the separately named **Petir Stage1** executable:

```powershell
cmake --preset windows-x64-stage1
cmake --build --preset windows-x64-stage1
ctest --preset windows-x64-stage1
```

The executable is `build/windows-x64-stage1/Release/petir-stage1.exe`, with its opening books in the adjacent `books` directory.

Command-line utilities:

```text
petir --selftest
petir --perft 5
petir --perft 3 "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"
petir --eval "7k/8/8/8/8/8/8/K7 w - - 0 1"
petir --book-dir path/to/books
```

The legacy PGN learner is retained as `--learn-pgn FILE`. It keeps the original simple PGN assumptions and writes `white.book` and `black.book` in the selected book directory. Use copies of your books in a separate directory when learning, e.g. `petir --learn-pgn games.pgn --book-dir copied-books`.

## Source layout

| Location | Purpose |
|---|---|
| `src/search.cpp`, `root.cpp`, `uiterative.cpp`, `next.cpp` | Original search and move ordering |
| `src/quiescence.cpp` | Quiescence search and its stalemate legality probe |
| `src/ueval.cpp`, `evalmask.cpp`, `usee.cpp` | Original full evaluation and SEE |
| `src/movgen.cpp`, `makemove.cpp`, `bitboard*.cpp`, `tools.cpp` | Original board and move routines |
| `src/hashing*.cpp`, `repetition.cpp`, `opening_book.cpp` | Original hash and book logic |
| `src/pascal_runtime.hpp` | Fixed-width Delphi semantics and portable I/O |
| `src/engine.cpp`, `main.cpp` | Initialization, FEN input, protocols, and thread lifecycle |
| `src/time_manager.cpp`, `time_manager.hpp` | Millisecond clock allocation, soft targets, and hard deadlines |
| `original/` | Unmodified Pascal units and full original uploaded archive |
| `tools/translate.py` | Reproducible, Petir-specific source translation |
| `PORTING_NOTES.md` | Exact compatibility changes and retained limitations |
| `VALIDATION.md` | Tests, results, and what has not been verified |

Builds use the checked-in C++ directly. Do not run the translator for a normal build: it regenerates the translated core source files and `petir.hpp`, overwriting edits to those files.
