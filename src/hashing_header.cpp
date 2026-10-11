// Faithful translation of hashing_header.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_hashing_header {
petir_hashing_header::thashdata hashTable{};
std::uint8_t generation{};
std::int32_t discardedRandom{};
// hashing_header.pas:89
void initHashKeys() {
    std::int32_t piece{};
    std::int32_t square{};
    pas::randomSeed = 99999;
    for (piece = -6; piece <= 6; ++piece) {
        for (square = 0; square <= 127; ++square) {
            const std::uint64_t key = pas::delphiHashRandom();
            if (square <= 63) petir_header::zobristPieceKeys[piece][square] = key;
            else petir_hashing_header::discardedRandom = static_cast<std::int32_t>(key);
        }
    }
    for (piece = 0; piece <= 255; ++piece) petir_header::zobristCastleKeys[piece] = pas::delphiHashRandom();
    for (piece = 0; piece <= petir_header::noEnPassant; ++piece) petir_header::zobristEnPassantKeys[piece] = pas::delphiHashRandom();
    petir_header::noEnPassantHashKey = petir_header::zobristEnPassantKeys[10];
}

// hashing_header.pas:74
std::uint64_t computeHashKey(petir_header::tdata& position, std::int32_t sideToMove) {
    std::uint64_t result{};
    std::uint64_t key{};
    std::int32_t square{};
    std::int32_t lastSquare{};
    key = 0;
    lastSquare = 63;
    for (square = 0; square <= lastSquare; ++square) {
        if ((position.board[square] != 0)) {
            key = (key ^ petir_header::zobristPieceKeys[position.board[square]][square]);
        }
    }
    key = (key ^ petir_header::zobristCastleKeys[position.castleRights]);
    key = (key ^ petir_header::zobristEnPassantKeys[position.enPassantFile]);
    result = key;
    return result;
}

// hashing_header.pas:44
void clearHashTable() {
    std::int32_t entryIndex{};
    std::int32_t lastEntry{};
    lastEntry = pas::sub(petir_header::hashTableSize, 1);
    for (entryIndex = 0; entryIndex <= lastEntry; ++entryIndex) {
        petir_hashing_header::hashTable[entryIndex].flags = (petir_hashing_header::hashTable[entryIndex].flags & (~petir_hashing_header::usedFlag));
        petir_hashing_header::hashTable[entryIndex].flags = (petir_hashing_header::hashTable[entryIndex].flags & (~petir_hashing_header::oldFlag));
    }
}

// hashing_header.pas:64
void advanceHashGeneration() {
    // The tag changes replacement priority only; key and rule-50 checks remain.
    generation = (generation+1)&generationMask;
}

// hashing_header.pas:37
void allocateHashTable() {
    petir_hashing_header::hashTable.resize(petir_header::hashTableSize);
}

// hashing_header.pas:59
void clearReverseTable() {
}

// hashing_header.pas:55
void reverseTable() {
}

// hashing_header.pas:113
void coba() {
}

}
