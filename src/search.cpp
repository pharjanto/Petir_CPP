// Faithful translation of search.pas; Delphi source is in original/.
// The main alpha-beta search. Helpers live in search_tools.cpp (move encoding,
// sides, legality) and search_history.cpp (history heuristics).
#include "petir.hpp"
#include "order_stats.hpp"
#include "search_tools.hpp"
#include "search_history.hpp"
namespace petir_search {
pas::Array<std::int32_t, 0, 63> currentMove{};
pas::Array<std::int32_t, 0, 63> currentPiece{};
pas::Array<petir_search::anonymous_record_1, 0, 63> searchStack{};
std::int32_t researchCount{};
// SEE pruning of quiet moves: up to this many plies left, a quiet move whose SEE
// (in pawns) is below the margin is skipped. The margin grows with the square of
// the plies left: 0 up to 2 plies (any loss), -1 at 3, -2 at 4, -3 at 5, -4 at 6.
static constexpr std::int32_t seeQuietMaxPlies = 6;
static std::int32_t seeQuietMargin(std::int32_t plies) {return -(plies * plies / 8);}

// search.pas:72
// Alpha-beta search of one node; scores are from the side to move's point of view.
// - searchDepth is in eighths of a ply (see wholePlies). extensionCredit is how
//   much depth this branch may still gain through extensions.
// - ply counts plies from the root, which is ply 0.
// - nodeType is pvNode, cutNode or allNode. A child gets the negated type when
//   it is the first move searched, and cutNode otherwise.
// - afterNullMove: the parent made a null move. inNullMoveTree: an ancestor did.
//   reduced is not used here.
//
// Material counts used below: whiteNonPawnMaterial and blackNonPawnMaterial
// are in pawns (minor piece 3, rook 5, queen 9; 31 for a full set).
// position.lastCaptureType tells what the last move captured: 0 for a quiet
// move, whitePawn, whiteKnight (both minor pieces), whiteRook or whiteQueen,
// and 10 or 11 for a promotion.
std::int32_t mainSearch(std::int32_t searchDepth, std::int32_t sideToMove, std::int32_t alpha, std::int32_t beta, petir_header::tdata& position, bool afterNullMove, std::int32_t ply, std::int32_t extensionCredit, bool inNullMoveTree, std::int32_t reduced, std::int32_t nodeType) {
    const std::int32_t opponent = opponentOf(sideToMove);
    // Portability guards: stop safely and keep the original 64-ply arrays in bounds.
    if (petir::searchShouldStop()) return 0;
    if (ply >= 63) return petir_ueval::evaluate(position, sideToMove, alpha, beta);
    // Continuation-history tables of the opponent's move into this node and of
    // our move before it.
    petir_header::piece_to_history* const continuationTables[2] = {continuationTable(ply - 1), continuationTable(ply - 2)};
    // This node's entry on the search stack, and the parent's.
    auto& frame = searchStack[ply];
    const auto& parentFrame = searchStack[ply - 1];
    frame.checkExtended = false;
    frame.mateThreatExtended = false;

    if (position.fiftyMoveCounter >= 101) return petir_header::drawScore;

    // ---- Hash table ----
    // A usable entry ends the search here, except at PV nodes. The probe also
    // supplies the hash move and may advise against a null move.
    std::int32_t score{};
    std::int32_t hashMove = petir_header::noMove;
    bool avoidNullMove = false;
    const bool hashHit = petir_hashing::probeHash(position.hashKey, alpha, beta, searchDepth, sideToMove, score, hashMove, avoidNullMove, position.fiftyMoveCounter);
    if (hashHit && (nodeType != pvNode)) return score;
    const std::uint64_t savedHashKey = position.hashKey;
    const bool isNullWindow = (alpha == beta - 1);

    // ---- Check extension ----
    // 3/4 of a ply while plenty of pieces remain or the window is open,
    // otherwise 1/4 of a ply.
    const bool inCheck = isInCheck(sideToMove, position);
    if (inCheck) {
        frame.checkExtended = true;
        if (extensionCredit > 0) {
            const bool manyPieces = (position.whiteNonPawnMaterial + position.blackNonPawnMaterial > 24);
            if (manyPieces || !isNullWindow) {
                searchDepth += 6;
            } else {
                searchDepth += 2;
            }
        }
    }

    // ---- Null move ----
    // Let the opponent move twice in a row, at reduced depth. If we still reach
    // beta, a real move very likely would too, so the node fails high.
    // A null move that runs into an immediate mate reveals a threat: the node
    // is then extended by 3/4 of a ply and marked mateThreatExtended.
    bool mateThreatExtended = false;
    if (!inCheck) {
        const std::int32_t plies = wholePlies(searchDepth);
        const std::int32_t ownMaterial = petir_ueval::materialForSide(sideToMove, position.materialScore);
        const bool betaIsMateScore = (beta >= petir_header::infinity - 100);
        const bool farBehind = (ownMaterial < -500);
        // Without pieces, zugzwang makes a null move unreliable.
        const bool hasPieces = (petir_tools::nonPawnMaterial(position, sideToMove) >= 3);
        const bool tryNullMove = (plies >= 2) && isNullWindow && !betaIsMateScore && !farBehind
            && !afterNullMove && !avoidNullMove && hasPieces;
        if (tryNullMove) {
            // Reduce 4 plies, and more from 6 plies left while more than 24 of
            // non-pawn material remains (not an endgame, where zugzwang and slow
            // threats are common): 3 + plies / 3 (6: 5, 9: 6, 12: 7, 15: 8).
            // Only 3 plies with exactly 4 plies left in a PV-type node where our
            // material balance is less than 500 above alpha.
            const bool notEndgame = (position.whiteNonPawnMaterial + position.blackNonPawnMaterial > 24);
            std::int32_t nullReduction = ((plies >= 6) && notEndgame) ? 3 + plies / 3 : 4;
            if ((plies == 4) && (ownMaterial < alpha + 500) && (nodeType == pvNode)) {
                nullReduction = 3;
            }
            std::int32_t nullMoveDepth = searchDepth - petir_header::plyUnits * nullReduction;
            // Passing the move cancels any en passant right.
            const std::int32_t savedEnPassantFile = position.enPassantFile;
            if (position.enPassantFile != petir_header::noEnPassant) {
                position.hashKey ^= petir_header::zobristEnPassantKeys[position.enPassantFile];
                position.enPassantFile = petir_header::noEnPassant;
                position.hashKey ^= petir_header::zobristEnPassantKeys[petir_header::noEnPassant];
            }
            frame.evaluated = false;
            currentMove[ply] = petir_header::noMove;
            if (nullMoveDepth < petir_header::plyUnits) {
                // Allow checks in the quiescence search unless both sides are
                // down to little material.
                const bool littleMaterial = (position.whiteNonPawnMaterial <= 9) && (position.blackNonPawnMaterial <= 9);
                const std::int32_t checkPlies = littleMaterial ? 0 : 1;
                score = -quiescenceSearch(opponent, -beta, -(beta - 1), position, checkPlies, ply + 1, extensionCredit, noValue, -nodeType);
            } else {
                score = -mainSearch(nullMoveDepth, opponent, -beta, -(beta - 1), position, true, ply + 1, extensionCredit, true, 0, -nodeType);
            }
            position.hashKey = savedHashKey;
            position.enPassantFile = savedEnPassantFile;
            if (petir::searchShouldStop()) return 0;
            if (score >= beta) return score;
            // Mated at ply + 2: the opponent mates at once after our null move.
            const bool mateThreat = (score == -petir_header::kingValue + ply + 2);
            if (mateThreat && (extensionCredit > 0)) {
                frame.mateThreatExtended = true;
                searchDepth += 6;
                extensionCredit -= 6;
                mateThreatExtended = true;
            }
        }
    }

    // The parent evaluated this position for futility (parentFrame.staticEval,
    // from our side). Don't reduce our moves when that evaluation was 90 or
    // more above our material balance, or already at or above beta.
    bool doNotReduce = false;
    if (parentFrame.evaluated) {
        const std::int32_t parentEval = parentFrame.staticEval;
        const std::int32_t ownMaterial = petir_ueval::materialForSide(sideToMove, position.materialScore);
        doNotReduce = (parentEval - ownMaterial >= 90) || (parentEval >= beta);
    }

    petir_header::tdata savedPosition = position;
    const std::uint64_t pinnedMask = inCheck ? 0 : petir_tools::pinnedPieces(position, sideToMove);
    // After any check or mate-threat extension.
    const std::int32_t remainingPlies = wholePlies(searchDepth);
    // Repetitions are recorded per side to move (table index side - 1); after
    // our move the opponent is to move.
    const std::int32_t repetitionSide = opponent - 1;

    // ---- Move loop state ----
    // Move-picker state. Generators write both fields in entries 1..count
    // before move ordering. Avoid clearing two 2 KiB buffers on every node;
    // capacity checks remain.
    std::int32_t phase = 0;
    petir_header::tmovelist quietList;
    petir_header::tmovelist captureList;
    std::int32_t captureIndex = 0;
    std::int32_t quietIndex = 0;
    std::int32_t captureCount = 0;
    std::int32_t quietMoveCount = 0;
    std::int32_t bestCaptureGain = 0;
    // The evasion picker (used in check) has its own counters and reuses the
    // quiet list.
    petir_header::tmovelist& evasionList = quietList;
    std::int32_t evasionIndex = 0;
    std::int32_t evasionCount = 0;
    std::int32_t checkerCount = 0;

    // currentAlpha rises as moves improve on it. currentBeta is beta for the
    // first move and currentAlpha + 1 (a null window) after it.
    std::int32_t currentAlpha = alpha;
    std::int32_t currentBeta = beta;
    std::int32_t bestScore = -petir_header::infinity - 100;
    std::int32_t bestMove = petir_header::noMove;
    bool bestIsRepetitionDraw = false;
    // Includes moves skipped by SEE pruning.
    std::int32_t legalMoveCount = 0;
    // The child depth and extension credit each move starts from. Single-reply
    // and double-check extensions raise them for all later moves too.
    std::int32_t nextDepth = searchDepth - petir_header::plyUnits;
    std::int32_t nodeExtensionCredit = extensionCredit;
    // Quiet moves searched at this node, penalised when another quiet move is best.
    std::int32_t searchedQuiets[64];
    std::int32_t searchedQuietCount = 0;

    // The move being searched, set at the start of each iteration of the move
    // loop. newDepth is the depth its child is searched to, and reduction how
    // far below the normal depth that is (a fail-high is re-searched without it).
    std::int32_t move = petir_header::noMove;
    std::int32_t toSquare = 0;
    std::int32_t movedPiece = 0;
    std::int32_t moveHistorySquare = 0;
    std::int32_t newDepth = 0;
    std::int32_t reduction = 0;
    std::int32_t childStaticEval = noValue;

    // ---- Steps of the move loop ----
    // SEE pruning of quiet moves (as in Stockfish): at non-PV nodes with few
    // plies left, skip a late quiet move that loses material by SEE, unless it
    // gives check. Hash and killer moves are never pruned, and at least one move
    // must already have a non-mated score. Called before the move is made.
    auto canPruneBySee = [&]() -> bool {
        if (phase != petir_header::otherMovesPhase) return false;
        if (remainingPlies > seeQuietMaxPlies) return false;
        if ((nodeType == pvNode) || (beta - alpha != 1)) return false;
        if ((legalMoveCount == 0) || mateThreatExtended) return false;
        const bool hasNonMatedScore = (bestScore > -petir_header::kingValue + 100);
        if (!hasNonMatedScore) return false;
        // Castling has no SEE.
        if (toSquare > 63) return false;
        const std::int32_t see = petir_usee::staticExchange(sideToMove, position, moveFromSquare(move), toSquare);
        if (see >= seeQuietMargin(remainingPlies)) return false;
        return !petir_tools::moveGivesCheck(position, sideToMove, move);
    };

    // Shrinks newDepth by amount (in eighths of a ply) and records it as the reduction.
    auto reduceBy = [&](std::int32_t amount) {
        newDepth -= amount;
        reduction = amount;
    };

    // A quiet move that has been tried at least minHistoryTries times and has
    // caused a cutoff at most once in 2^shift tries.
    auto rarelyCutsOff = [&](std::int32_t shift) -> bool {
        const std::int32_t tried = historyTried[movedPiece][moveHistorySquare];
        const std::int32_t succeeded = historySucceeded[movedPiece][moveHistorySquare];
        return (tried >= minHistoryTries) && (succeeded <= pas::shiftRight(tried, shift));
    };

    // Extensions for promising moves. Returns true when the move was extended;
    // it then skips razoring and reductions.
    auto extendMove = [&]() -> bool {
        const std::int32_t capturedType = position.lastCaptureType;
        const bool isPvNode = (nodeType == pvNode);

        // Pawn endings at PV nodes: half a ply for capturing a pawn, and two
        // plies for the capture that took the last piece off the board, unless
        // the material balance is beyond 385.
        const bool isPawnEnding = (position.whiteNonPawnMaterial == 0) && (position.blackNonPawnMaterial == 0);
        if (isPawnEnding && isPvNode) {
            const bool capturedPawn = (capturedType == petir_header::whitePawn);
            if (capturedPawn && (extensionCredit > 0)) {
                newDepth += 4;
                extensionCredit -= 4;
            }
            const bool capturedPiece = (capturedType >= petir_header::whiteKnight) && (capturedType <= petir_header::whiteQueen);
            const bool materialIsClose = (position.materialScore >= -385) && (position.materialScore <= 385);
            if (capturedPiece && materialIsClose && (extensionCredit > 0)) {
                extensionCredit -= 16;
                newDepth += 16;
                return true;
            }
        }

        // One ply for a pawn reaching the seventh rank at a PV node, unless SEE
        // says it is lost there.
        const bool landedPawn = (toSquare <= 63) && (pas::abs(position.board[toSquare]) == petir_header::whitePawn);
        const bool onSeventhRank = (sideToMove == petir_header::whiteSide) ? (toSquare >= petir_header::a7) : (toSquare <= petir_header::h2);
        if (landedPawn && isPvNode && (extensionCredit > 0) && onSeventhRank && !inCheck) {
            const bool safe = petir_usee::staticExchange(sideToMove, savedPosition, moveFromSquare(move), toSquare) >= 0;
            if (safe) {
                newDepth += 8;
                extensionCredit -= 8;
                return true;
            }
        }

        // One ply for recapturing a piece of the kind the opponent just took
        // (pawn captures and promotions don't count), when the window is open.
        const bool capturedPieceNotPromotion = (capturedType > petir_header::whitePawn) && (capturedType < 9);
        const bool sameKindAsOpponentTook = (capturedType == savedPosition.lastCaptureType);
        if (!isNullWindow && capturedPieceNotPromotion && sameKindAsOpponentTook && !inCheck && (extensionCredit > 0) && !mateThreatExtended) {
            newDepth += 8;
            extensionCredit -= 8;
            return true;
        }
        return false;
    };

    // Razoring for quiet moves at null-window non-PV nodes with one or two
    // plies left: when even the material balance plus a margin can't reach
    // alpha, take that estimate as the move's score instead of searching it.
    // Checking moves, positions with a pawn about to promote, and positions
    // with few pieces or without pawns on one side are left alone. Returns
    // true and sets razorScore when the move is razored.
    auto razorMove = [&](std::int32_t& razorScore) -> bool {
        if ((remainingPlies != 1) && (remainingPlies != 2)) return false;
        if (inCheck || (nodeType == pvNode) || (beta - alpha != 1)) return false;
        if ((position.lastCaptureType != 0) || (move == hashMove)) return false;
        if (parentFrame.mateThreatExtended || parentFrame.checkExtended) return false;

        std::int32_t estimate = petir_ueval::materialForSide(sideToMove, position.materialScore);
        // With fewer pieces, positional factors weigh more, so be careful.
        if (position.whiteNonPawnMaterial + position.blackNonPawnMaterial <= 24) {
            estimate += 300;
        }
        const bool onePlyLeft = (remainingPlies == 1);
        const std::int32_t margin = onePlyLeft ? 250 : 300;
        // Each side needs more non-pawn material than this.
        const std::int32_t minimumPieces = onePlyLeft ? 3 : 5;

        if (estimate + margin >= currentAlpha) return false;
        if (petir_tools::moveGivesCheck(savedPosition, sideToMove, move)) return false;
        if ((position.whiteNonPawnMaterial <= minimumPieces) || (position.blackNonPawnMaterial <= minimumPieces)) return false;
        if (petir_tools::hasPawnOnSeventh(sideToMove, position)) return false;
        if (!onePlyLeft && petir_tools::hasFreeSixthRankPawn(sideToMove, position)) return false;
        if ((position.whitePawns == 0) || (position.blackPawns == 0)) return false;
        razorScore = estimate;
        return true;
    };

    // Reduction step 1 applies to a late quiet move at a null-window node that
    // alpha hasn't been raised in yet, when the move creates no danger: no pawn
    // on the seventh, no rook to the seventh or passed-pawn move (isGoodMove), no
    // attack on the enemy king zone and no discovered check. Both sides must
    // keep enough pieces. Sets threatenedValue to the value of the best enemy
    // piece the moved piece attacks (0 if none).
    auto isCalmLateQuietMove = [&](std::int32_t& threatenedValue) -> bool {
        if (inCheck || (remainingPlies < 2) || (ply < 2)) return false;
        if (position.lastCaptureType != 0) return false;
        if (phase <= petir_header::killerMove3Phase) return false;
        if ((alpha != currentAlpha) || (beta - alpha != 1)) return false;
        if (petir_tools::hasPawnOnSeventh(sideToMove, position)) return false;
        if (mateThreatExtended) return false;
        if (petir_tools::isGoodMove(position, toSquare, sideToMove)) return false;
        const bool bothHaveTwoPieces = (position.whiteNonPawnMaterial >= 6) && (position.blackNonPawnMaterial >= 6);
        const bool bothHaveAPiece = (position.whiteNonPawnMaterial >= 3) && (position.blackNonPawnMaterial >= 3);
        if (!bothHaveTwoPieces && !(bothHaveAPiece && (remainingPlies == 2))) return false;
        if (petir_tools::moveThreatensKing(sideToMove, position, move, threatenedValue)) return false;
        return !givesDiscoveredCheck(sideToMove, savedPosition, move);
    };

    // Reduction step 2 (4.39 search.pas:758): near the root (up to ply 6), at a
    // non-PV node, a very late quiet move (after 12 others) whose history is
    // very poor (at most 1 cutoff in 64 tries).
    auto isHopelessMoveNearRoot = [&]() -> bool {
        if (inCheck || (ply > 6) || (nodeType == pvNode)) return false;
        if (position.lastCaptureType != 0) return false;
        if ((ply < 2) || (legalMoveCount <= 12)) return false;
        if (mateThreatExtended || (reduction != 0)) return false;
        if (parentFrame.mateThreatExtended || searchStack[ply - 2].mateThreatExtended) return false;
        if (petir_tools::hasPawnOnSeventh(sideToMove, position) || doNotReduce) return false;
        if (petir_tools::isGoodMove(position, toSquare, sideToMove)) return false;
        if (!rarelyCutsOff(6)) return false;
        return !petir_tools::moveGivesCheck(savedPosition, sideToMove, move);
    };

    // Reduction step 3, late-move reduction: with at least 4 plies left at a
    // non-PV node, a quiet move past the killers that doesn't check, after the
    // fourth move at cut nodes and after the seventh at all nodes.
    auto isLateMove = [&]() -> bool {
        if (inCheck || (remainingPlies < 4) || (ply < 2) || (nodeType == pvNode)) return false;
        const bool lateInCutNode = (nodeType == cutNode) && (legalMoveCount > 3);
        const bool lateInAllNode = (nodeType == allNode) && (legalMoveCount > 6);
        if (!lateInCutNode && !lateInAllNode) return false;
        if (mateThreatExtended || (phase <= petir_header::killerMove3Phase)) return false;
        if ((reduction != 0) || (position.lastCaptureType != 0)) return false;
        if (petir_tools::moveGivesCheck(savedPosition, sideToMove, move)) return false;
        if (petir_tools::hasPawnOnSeventh(sideToMove, position)) return false;
        if (petir_tools::isGoodMove(position, toSquare, sideToMove)) return false;
        return !doNotReduce;
    };

    // Reductions for quiet moves that look unpromising, in three steps.
    auto reduceMove = [&]() {
        std::int32_t threatenedValue = 0;
        if (isCalmLateQuietMove(threatenedValue)) {
            // 1a. History: one ply if the move rarely cuts off (at most 1 in 8
            //     tries), two plies if very rarely (1 in 16). It must attack
            //     nothing, unless the line looks quiet: our previous move was a
            //     late quiet move and the opponent's reply was among its first
            //     three moves.
            const bool lineLooksQuiet = (legalMoveCount > 5) && (searchStack[ply - 2].phase > petir_header::killerMove4Phase)
                && (parentFrame.legalMoveCount <= 3) && !parentFrame.mateThreatExtended;
            const bool attacksNothing = (threatenedValue == 0);
            if ((attacksNothing || lineLooksQuiet) && (legalMoveCount > 3) && rarelyCutsOff(3)) {
                const std::int32_t tried = historyTried[movedPiece][moveHistorySquare];
                const bool veryRarely = (historySucceeded[movedPiece][moveHistorySquare] <= pas::shiftRight(tried, 4));
                reduceBy(veryRarely ? 16 : 8);
                return;
            }
            // 1b. Futility, up to 8 plies left: compare the evaluation after the
            //     move, plus the threatened piece's value and a margin, with alpha.
            if (remainingPlies <= 8) {
                // From our side; the flag says the opponent is to move. The
                // child's quiescence search reuses it as its static evaluation.
                const std::int32_t evalAfterMove = petir_ueval::evaluate(position, sideToMove, -petir_header::infinity, petir_header::infinity, true);
                childStaticEval = -evalAfterMove;
                frame.evaluated = true;
                frame.staticEval = childStaticEval;
                const std::int32_t margin = futilityMargins[remainingPlies];
                const std::int32_t optimisticScore = evalAfterMove + threatenedValue + margin;
                if ((optimisticScore + 30 < currentAlpha) && !petir_tools::hasFreeSixthRankPawn(sideToMove, position)) {
                    reduceBy(24);
                } else if (optimisticScore < currentAlpha) {
                    reduceBy(16);
                } else if (optimisticScore - 5 < currentAlpha) {
                    reduceBy(8);
                } else if ((threatenedValue > 0) && (evalAfterMove + margin + 40 + threatenedValue / 2 < currentAlpha)) {
                    // Drop straight into the quiescence search, without
                    // counting it as a reduction (no re-search on a fail-high).
                    newDepth = 0;
                } else if ((bestCaptureGain > 1) && attacksNothing && (evalAfterMove < currentAlpha)) {
                    // A good capture is available (bestCaptureGain comes from
                    // the move picker) and this quiet move doesn't reach alpha.
                    reduceBy((bestCaptureGain >= 5) ? 16 : 8);
                }
            }
        }
        if (isHopelessMoveNearRoot()) {
            reduceBy(8);
            return;
        }
        if (isLateMove()) {
            reduceBy(8);
        }
    };

    // Searches the position after the move to childDepth, with the window
    // (lower, upper) seen from our side, and returns the score from our side.
    // Below one ply it calls the quiescence search, with checks allowed.
    auto searchChild = [&](std::int32_t childDepth, std::int32_t lower, std::int32_t upper, std::int32_t staticEvalForChild, std::int32_t childReduction, std::int32_t childNodeType) {
        if (childDepth < petir_header::plyUnits) {
            return -quiescenceSearch(opponent, -upper, -lower, position, 1, ply + 1, extensionCredit, staticEvalForChild, childNodeType);
        }
        return -mainSearch(childDepth, opponent, -upper, -lower, position, false, ply + 1, extensionCredit, inNullMoveTree, childReduction, childNodeType);
    };

    // Takes the move back.
    auto unmakeMove = [&]() {
        petir_repetition::removeRepetition(position.hashKey, repetitionSide);
        position = savedPosition;
    };

    // ---- Move loop ----
    PETIR_ORDER_STAT(petir_order_stats::SearchNode orderStats(remainingPlies, sideToMove, hashMove, nodeType, inCheck);)
    for (;;) {
        if (petir::searchShouldStop()) return 0;
        frame.evaluated = false;
        bool isRepetitionDraw = false;
        childStaticEval = noValue;
        extensionCredit = nodeExtensionCredit;
        newDepth = nextDepth;
        reduction = 0;

        // Pick the next move and skip pseudo-legal moves that are illegal.
        if (!inCheck) {
            petir_next::nextMove(sideToMove, ply, position, move, phase, hashMove, captureIndex, quietIndex, captureCount, quietMoveCount, quietList, captureList, bestCaptureGain, continuationTables);
            if (move == petir_header::noMove) break;
            toSquare = moveToSquare(move);
            if (leavesKingAttacked(sideToMove, position, move, pinnedMask)) continue;
            if (canPruneBySee()) {
                legalMoveCount++;
                continue;
            }
        } else {
            // In check the picker generates evasions only, and reports
            // checkmate when there are none.
            const bool checkmated = petir_next::nextEvasionMove(sideToMove, position, move, phase, hashMove, evasionIndex, evasionCount, checkerCount, evasionList);
            if (checkmated) return -petir_header::kingValue + ply;
            if (move == petir_header::noMove) break;
            toSquare = moveToSquare(move);
            // Extensions decided at the first evasion, for every evasion.
            if ((legalMoveCount == 0) && (extensionCredit > 0)) {
                // A single legal reply: extend, rounded down to whole plies.
                // This also gives the branch a ply of extra credit.
                if (evasionCount == 1) {
                    newDepth += petir_header::singleReplyExtension;
                    nextDepth += petir_header::singleReplyExtension;
                    extensionCredit += 8;
                    nodeExtensionCredit += 8;
                    newDepth -= newDepth & 7;
                }
                if (checkerCount == 2) {
                    newDepth += petir_header::doubleCheckExtension;
                    nextDepth += petir_header::doubleCheckExtension;
                    extensionCredit -= petir_header::doubleCheckExtension;
                    nodeExtensionCredit -= petir_header::doubleCheckExtension;
                }
                // Pays for the check extension made on entry.
                extensionCredit -= petir_header::checkExtension;
                nodeExtensionCredit -= petir_header::checkExtension;
            }
        }

        makeSideMove(sideToMove, move, position);
        // Children at full depth probe the hash table first; overlap that
        // memory access with the extension and reduction work below.
        if (nextDepth >= petir_header::plyUnits) {
            petir_hashing::prefetchHash(position.hashKey, opponent);
        }
        if ((toSquare == petir_header::enPassantTarget) && isInCheck(sideToMove, position)) {
            position = savedPosition;
            continue;
        }
        movedPiece = savedPosition.board[moveFromSquare(move)];
        moveHistorySquare = historySquare(sideToMove, toSquare);
        const bool isQuiet = (position.lastCaptureType == 0);
        if (isQuiet) {
            auto& tried = historyTried[movedPiece][moveHistorySquare];
            tried = pas::increment(tried, pas::mul(remainingPlies, remainingPlies));
            if (searchedQuietCount < 64) {
                searchedQuiets[searchedQuietCount++] = move;
            }
        }
        currentMove[ply] = move;
        currentPiece[ply] = movedPiece;
        legalMoveCount++;
        PETIR_ORDER_STAT(orderStats.move(move, hashMove, phase, quietIndex, quietMoveCount, quietList);)

        // A position that is already on the repetition table counts as a draw
        // at once, without waiting for the third occurrence.
        if (petir_repetition::addRepetition(position.hashKey, repetitionSide) == 1) {
            score = petir_header::drawScore;
            isRepetitionDraw = true;
        } else {
            frame.phase = phase;
            frame.legalMoveCount = legalMoveCount;
            bool razored = false;
            if (!extendMove()) {
                razored = razorMove(score);
                if (!razored) reduceMove();
            }
            if (!razored) {
                PETIR_ORDER_STAT(orderStats.searching(reduction);)
                // Below one ply, extend to one ply a move that forks two
                // pieces or wins a pawn (this uses up the extension credit).
                if ((newDepth < 8) && forksOrWinsPawn(sideToMove, toSquare, position) && (extensionCredit > 0)) {
                    newDepth = 8;
                    extensionCredit = 0;
                }
                // Principal variation search: the first move gets the full
                // window and later moves a null window.
                const std::int32_t childNodeType = (legalMoveCount == 1) ? -nodeType : cutNode;
                score = searchChild(newDepth, currentAlpha, currentBeta, childStaticEval, reduction, childNodeType);
                if (petir::searchShouldStop()) {
                    unmakeMove();
                    return 0;
                }
                const bool insideWindow = (score > currentAlpha) && (score < beta);
                if (insideWindow && (legalMoveCount > 1)) {
                    // A later move beat alpha in the null window: search it
                    // again as a PV node, without its reduction.
                    PETIR_ORDER_STAT(orderStats.pvsResearch();)
                    newDepth += reduction;
                    score = searchChild(newDepth, currentAlpha, beta, noValue, reduction, pvNode);
                    PETIR_ORDER_STAT(orderStats.pvsResult(score > currentAlpha);)
                } else if ((score >= beta) && (reduction != 0)) {
                    // A reduced move failed high: verify it without the
                    // reduction, with the window (currentAlpha, score).
                    PETIR_ORDER_STAT(orderStats.lmrResearch();)
                    newDepth += reduction;
                    reduction = 0;
                    score = searchChild(newDepth, currentAlpha, score, noValue, reduction, pvNode);
                    PETIR_ORDER_STAT(orderStats.lmrResult(score >= beta);)
                }
            }
        }
        if (petir::searchShouldStop()) {
            unmakeMove();
            return 0;
        }

        if (score > bestScore) {
            bestScore = score;
            bestIsRepetitionDraw = isRepetitionDraw;
            if (score > currentAlpha) {
                bestMove = move;
                currentAlpha = score;
                PETIR_ORDER_STAT(orderStats.raised(legalMoveCount);)
                if (bestScore >= beta) {
                    // Beta cutoff. A quiet move earns history credit, becomes the
                    // killer, and the quiet moves tried before it are penalised.
                    PETIR_ORDER_STAT(orderStats.cutoff(legalMoveCount);)
                    if (position.lastCaptureType == 0) {
                        auto& succeeded = historySucceeded[movedPiece][moveHistorySquare];
                        succeeded = pas::increment(succeeded, pas::mul(remainingPlies, remainingPlies));
                    }
                    if ((position.lastCaptureType == 0) && !inCheck) {
                        addHistory(remainingPlies, ply, move, sideToMove, movedPiece);
                        updateContinuation(continuationTables, movedPiece, toSquare, historyBonus(remainingPlies));
                        penaliseQuiets(remainingPlies, sideToMove, savedPosition, continuationTables, searchedQuiets, searchedQuietCount, move);
                    }
                    unmakeMove();
                    break;
                }
            }
        }
        currentBeta = currentAlpha + 1;
        unmakeMove();
    }
    if (petir::searchShouldStop()) return 0;
    PETIR_ORDER_STAT(orderStats.finish(bestScore, alpha, beta, legalMoveCount);)

    // ---- Result ----
    if (legalMoveCount == 0) {
        // No legal move: stalemate.
        bestMove = petir_header::noMove;
        bestScore = petir_header::drawScore;
    } else if (bestMove == petir_header::noMove) {
        // Every move failed low; keep the hash move for the hash entry.
        bestMove = hashMove;
    }

    // Store the result, unless it rests on a repetition draw, which depends on
    // the path to this position. An exact score from a quiet best move also
    // updates the history tables.
    if (!bestIsRepetitionDraw) {
        if (bestScore <= alpha) {
            petir_hashing::storeHash(savedHashKey, sideToMove, searchDepth, bestMove, bestScore, petir_header::upperBound, position.fiftyMoveCounter);
        } else if (bestScore >= beta) {
            petir_hashing::storeHash(savedHashKey, sideToMove, searchDepth, bestMove, bestScore, petir_header::lowerBound, position.fiftyMoveCounter);
        } else {
            petir_hashing::storeHash(savedHashKey, sideToMove, searchDepth, bestMove, bestScore, petir_header::exactBound, position.fiftyMoveCounter);
            if (bestMove != petir_header::noMove) {
                const std::int32_t bestToSquare = moveToSquare(bestMove);
                // Castling counts as quiet; en passant and captures don't.
                const bool bestIsQuiet = (bestToSquare != petir_header::enPassantTarget)
                    && ((bestToSquare > 63) || (position.board[bestToSquare] == 0));
                if (bestIsQuiet) {
                    const std::int32_t bestPiece = position.board[moveFromSquare(bestMove)];
                    updateHistory(sideToMove, bestPiece, bestToSquare, historyBonus(remainingPlies));
                    updateContinuation(continuationTables, bestPiece, bestToSquare, historyBonus(remainingPlies));
                    penaliseQuiets(remainingPlies, sideToMove, position, continuationTables, searchedQuiets, searchedQuietCount, bestMove);
                }
            }
        }
    }
    return bestScore;
}

}
