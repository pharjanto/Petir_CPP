// Faithful translation of repetition.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_repetition {
petir_repetition::tarrreprecord repetitionTable{};
// repetition.pas:22
void clearRepetitionTable() {
    std::int32_t slot{};
    std::int32_t side{};
    std::int32_t lastSide{};
    std::int32_t lastSlot{};
    lastSide = 1;
    for (side = 0; side <= lastSide; ++side) {
        lastSlot = pas::sub(petir_repetition::repetitionTableSize, 1);
        for (slot = 0; slot <= lastSlot; ++slot) {
            petir_repetition::repetitionTable[side][slot].used = false;
            petir_repetition::repetitionTable[side][slot].count = 0;
        }
    }
}

// repetition.pas:53
std::int32_t addRepetition(std::uint64_t key, std::int32_t side) {
    std::int32_t result{};
    std::uint64_t slot{};
    petir_repetition::treprecord* record{};
    slot = (key & pas::sub(petir_repetition::repetitionTableSize, 1));
    result = pas::negate(1);
    while ((petir_repetition::repetitionTable[side][slot].used && (petir_repetition::repetitionTable[side][slot].key != key))) {
        if ((slot == pas::sub(petir_repetition::repetitionTableSize, 1))) {
            slot = pas::negate(1);
        }
        (slot = pas::increment(slot, 1));
    }
    record = pas::address(petir_repetition::repetitionTable[side][slot]);
    if (((*record).used == false)) {
        (*record).used = true;
        (*record).key = key;
        (*record).count = 0;
        result = 0;
    } else {
        ((*record).count = pas::increment((*record).count, 1));
        result = (*record).count;
    }
    return result;
}

// repetition.pas:81
void removeRepetition(std::uint64_t key, std::int32_t side) {
    std::uint64_t slot{};
    slot = (key & pas::sub(petir_repetition::repetitionTableSize, 1));
    while ((petir_repetition::repetitionTable[side][slot].key != key)) {
        if ((slot == pas::sub(petir_repetition::repetitionTableSize, 1))) {
            slot = pas::negate(1);
        }
        (slot = pas::increment(slot, 1));
    }
    if ((petir_repetition::repetitionTable[side][slot].count == 0)) {
        petir_repetition::repetitionTable[side][slot].used = false;
    } else {
        (petir_repetition::repetitionTable[side][slot].count = pas::decrement(petir_repetition::repetitionTable[side][slot].count, 1));
    }
}

// repetition.pas:34
bool isRepetition(std::uint64_t key, std::int32_t side) {
    bool result{};
    std::uint64_t slot{};
    slot = (key & pas::sub(petir_repetition::repetitionTableSize, 1));
    while ((petir_repetition::repetitionTable[side][slot].key != key)) {
        if ((slot == pas::sub(petir_repetition::repetitionTableSize, 1))) {
            slot = pas::negate(1);
        }
        (slot = pas::increment(slot, 1));
    }
    if ((petir_repetition::repetitionTable[side][slot].count == 2)) {
        result = true;
        return result;
    }
    result = false;
    return result;
}

}
