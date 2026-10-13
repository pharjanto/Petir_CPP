// Each input line is FEN|expected raw term from the independent Python model.
#include "engine.hpp"
#include <iostream>
#include <string>
namespace petir_ueval {
int candidatePasserTermForTest(const petir_header::tdata&, bool);
bool candidateCacheMatchesForTest(const petir_header::tdata&);
int evaluateInjectedPotentialForTest(const petir_header::tdata&, int, int);
}
int main() {
    petir::initialize("books");
    for (std::string line; std::getline(std::cin, line);) {
        if (line.empty()) continue;
        const auto separator = line.rfind('|');
        if (separator == std::string::npos) return 2;
        int side = 1;
        auto position = petir::fromFen(line.substr(0, separator), side);
        const int expected = std::stoi(line.substr(separator + 1));
        const int raw = petir_ueval::candidatePasserTermForTest(position, true);
        const int fresh = petir_ueval::candidatePasserTermForTest(position, false);
        const bool cacheMatches = petir_ueval::candidateCacheMatchesForTest(position);
        const int score = petir_ueval::evaluate(position, side, -20500, 20500);
        std::cout << "white=" << (side == 1 ? score : -score)
                  << " term=" << raw << " fresh=" << fresh << " cache=" << cacheMatches
                  << " reference=" << petir_ueval::evaluateInjectedPotentialForTest(position, side, expected)
                  << " without=" << petir_ueval::evaluateInjectedPotentialForTest(position, side, 0) << '\n';
    }
}
