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

Wait for `bestmove`. Send `stop` to interrupt search and `quit` to exit. Options include `Hash`, `OwnBook`, `Contempt`, `MTDf`, `Move Overhead`, and `Clear Hash`.

Supported UCI search controls: `searchmoves`, `depth`, `nodes`, `mate`, `movetime`, `wtime`, `btime`, `winc`, `binc`, `movestogo`, `infinite`, `ponder`, `ponderhit`, and `stop`. Each new search starts iterative deepening at depth 1 and searches root moves at every depth, using the cached root move for ordering. Hash score cutoffs remain enabled below the root. Hash entries are retained between moves and aged for replacement using a generation tag, while move-ordering history uses the original decay and killers reset. Use `Clear Hash` for a search without prior hash entries; a new game also resets move-ordering history. Game history is retained for repetition detection; opening-book replies still bypass ordinary searches. Depth limits are respected, including requests combined with clocks. Interrupted searches retain the last completed iteration's move and report its score and PV before `bestmove`. Replacing a position, starting a new game, or quitting discards the pending result.

`go searchmoves e2e4 d2d4 depth 8` searches only those legal root moves; the list can appear before or after other controls. Castling, en passant, and queen/knight promotions are supported. Rook/bishop promotions and illegal moves in the list are rejected. An empty list returns `bestmove 0000`. Restricted searches also restrict emergency fallback replies and do not store their root score in the hash table. `go nodes 10000` caps the engine's node counter (including quiescence); counts use 64-bit integers, and `nodes 0` returns a legal fallback without searching. PV reconstruction is not charged to an explicit node budget. `go mate 3` searches for a winning mate within three full moves, stopping after a completed iteration finds one or at depth 6. The engine's existing maximum depth of 49 still applies. Mate searches use the normal selective search, not an exhaustive mate solver, and report found mates with UCI `score mate`. These controls bypass OwnBook, reset for each search, and can be combined with depth and clock limits; the first limit reached stops computation. Explicit `infinite` and `ponder` searches hold their result until `stop` or `ponderhit`, respectively, even after computation reaches a node or mate limit.

The default build handles clocks and increments in 64-bit monotonic milliseconds. `Move Overhead` reserves **50 ms by default** (range 0–5000), including for fixed `movetime`. Search preparation counts against the deadline. A soft target and iteration-duration estimate control whether to deepen; direct deadline checks in search enforce a hard ceiling that score drops and move changes cannot extend. When the clock is too low for useful search, the engine immediately returns a validated legal root move. If no iteration completes, the final report supplies a depth-0 static evaluation of the current position and a one-move PV matching the returned move; WinBoard respects `post`/`nopost`. The allocator also banks overhead for future moves in games without increment. Configure the reserve through `setoption name Move Overhead value 100` in UCI or `option Move Overhead=100` in WinBoard if your GUI needs a larger margin. `go ponder` starts its timed budget on `ponderhit`, which continues the existing search at its current depth; depth-only and infinite searches have no clock deadline.

Analysis starts with an empty transposition table, move-ordering and continuation history, killers, search stack, and pawn evaluation caches. This applies to UCI `go infinite` (or an unrestricted bare `go`), searches with `setoption name UCI_AnalyseMode value true`, and WinBoard `analyze`, including restarts after a board change or undo. Search tables work normally within each analysis. The current position, engine options, and actual game history for repetition detection are preserved. UCI analysis bypasses the opening book; set `UCI_AnalyseMode` back to `false` to restore ordinary play with retained tables. Pondering retains its normal behavior.

For **WinBoard**, send `xboard` and `protover 2`. Supported commands include `new`, `force`, `go`, coordinate `usermove`, `setboard`, `time`, `otim`, `level`, `st`, `sd`, `ping`, `post`, `nopost`, `undo`, `remove`, `analyze`, `exit`, `?`, and `quit`. Analysis searches without a clock or playing a move until `exit`; board changes restart analysis. WinBoard `hard`/`easy` are accepted, but automatic WinBoard pondering is not implemented; UCI pondering is available. SAN is not advertised.

Clock updates, posting controls, and unsupported WinBoard commands leave the search running. A late `time` update can tighten the active clock deadline while retaining the overhead reserve. `?` returns a move; `force`, `new`, `setboard`, `result`, undo, and replacement moves discard an active search. WinBoard reports checkmate, stalemate, insufficient mating material, threefold repetition, and the 50-move rule. Period timing wraps the move count for each new period without losing the game's fullmove number. Hash resizing retains the previous usable table if allocation fails. The diagnostic `d` command prints all six FEN fields.



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
