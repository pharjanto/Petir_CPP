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
