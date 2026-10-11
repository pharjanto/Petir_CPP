// Faithful translation of undoredo.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_undoredo {
// undoredo.pas:24
void resetUndo(petir_undoredo::tundo& undo) {
    undo.count = 0;
    undo.maxCount = 0;
}

// undoredo.pas:29
void addUndo(petir_undoredo::tundo& undo, petir_header::tdata position, std::int32_t whiteClock, std::int32_t blackClock) {
    if ((undo.count == undo.maxCount)) {
        (undo.count = pas::increment(undo.count, 1));
        (undo.maxCount = pas::increment(undo.maxCount, 1));
        undo.entries[undo.count].position = position;
        undo.entries[undo.count].whiteClock = whiteClock;
        undo.entries[undo.count].blackClock = blackClock;
    } else {
        (undo.count = pas::increment(undo.count, 1));
        undo.maxCount = undo.count;
        undo.entries[undo.count].position = position;
        undo.entries[undo.count].whiteClock = whiteClock;
        undo.entries[undo.count].blackClock = blackClock;
    }
}

// undoredo.pas:75
bool takeBack(petir_undoredo::tundo& undo, petir_header::tdata& position, std::int32_t& whiteClock, std::int32_t& blackClock, std::uint8_t& sideToMove) {
    bool result{};
    result = false;
    if ((undo.count > 0)) {
        result = true;
    }
    if ((undo.count == 1)) {
        (undo.count = pas::decrement(undo.count, 0));
        sideToMove = pas::sub(3, sideToMove);
    } else {
        (undo.count = pas::decrement(undo.count, 1));
    }
    sideToMove = pas::sub(3, sideToMove);
    position = undo.entries[undo.count].position;
    whiteClock = undo.entries[undo.count].whiteClock;
    blackClock = undo.entries[undo.count].blackClock;
    return result;
}

// undoredo.pas:52
bool moveForward(petir_undoredo::tundo& undo, petir_header::tdata& position, std::int32_t& whiteClock, std::int32_t& blackClock, std::uint8_t& sideToMove) {
    bool result{};
    result = false;
    if ((undo.count < undo.maxCount)) {
        result = true;
        if ((undo.count == pas::sub(undo.maxCount, 1))) {
            (undo.count = pas::increment(undo.count, 1));
        } else {
            (undo.count = pas::increment(undo.count, 1));
        }
        position = undo.entries[undo.count].position;
        whiteClock = undo.entries[undo.count].whiteClock;
        blackClock = undo.entries[undo.count].blackClock;
        sideToMove = pas::sub(3, sideToMove);
    }
    return result;
}

}
