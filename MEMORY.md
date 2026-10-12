# Petir evaluation follow-up

Saved on 2026-10-10 at the user's request for recall in later sessions.

## Current decision

The user wants to test the current changes first. Keep the ideas below as
suggestions only. Do not start another evaluation change without a new request.

## Build to test

- Executable: [petir64_endgame_asymetry.exe](build/windows-x64-endgame-asymetry/Release/petir64_endgame_asymetry.exe).
- Source commit: `eca33516a2df2fc5a41b403fc5ac0a861f407d49`, pushed to GitHub `main`.
- Includes previous approved optimizations, exact C++ KPK, passed-pawn activity,
  endgame rook/knight activity, and conservative KRP-versus-KR scaling.
- Also fixes all three investigated color asymmetries: minor-plus-pawn material
  scaling, the passed-pawn king-distance condition, and connected-passer scoring
  that depended on iteration order and promotion shortcuts.
- Both C++ and original Pascal sources received the latest symmetry fixes.
- Validation: 5,229 regression checks and all three CTest suites passed in Release
  and bounds-checked Release; 2,598 mirrored position pairs passed the audit.
  Pascal was not compiled. Playing strength and NPS impact of the latest
  evaluation changes remain unmeasured; correctness checks do not establish Elo.
- Build details: [results](build/windows-x64-endgame-asymetry/RESULTS.md) and
  [manifest](build/windows-x64-endgame-asymetry/BUILD_MANIFEST.json).

## Remaining evaluation suggestions

These are the later, general evaluation suggestions. Their numbering is separate
from the older tournament endgame suggestions.

1. **Pawn-safe knight and bishop mobility.** Existing general mobility counts
   reachable squares without adequately distinguishing enemy pawn control.
   Discount squares attacked by enemy pawns, especially for knights and bishops.
   Test whether this avoids rewarding activity on squares a piece cannot use.

2. **Useful king activity near ordinary weak pawns.** Existing new king-distance
   scoring concerns advanced passers. Consider king access to vulnerable ordinary
   pawns, defending its own pawns, and safe entry routes. Avoid rewarding central
   placement when the king cannot reach a useful target or is exposed to checks.

3. **Weak-pawn pressure and overloaded defenders.** Extend beyond isolated-pawn
   structure and the current narrow endgame activity terms: consider attackable
   backward/isolated pawns, available defenders, and pieces tied to multiple
   defensive tasks. Avoid counting the same pressure twice or treating every
   geometric attack as a pawn that can actually be won.

4. **Selective opposite-colored-bishop draw scaling.** Investigate replacing the
   existing blanket discount with conditions reflecting remaining pieces,
   blocked pawns, passers, king access, and attacking chances. Opposite-colored
   bishops alone should not imply the same drawing tendency in every position.

5. **Refine the existing bad-bishop penalty.** Petir already has this term.
   Consider fixed pawns on the bishop's color, blocked central pawns, usable
   diagonals, and targets on both wings, rather than adding another blanket
   same-color-pawn penalty that duplicates the existing one.

6. **Smooth phase transitions.** Investigate blending suitable middlegame and
   endgame terms gradually where current material switches cause abrupt changes.
   Keep exact endgame probes and specialized material rules separate. This is
   broader work and should follow smaller, attributable changes.

Suggested order after testing the current build: investigate 1, then 2, then 3;
defer 4-6 until the smaller ideas have evidence. None of these six suggestions
has been implemented as a follow-up to the symmetry fixes.

## Prior decisions to preserve

- Earlier tournament suggestion #1 (passed-pawn safety/king proximity), #3
  (endgame piece activity), and #4 (KRP-KR scaling) were implemented in the
  previous endgame build and are included in the current build.
- Earlier tournament suggestion #2, an endgame king-danger term, was explicitly
  discarded by the user. Do not reintroduce it implicitly. The ordinary-pawn
  king-activity suggestion above is a separate idea.
- KRP-KR uses conservative soft scaling, not an exact tablebase. The exact C++
  KPK table is generated in memory and needs no separate release data file.
- The morning tournament predates the current endgame/symmetry build, so its
  results cannot establish the strength of this new executable.
- Historical tournament analysis: [report](build/tournament-morning-20261010/REPORT.md),
  [loss phases](build/tournament-morning-20261010/LOSS_PHASES.md), and
  [original endgame suggestions](build/tournament-morning-20261010/ENDGAME_SUGGESTIONS.md).
  Those files describe the code at the time of analysis; their suggestion status
  is superseded by the implementation decisions recorded here.
- Links under `build/` refer to local ignored artifacts and may be absent in a
  fresh clone. This memory note preserves the decisions independently of them.

## How to resume

Ask to recall the evaluation suggestions in `MEMORY.md`. When new tournament
results arrive, compare this build with its predecessor using paired openings
and colors; account for repeated opponents/openings when assessing confidence.
For any later change, implement and test one idea at a time, check color symmetry
and speed, then use match results to assess strength.

## Engine fact: stale king bitboards during search

Found on 2026-10-11 while optimizing SEE (`src/usee.cpp`).

- Petir's search reaches boards where a king has been captured, for example a
  white pawn taking the black king on h3. `makemove` leaves the captured king's
  bitboard set (`kingblack` still has h3), while `papan`, `whitepieces` /
  `blackpieces` and the occupancy boards show the capturing piece.
- The original SEE lets this phantom king recapture, valued by `papan` (here as
  a pawn). A rewrite that stopped early on `attack & blackpieces` and took
  attacker values from the bitboard type returned 0 instead of -5, which changed
  the search by 2 nodes on the 360-position bench (middlegame-076, depth 12).
- Speed work must keep search results exactly identical. So an exact rewrite
  must read the same board fields in the same order as the original. Don't
  assume that the side boards match the piece boards, that `papan` matches the
  bitboards, or that `allpieces` matches the rotated occupancy boards.
- Differential tests must include boards after pseudo-legal captures and after
  king captures, not only legal game positions. The SEE harness
  `build/seeopt/see_test.cpp` does this, and it reports mismatches against that
  rewrite.

- Root cause: `isvalid` (`src/valid.cpp:50`, and the Delphi original
  `original/valid.pas:54` and `:78`) accepts a pawn capture whenever the square
  difference is 7 or 9 with an enemy piece on the target. It doesn't check the
  file, so an a- or h-file pawn "captures" around the board edge. Observed: the
  killer move a3->h3 (squares 16->23), a rook move elsewhere in the tree, was
  accepted for the white pawn on a3 and captured the black king on h3. FEN
  before that capture, White to move:
  `2b4r/2r2p1p/2p1q1p1/3pP3/p1p5/P6k/1P3P2/2KR4 w - - 0 1`. `makemove` has no
  king case, so it treats the king as a captured queen, and `kingblack` stays set.
  Fixed in 539f94d (2026-10-11): isvalid now requires a file change of one. Bench at depth +2:
  only middlegame-076 changed (+544 nodes, same best move) and no king captures remain. Other
  routes to a king capture are not proven impossible, so exact rewrites should still read the
  same fields as the original.

## Engine fact: mask initialization order

`petir_bitboard::init_mask` does not set every ray mask: `rook_attack_up_mask` and
`rook_attack_down_mask` are filled later, in `petir_evalmask::initevalmask`. Any table derived
from the ray masks must be built after `initevalmask`. `petir_usee::init_xray` (34e9932, SEE
x-ray tables) is called right after it in `petir::initialize`; called after `init_mask` only,
its vertical rays were empty and the self-test's x-ray walker reported 2 failures.

## Match testing with cutechess-cli

Set up on 2026-10-11. Use this for engine-vs-engine strength tests instead of
Arena: it has much less overhead per game and plays several games at once.

- Tool: `C:\chess\cutechess\cutechess-1.5.1-win64\cutechess-cli.exe` (the GUI
  `cutechess.exe` is in the same folder). Run `cutechess-cli.exe --help` to
  check option names before relying on older docs.
- Openings: `books\8moves_v3.pgn` in that folder (34,700 lines). Always use
  `order=sequential start=1` with `-repeat -games 2`, so every build meets the
  same openings and each opening is played once with each colour. The user
  rejected random order. Raise `start` only when asked for fresh positions.
- Concurrency: `-concurrency 10` by default. The test PC is a Ryzen 9 7900X
  (12 cores / 24 threads). Petir is single-threaded and pondering is off, so
  each game needs about one core. Don't use the SMT threads, because they add
  timing noise. Use fewer only when another match is already running.
- Engines: the test exes live in `C:\chess\arena_3.5.1\Engines\PetirCPP\`.
  Petir speaks UCI. Pass `option.OwnBook=false`, because the engine's own book
  is on by default and would override the test openings.
- Standard settings: `tc=10+0.1 option.Hash=64` (since 2026-10-11; 15+0.1 before), `-draw movenumber=40
  movecount=8 score=10`, `-resign movecount=3 score=600 twosided=true`,
  `-recover`, `-ratinginterval 20`, `-pgnout C:\chess\results\<name>.pgn`.
- SPRT: `-sprt elo0=0 elo1=10 alpha=0.05 beta=0.05` with `-rounds` as a game
  cap tests for a real gain. `elo1=50` only detects big gains and usually
  rejects a true +10. For speedups or simplifications use `elo0=-10 elo1=0`.
- Template: `C:\chess\cutechess\seeprune.bat`. Copy it for a new test and
  change the engine names, exe paths and PGN name; leave the openings alone.
  Don't edit a .bat while it is running, because cmd re-reads the file as it
  executes.
- Throughput: games average about 37 s at 15+0.1, so each concurrent slot
  plays roughly 90-100 games per hour (about 550/hour at 6, about 900/hour
  at 10).
- cutechess prints "Score of A vs B: W - L - D [pct] N", "Elo difference:
  X +/- Y, LOS", and the SPRT llr against its bounds. `Warning: Illegal PV
  move` means the engine printed a PV move that is not legal in the position;
  investigate it, because it can mean the search played a wrong move.

## Running benchmarks and tests: wait for idle cores

Rule from the user, 2026-10-11: start a benchmark or test only when at least
two cores are idle. If fewer are idle, wait and tell the user rather than
starting anyway.

- Several sessions share this machine and the main checkout. Their regression
  runs, eval diffs and cutechess matches load the CPU, and a timing run made
  under that load is not valid.
- Before starting, check the load, for example with PowerShell
  `Get-CimInstance Win32_Process | Where-Object { $_.Name -match 'petir|python|cutechess' }`
  for running engines and drivers, and
  `(Get-Counter '\Processor(_Total)\% Processor Time').CounterSamples.CookedValue`
  for overall CPU use.

## EPD test suites (tactical and endgame positions)

Added 2026-10-12 for checking that a search change doesn't hide tactics or
break zugzwang handling. A suite score is a safety check, not a strength
measure: a stronger build can solve fewer positions. Matches decide.

- Python: the portable one, `C:/Users/USER/AppData/Local/Temp/petir-protocol-python-3.12.10/python.exe`
  (run with `-I`). python-chess is needed only to fetch or convert. It is
  imported from `build/kpk-audit/python-chess-master` when not installed.
- `tools/epd_suites.py fetch [NAME,..]` downloads the suites into
  `build/suites/NAME.json` (expected moves already in UCI; raw files in
  `build/suites/raw/`). `epd_suites.py list` shows what is fetched;
  `epd_suites.py convert FILE.epd NAME` adds a local EPD file.
- Suites: `wac` (300, easy tactics), `arasan` (200, hard), `ecm` (183,
  middlegame tactics), `bt2630` (30), `eet` (100, endgames), `sbd` (134,
  quiet best moves: good for pruning and reduction changes), `bk` (24),
  `kaufman` (24), `lct2` (35), `hard-mates` (24, long mates), `zugzwang` (5,
  null-move failure positions). Sources: the Arasan repo (jdart1/arasan-chess
  `tests/`), ChrisWhittington/Chess-EPDs, and the Chess Programming Wiki.
- `tools/epd_test.py LABEL=EXE [LABEL=EXE ..] --suites wac,sbd,...
  --movetime MS | --depth N [--jobs N --pin K] [--json OUT --report OUT]`
  runs every engine on each position in the same worker, one after the other,
  and reports solved counts, mean time to solution over positions all engines
  solved, and the positions solved by some engines but not all. Petir supports
  `go movetime` and `go depth`, not `go nodes`.
- Compare pruning changes at fixed time (`--movetime`), not fixed depth:
  pruning makes each depth cheaper, so fixed depth hides its cost. With
  `--jobs 10 --pin 2` each worker gets its own physical core (cores 2-20),
  which leaves two cores free. Fixed-time results depend on load: compare only
  builds from the same run.
- First use: `build/rfp/epd/` (RFP none/v1/v2 at 1 s per position).

## Todo queue

Items the user parked to revisit later. Don't start one without a new request,
and remove it from this list once it is done.

1. **Stale `mateThreatExtended` in check** (queued 2026-10-11, found during the
   readability refactor of `src/search.cpp`). In `mainSearch`,
   `searchStack[ply].mateThreatExtended` is reset to false only when the side
   to move is not in check. In check it keeps whatever an earlier node at the
   same ply stored. Child nodes then read the stale value as
   `searchStack[ply - 1].mateThreatExtended` (the razoring guard, the "line
   looks quiet" test in reduction step 1, and reduction step 2) and as
   `searchStack[ply - 2].mateThreatExtended` (reduction step 2). The Delphi
   original (`original/search.pas`) behaves the same. A fix changes the search,
   so judge it with the 360-position bench and a cutechess match, not a parity
   check.

### Colour asymmetries (queued 2026-10-12)

Found by a full colour-mirror sweep on 2026-10-11: each position P was compared
with its mirror (ranks flipped, colours and side to move swapped) over 52,114
positions. Unless noted, each item is inherited from the Delphi original. Fix
them one at a time. A fix changes the search, so judge it with the bench and a
match, not a parity check. Item 5 of that sweep (the bishop-versus-lone-pawn
draw rule in `isTheoreticalDraw`) was fixed on 2026-10-12.

2. **Black king move scored as a knight move** in `blackMoveThreatensKing`
   (`src/tools.cpp` ~1194-1196). It uses `knightAttacks[toSquare]`; White
   (~977-979) uses `kingAttacks`. About 33% of king moves give a different result
   from their mirror. This feeds `isCalmLateQuietMove` and the `threatenedValue`
   futility reductions in `src/search.cpp` (~332, ~378-409). Delphi
   tools.pas:481.
3. **Black bishop loses its x-ray attacks** in the same function
   (`src/tools.cpp` ~1133-1134). The x-ray-through-own-pawn/queen map is
   overwritten by plain diagonal attacks; White keeps it. About 2.7% of bishop
   moves differ.
4. **Evasion king-capture score**: White `pieceOrderValue << 14`
   (`src/movgen.cpp` ~1398), Black `<< 15` (~1667). Evasion scores differ in
   2,220 of 6,467 in-check positions. Delphi movgen.pas:135 vs 466.
5. **Bare-king passer "rule 2" exception** in `src/ueval.cpp`: Black tests its
   own king (`blackKingSquare >= a3`, ~982), White tests the enemy king
   (`blackKingSquare <= h6`, ~1178), which never holds, so White never takes the
   exception. Example `8/8/8/7p/7P/4k3/2Kp4/8 w - - 0 1` scores -290, its mirror
   -920. The comment at ~966-971 describes it. Delphi ueval.pas:1816 vs 2071.
   Four pawn endgames still show a colour gap in eval after a move, for example
   `8/1K1Pk3/8/8/5p1P/8/8/8 w`; they are probably this item, not yet confirmed.
6. **Generator scan order**: Black piece loops mostly use `move_bits<true>`
   (legacy 16-bit-word order), White `move_bits<false>` (ascending). Move sets
   and scores match, but `selectBestMove` takes the first maximum, so ties come
   out in a different order (783 quiet, 1,382 capture, 1,814 capture+push and
   8,500 capture+check lists out of 45,647). This is the main structural reason
   mirrored searches differ. It touches move ordering, so do it in the
   `build/searches` worktree.
7. **WinBoard clock, C++ only** (`src/main.cpp` ~250, side set at ~276): after
   `new`, computerSide is Black, so `time`/`otim` sent before the first `go` as
   White are stored in swapped slots and the first move budgets from the
   opponent's clock. `tighten` still caps with the engine's own time. UCI is not
   affected.
8. **Book-learning weights differ by colour** (`src/opening_book.cpp`
   ~443-490): upset wins add +3/+1 (White) vs +4/+2/+1 (Black); loss penalties
   trigger at 200/100 vs 250/150 Elo. Only `--learn-pgn` uses it. It may be
   deliberate, so ask the user before changing it.
9. **Pinned en passant rejected (not a colour bug, both sides)**:
   `exposesWhiteKing` / `exposesBlackKing` (`src/tools.cpp` ~217-222, ~387-392)
   compute the en passant to-square in the opponent's direction, so search and
   quiescence reject a legal en passant by a pawn pinned along the capture
   diagonal. The root is unaffected. Examples:
   `6k1/6b1/8/4Pp2/8/8/8/K7 w - f6 0 1`, `k7/8/8/8/4pP2/8/6B1/6K1 b - f3 0 1`.

Harmless, no action needed: `lastCaptureType` is 10 after a White promotion
and 11 after a Black one (`src/makemove.cpp` ~110, ~279); every reader treats
them the same.

Mirror harness, to re-check after each fix: compile `src/*.cpp` minus
`main.cpp`/`selftest.cpp` (unity build) with a driver that plays random games
and random endgames. On each position and its mirror, the driver compares eval
(full, null and lazy windows), `isTheoreticalDraw`, SEE, each generator's move
and score set and picked order, `moveThreatensKing`, legality and `makeMove`.
The mirror must copy `whiteCastleType`/`blackCastleType` swapped, because FEN
does not carry them. A copy lives in `build/colour-mirror` (git-ignored):
`harness.cpp`, `CMakeLists.txt`, `targeted.fen` and the post-fix
`result5.txt`. Copy `src/` into `build/colour-mirror/src`, configure with
CMake, and build Release. Run `symmetry_harness.exe 40 < targeted.fen` for the
full run, or `symmetry_harness.exe show "<fen>" ...` to print a FEN's eval next
to its mirror's. Write `targeted.fen` to stdin without a byte-order mark,
because PowerShell piping adds one and the first FEN is then dropped.

### Reverse futility: dropped, revisit later (queued 2026-10-12)

RFP (static null move) was tried and dropped; main and `build/searches` are
back to plain 9990d93/cff1185 with no RFP. Patches kept for a later retry:
`build/rfp/rfp-v1-main.patch` (another session's v1: fires only below a parent
move that went through futility step 1b) and `build/rfp/rfp-v2-searches.patch`
(v2: own lazy eval when the parent didn't evaluate, incoming captures allowed,
1-3 plies, 90 per ply plus the threatened piece). Bench and counters:
`build/rfp/RESULTS.md`. v2 saved about 3-4.5% nodes and 3% time to depth, EPD
suites unchanged, margins 50/70/90 indistinguishable. Match `rfp9990.bat`
(9990d93 + v2 vs 9990d93, 10+0.1) stopped at 532 games: +124 -142 =266, about
-12 Elo, with RFP losing both time forfeits. Why it gains little in Petir: null
move is tried at nearly every non-PV node with a flat 4-ply reduction, so at
1-3 plies it is a quiescence search of about 2.4 nodes that is more accurate
than a static eval; futility step 1b already reduces late quiet moves on the
eval after the move; the static eval is not otherwise computed at interior
nodes (RFP adds an eval call, -1.7% nps); and the lazy eval exit can skip king
safety. Engines that gain from RFP compute and cache a static eval at every
node, use it up to 8-18 plies, and add improving flags and correction history.
Don't retry RFP without that foundation, and only on a new request.

10. **Revisit RFP later.** Candidates once the foundation is there: RFP up to
    6-8 plies (where it replaces a real null-move search, not a quiescence
    call), margins with an improving term, and a full eval instead of the lazy
    exit so king safety counts. Judge each step with the bench and a match.
