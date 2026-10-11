#pragma once

namespace petir_header { struct tdata; }

namespace petir_kpk {
// Generated in memory; no external tablebase files or engine masks are needed.
void initialize();
// Returns false for material other than exactly two kings and one pawn, or an
// illegal KPK position. sideToMove is the actual side to move; score uses its perspective
// and draws return zero (the caller applies its contempt policy).
bool probe(const petir_header::tdata& position, int sideToMove, int& score);
}
