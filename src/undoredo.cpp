// Faithful translation of undoredo.pas; Delphi source is in original/.
#include "petir.hpp"
namespace petir_undoredo {
// undoredo.pas:24
void resetundo(petir_undoredo::tundo& undo) {
    undo.count = 0;
    undo.max = 0;
}

// undoredo.pas:29
void addundo(petir_undoredo::tundo& undo, petir_header::tdata data, std::int32_t whiteclock, std::int32_t blackclock) {
    if ((undo.count == undo.max)) {
        (undo.count = pas::increment(undo.count, 1));
        (undo.max = pas::increment(undo.max, 1));
        undo.moves[undo.count].data = data;
        undo.moves[undo.count].whiteclock = whiteclock;
        undo.moves[undo.count].blackclock = blackclock;
    } else {
        (undo.count = pas::increment(undo.count, 1));
        undo.max = undo.count;
        undo.moves[undo.count].data = data;
        undo.moves[undo.count].whiteclock = whiteclock;
        undo.moves[undo.count].blackclock = blackclock;
    }
}

// undoredo.pas:75
bool takeback(petir_undoredo::tundo& undo, petir_header::tdata& data, std::int32_t& wc, std::int32_t& bc, std::uint8_t& giliran) {
    bool return_value{};
    return_value = false;
    if ((undo.count > 0)) {
        return_value = true;
    }
    if ((undo.count == 1)) {
        (undo.count = pas::decrement(undo.count, 0));
        giliran = pas::sub(3, giliran);
    } else {
        (undo.count = pas::decrement(undo.count, 1));
    }
    giliran = pas::sub(3, giliran);
    data = undo.moves[undo.count].data;
    wc = undo.moves[undo.count].whiteclock;
    bc = undo.moves[undo.count].blackclock;
    return return_value;
}

// undoredo.pas:52
bool moveforward(petir_undoredo::tundo& undo, petir_header::tdata& data, std::int32_t& wc, std::int32_t& bc, std::uint8_t& giliran) {
    bool return_value{};
    return_value = false;
    if ((undo.count < undo.max)) {
        return_value = true;
        if ((undo.count == pas::sub(undo.max, 1))) {
            (undo.count = pas::increment(undo.count, 1));
        } else {
            (undo.count = pas::increment(undo.count, 1));
        }
        data = undo.moves[undo.count].data;
        wc = undo.moves[undo.count].whiteclock;
        bc = undo.moves[undo.count].blackclock;
        giliran = pas::sub(3, giliran);
    }
    return return_value;
}

}
