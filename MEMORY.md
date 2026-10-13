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

## Texel tuning data (started 2026-10-14)

Positions labelled for tuning the evaluation. Labels will blend the game
result with a Stockfish score; the blend weight is chosen once a tuner exists.

- Source: the CCRL 40/15 database (2,463,682 games), extracted to
  `build/texel/ccrl/CCRL-4040.pgn`. Windows `tar` can't decode its PPMd
  compression; the 7-Zip in `C:\Program Files\NVIDIA Corporation\NVIDIA app\`
  can. Keep the Arena tournament games (`C:\chess\arena_3.5.1\Tournaments\pgn`)
  out of training: they are the held-out check.
- `tools/texel/sample_positions.py` samples positions (seeded, spread over
  the whole file, book plies skipped, cheap quiet filter, duplicates removed).
  `tools/texel/label_stockfish.py` labels them with Stockfish 14
  (`C:\chess\arena_3.5.1\Engines\stockfish_14_win_x64_avx2\`), keeping the
  score of every depth from one search. `tools/texel/pilot_report.py`
  summarises a labelled set.
- Pilot (`build/texel/pilot/REPORT.txt`, 1,000 positions to depth 20): the
  fitted K is about 1.62; on 10 cores depth 14 labels about 490,000 positions
  an hour, depth 16 about 205,000, depth 18 about 92,000. The fit to game
  results stops improving at depth 14.
- Quiet filter, run after sampling and before labelling:
  `tools/texel/quiet_filter.py QUIET_EVAL IN OUT [--rejected FILE]`. It keeps
  a position when Petir's full-window quiescence search (captures and pushes)
  returns its static eval, and drops in-check positions and exact-rule scores
  (KPK, material and theoretical draws, rook + minor against rook). Build
  `quiet_eval` from a separate tree whose eval will be tuned:
  `cmake -S tools/texel/qfilter -B build/texel/qfilter -G "Visual Studio 17 2022" -A x64 -DPETIR_SRC=<tree>/src`
  (currently built from `build/base-08c0f11`). It scores about 10,000
  positions a second. On the pilot it kept 87.5%. On the positions it dropped,
  the quiescence score agreed better with Stockfish than the static eval did
  (correlation 0.74 against 0.65). Don't use `--checks`: it dropped 7% more,
  mostly for small eval shifts (median 15 cp) rather than tactics, and the
  dropped positions were the more decided ones.
- On the pilot's kept positions, squared error against game results: constant
  0.124, material count 0.111, Petir's static eval 0.100, Stockfish depth 16
  0.056.
- Plan agreed with the user (2026-10-14), two tuning rounds on the same
  labelled data, each with its own match:
  1. Tune the current eval (no taper): about 750 additive weights (material,
     piece-square tables, outposts, mobility, pawn structure, passers, bishop,
     knight and rook terms, king-safety tables). Keep fixed: scale factors
     (endgameScale, rookScale, king-attack multipliers, draw divisors, 50-move
     fade), rare terms (trapped knight, KRP-KR, bare-king promotion rules) and
     exact rules. The pawn-shield hole counts and king-pressure counts stay
     fixed too: they index pawnShieldPenalty and kingPressurePenalty, whose
     entries are tuned instead. Match against base.
  2. Only if the user decides to: untie middlegame and endgame values for
     material, piece-square tables and mobility, blend them by non-pawn
     material in the engine (this revives suggestion 6, smooth phase
     transitions, which was shelved),
     retune, and match against round 1. Measure nps: it is a structural change.
  The tuner keeps a middlegame and an endgame value per weight from the start,
  tied together in round 1, so round 2 needs no rewrite. Piece values also
  steer search (quiescence pruning margin, threat detection in tools.cpp),
  and the lazy-eval margin (280 plus adjustments) needs a recheck if tuned
  terms grow.
- Eval weights for tuning (2026-10-14): the numbers ueval.cpp used to write
  inline (side to move 10, +4/+8 per pawn, queen against pawns 35, safe
  advance on move 40, staggered connected passers, passer activity, endgame
  knight and rook activity, breakthrough 16) are named weights in
  petir.hpp. Scalar weights are declared `PETIR_EVAL_WEIGHT`: `inline
  constexpr` in the engine, plain variables when PETIR_EVAL_TUNING is
  defined, which also makes the pawn caches recompute every entry.
- `tools/texel/tuner/eval_trace` (built with PETIR_EVAL_TUNING, same CMake
  pattern as the quiet filter) lists the 744 tunable weights (`--list`) and
  writes, per FEN, the White-view eval and each weight's coefficient
  (d eval / d weight, measured by changing the weight by 16). It ties White
  and Black tables (and the kingside/queenside endgame king tables) into one
  weight and checks they are mirrored. Material is traced through the
  position's running material score. The linear model holds:
  `--check` with every weight changed by up to 10 predicted the eval within
  1 cp in 98% of pilot positions (mean error 0.36 cp). About 33 non-zero
  coefficients per position; 875 positions take 0.15 s.

## Exactness tests for speed-ups and refactors

Added 2026-10-12 during the exact-score `ueval.cpp` optimization. Use these
tests whenever a change must leave scores and search output unchanged. Build
the baseline and the candidate from separate trees, for example
`git worktree add --detach build/<work>/base-tree HEAD`, never from the shared
`src/`. Run them only when at least two cores are idle.

- `tools/eval_diff/` (see its README.md) compares evaluation scores call by
  call. It is a CMake project with `-DCANDIDATE_SRC=<tree>/src` and
  `-DBASELINE_SRC=<tree>/src`, and it builds two harnesses:
  - `evaldiff` is the UCI engine. Every `evaluate()` call that search makes
    also runs the baseline evaluation, with extra windows, sides and helper
    checks. Drive it with `tools/eval_diff/diff_driver.py EXE --jobs 10`,
    which prints `PASS` or `FAIL` and coverage counts.
  - `evalrandom GAMES ROUNDS PAWN_BOARDS SEED` covers random games, random
    endgames, random pawn structures, and boards after every pseudo-legal
    capture, including king captures (stale king bitboards).
  - A mutation test (one bonus changed by 1) failed in both harnesses.
    `-DEVAL_DIFF_CHECK_BOUNDS=ON` builds with Pascal bounds checks.
- `tools/search_parity.py BASE_EXE CAND_EXE [--depth-offset 2]` compares UCI
  output on the 360-position bench (`tools/eval_diff/bench-games.json`),
  with a fresh hash per position and with the hash carried over (22 shards).
  It removes time, nps and hashfull from each line and prints `PASS` when
  everything else is identical.
- Speed: `tools/order_timing.py BASE CAND --positions tools/eval_diff/bench-games.json
  --depth-offset 2 --shard K/6 --core 2K --trials 3 --json tK.json`, run as 6
  shards in parallel, one pinned core each. Sum the medians of the shards.
  Judge a speed change only by engine nps against a baseline built from the
  same commit, since micro-benchmarks have not predicted engine nps here.

## Todo queue

Items the user parked to revisit later. Don't start one without a new request,
and remove it from this list once it is done.

### Colour asymmetries (queued 2026-10-12)

Found by a full colour-mirror sweep on 2026-10-11: each position P was compared
with its mirror (ranks flipped, colours and side to move swapped) over 52,114
positions. Unless noted, each item is inherited from the Delphi original. Fix
them one at a time. A fix changes the search, so judge it with the bench and a
match, not a parity check. On 2026-10-12 items 2, 3, 4, 5, 7 and 9 of the sweep,
the stale `mateThreatExtended` in check, and a wrap bug in the bishop x-ray
step past an edge pawn were fixed and committed (4ac267e..e6fc329), after a
cutechess non-regression match (+5.3 Elo [-6.1, +16.7] at 1,902 games). Item 6
(generator scan order) was done on 2026-10-13 by b13b386: every generator now
scans forward-first (White rank 8 down to 1, Black the exact mirror, files a to
h), +10.7 Elo [-3.5, +25.0] at 1,203 games.

Dropped by the user on 2026-10-12, no action: book-learning weights differ by
colour (`src/opening_book.cpp` ~443-490). Only `--learn-pgn` uses it.

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

### Butterfly history decay (queued 2026-10-13)

11. **Try a milder decay of butterfly history between moves.** `prepareSearch`
    (`src/engine.cpp` ~170) divides `whiteHistory`/`blackHistory` by 256 at the
    start of every search, so an entry at the 16384 limit drops to 64 and each
    move's search starts with almost no butterfly history. Continuation history
    is never decayed (cleared only by `newGame`). Candidates: /2, /4, /8. The
    only earlier test (2026-10-10, `build/order-stats/malus/RESULTS.md`) ran /2
    on the game-like bench with `--warmup 2`: one earlier search 2 plies back,
    depth 12, -1.3% nodes (-3.5% to +1.0%), judged no difference. No match was
    played, and one short warm-up search builds little history to carry over.
    Judge it with a match.

### King activity against weak pawns: parked (queued 2026-10-13)

12. **Retest the king-target term after a candidate-passer term exists.**
    Eval suggestion 2, limited by the user to endings with only kings, pawns
    and knights. Worktree `build/kingtarget` (branch `kingtarget` from b13b386,
    uncommitted; patch `build/kingtarget-endings/kingtarget2.patch`). It gives
    the king that gets next to an enemy pawn that no pawn defends and that is
    not passed, before the enemy king does, 40/30/20/10 for 0-3 steps in pawn
    endings and 24/18/12/6 with knights; half at a one-step lead, half again if
    an enemy knight defends the pawn. Results: normal openings (kingtarget2.pgn)
    -7.4 Elo [-18.6, +3.8] at 1,553 games, H0 for [0,10]; pawn-ending start
    positions (kingtarget_pawn.pgn, 352 positions x 2 colours) 49.6%, with 24
    pairs base win + draw against 19 the other way. Lost game r55 shows the
    flaw: the term pulled Black's king to a weak c6 pawn while White's g4+h4
    against h6 became a passer on the other wing. Petir has no general
    candidate-passer term (`potentialPasserBonus` covers only a7/b7/g7/h7
    patterns while pieces remain), so the user parked this to add a
    candidate-passer term first. Start positions, pair and divergence scripts:
    `build/kingtarget-endings`.

### Candidate-passer term: kept in pawn endings (2026-10-14)

13. **Revisit the candidate-passer (pawn-potential) term in other endings.**
    Committed 08c0f11 for pawn endings only (kings and pawns, no pieces);
    independent checker in `tools/candidate_pawns`. Applied in all endgames
    (both sides <= 12 piece material, uncommitted worktree `build/candpasser`)
    it lost the 10+0.1 match `candpotential.pgn`: H0 after 1,965 games, -5.1
    Elo [-16.6, +6.3], although fixed-depth ending start positions leaned
    positive (pawn d14 +3.0, d12 +11.8; knight d14 +10.2, d12 -0.7, all within
    the error bars). The pawn-only match `candpawnonly.pgn` was +3.0 Elo
    [-5.3, +11.3] at 3,536 games when the user kept it; only ~7% of games reach
    a pawn ending. Item 12 can now be retested on top of it.
