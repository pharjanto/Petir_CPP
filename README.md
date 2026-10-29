# Petir 64 — C++20 engine port

Petir was first written using Delphi/pascal programming language in around 2022-2003 for my university final project. First public release was in September 2004. Petir has competed in several CCT tournaments between 2004-2007, with best result came in 2006 CCT8 5/9 and beat Deep Junior in round 2. Last work/release on Petir was in 2007.

This project is to port legacy codes from delphi/pascal to C++ 64 bit and to bring improvements using modern chess programming techniques in the past 20 years since I have been inactive.

Majority of legacy Pascal code and structure is still retained. Old Delphi code has windows GUI on it, this port does not have. Instead, now it support UCI and xBoard protocol whereby previously only xBoard was supported.

The source is editable C++, with no Pascal compiler or Python dependency at runtime. Python is optional for protocol tests and translation provenance tooling.

# Methodologies and AI Policies

ChatGpt is used to port the entire code. 

Both ChatGPT and Claude is used to find bugs in old pascal code that carried to ported C++. 

Claude build tools to automatically collect statistics in various area to find weak spot, inefficiencies and area to improve.

Every changes is directed and supervised by me. The goal is to increase playing strength while preserving the originality as much as possible. 

Every single change is being made one at a time, tested using CuteChess SPRT statistic or as long as I confident it at least does not regress. 

## Features

### Search
- Principal variation search (PVS) with iterative deepening and aspiration windows
- Transposition table, 1–1024 MB (UCI option `Hash`)
- Quiescence search with SEE-based capture filtering and futility pruning
- Check, mate-threat and recapture extensions
- Singular extension with multi-cut
- Repetition and fifty-move rule detection

### Pruning and reductions
- Null-move pruning
- Late move reductions
- Late move pruning
- Futility pruning
- ProbCut
- SEE pruning of losing captures and bad quiet moves

### Move ordering
- Hash move first, then captures ordered by static exchange evaluation (SEE)
- Staged move generation
- Killer moves
- History heuristic and continuation history (previous one and two moves)

### Evaluation
- Hand-crafted evaluation, Texel-tuned on CCRL games and Petir's own gauntlet games
- Static-evaluation correction history (pawn-structure and material tables)
- Tapered middlegame/endgame scoring with material imbalance terms
- Pawn structure: passed, candidate, connected, doubled, isolated and backward pawns, cached in a pawn hash table
- Piece activity: safe mobility, outposts, bishop pair, rooks on open files, trapped pieces
- King safety: pawn shelter and storm, attack-based king danger with safe checks
- Threat detection
- KPK bitbase and scaling for drawish endings

### Protocols and options
- UCI and xboard/WinBoard
- Options: `Hash`, `Contempt`, `Move Overhead`, `MTDf`, `UCI_AnalyseMode`, `Clear Hash`
- Single-threaded
- Windows builds: generic x64 and AVX2 (CPUs from about 2013 on: Intel Haswell or AMD Zen and newer)

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

## Source layout

| Location | Purpose |
|---|---|
| `src/search.cpp`, `root.cpp`, `uiterative.cpp`, `next.cpp` | Original search and move ordering |
| `src/search_tools.cpp`, `search_history.cpp` | Main-search helpers: move encoding and legality tests; history heuristics |
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
