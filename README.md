# Petir 1 — faithful C++20 engine port

This project ports the **active chess engine core** from your Delphi source to native 64-bit C++. It retains Petir's search, evaluation constants, move ordering, rotated bitboards, hash tables, repetition logic, and binary opening books. No playing-strength changes have been made. The VCL chessboard and desktop dialogs are outside this engine-only port.

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

Wait for `bestmove`. Send `stop` to interrupt search and `quit` to exit. Options include `Hash`, `OwnBook`, `Contempt`, `MTDf`, and `Clear Hash`.

Supported UCI search controls: `depth`, `movetime`, `wtime`, `btime`, `movestogo`, `infinite`, `ponder`, `ponderhit`, and `stop`. Increments are accepted but do not change Petir's original budgeting formula. `searchmoves`, `nodes`, and `mate` limits are not implemented. The original iterative routine starts with depth 4 and 5, so requests below depth 5 still perform those initial iterations unless stopped. Fixed `movetime` adds a wall-clock deadline around the original search.

For **WinBoard**, send `xboard` and `protover 2`. Supported commands include `new`, `force`, `go`, coordinate `usermove`, `setboard`, `time`, `otim`, `level`, `st`, `sd`, `ping`, `post`, `nopost`, `undo`, `remove`, `?`, and `quit`. WinBoard `hard`/`easy` are accepted, but automatic WinBoard pondering is not implemented; UCI pondering is available. SAN and analyze mode are not advertised.

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
| `src/ueval.cpp`, `evalmask.cpp`, `usee.cpp` | Original full evaluation and SEE |
| `src/movgen.cpp`, `makemove.cpp`, `bitboard*.cpp`, `tools.cpp` | Original board and move routines |
| `src/hashing*.cpp`, `repetition.cpp`, `opening_book.cpp` | Original hash and book logic |
| `src/pascal_runtime.hpp` | Fixed-width Delphi semantics and portable I/O |
| `src/engine.cpp`, `main.cpp` | Initialization, FEN input, protocols, and thread lifecycle |
| `original/` | Unmodified Pascal units and full original uploaded archive |
| `tools/translate.py` | Reproducible, Petir-specific source translation |
| `PORTING_NOTES.md` | Exact compatibility changes and retained limitations |
| `VALIDATION.md` | Tests, results, and what has not been verified |

Builds use the checked-in C++ directly. Do not run the translator for a normal build: it regenerates the 24 translated core files and `petir.hpp`, overwriting edits to those files.
