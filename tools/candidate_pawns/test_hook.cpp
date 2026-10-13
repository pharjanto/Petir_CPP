// Compiled only into evalshow. The released engine has no override branch.
#define PETIR_PAWN_POTENTIAL_TEST 1
#include "ueval.cpp"
#undef PETIR_EVAL_INLINE
#undef PETIR_EVAL_NOINLINE

namespace petir_ueval {
int candidatePasserTermForTest(const petir_header::tdata& position, bool cached) {
    return candidatePawnPotential(position, true, cached) - candidatePawnPotential(position, false, cached);
}
bool candidateCacheMatchesForTest(const petir_header::tdata& position) {
    const auto& cached = probePawnPotential(position.whitePawns, position.blackPawns);
    const auto white = buildPawnPotential(position.whitePawns, position.blackPawns, true);
    const auto black = buildPawnPotential(position.blackPawns, position.whitePawns, false);
    return std::memcmp(&white, &cached.white, sizeof(white)) == 0
        && std::memcmp(&black, &cached.black, sizeof(black)) == 0;
}
int evaluateInjectedPotentialForTest(const petir_header::tdata& position, int side, int term) {
    const int saved = pawnPotentialOverrideForTest;
    pawnPotentialOverrideForTest = term;
    const int value = evaluate(position, side, -20500, 20500);
    pawnPotentialOverrideForTest = saved;
    return side == 1 ? value : -value;
}
}
