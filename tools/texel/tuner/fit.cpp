// Fits the weights of Petir's evaluation to labelled positions (Texel tuning).
//
// Input is a directory written by tools/texel/trace_labels.py: positions.tsv
// (game, ply, result, Stockfish score, FEN), trace.txt (Petir's evaluation and
// its coefficient for each weight, per position) and weights.tsv (the weights
// the trace was made at). The evaluation at changed weights w is predicted
// with the trace's linear model,
//   eval(w) = eval(w0) + sum of coefficient[i] * (w[i] - w0[i]),
// and turned into an expected score for White,
//   sigma(eval) = 1 / (1 + 10^(-K eval / 400)).
// The target blends the game result with Stockfish's score:
//   target = lambda * result + (1 - lambda) * sigma_sf(Stockfish score),
// where sigma_sf has the K that best maps Stockfish's scores to the game
// results of the training positions; mate scores count as 1 or 0. Petir's K is
// fitted to the targets at the starting weights and then kept, so the fit does
// not change the scale of the evaluation, which the search margins rely on.
//
// The fit minimises the mean squared difference between sigma(eval) and the
// target over the training positions, plus a small ridge that pulls each
// weight toward its start, with Gauss-Newton steps. It keeps each weight in
// its range and rounds the result. One game in --holdout is kept out of
// training, picked by game number so that positions of one game stay on one
// side, and the errors on those positions are reported too.
//
// Usage: texel_fit DIR OUT [options]
//   OUT              the fitted weights, written like weights.tsv
//   --lambda L       weight of the game result in the target (default 0.5)
//   --holdout N      keeps one game in N out of training (default 10; 0: none)
//   --min-count N    keeps a weight fixed when fewer than N training positions
//                    depend on it (default 100)
//   --ridge R        the pull toward the start: as strong as R positions at an
//                    even score that depend on the weight (default 100)
//   --split-ridge S  the pull toward the start on the split between a weight's
//                    middlegame and endgame values (untied weights, or name.mg
//                    and name.eg): as strong as ridge S, while their common
//                    level keeps --ridge (default: --ridge)
//   --freeze LIST    keeps the weights whose names start with one of the
//                    comma-separated prefixes fixed
//   --untie LIST     fits the weights whose names start with one of the
//                    prefixes with separate middlegame and endgame values,
//                    blended by the game phase (see --phase). OUT then has
//                    both columns; untie_weights.py turns them into the
//                    .mg/.eg weights of the tapered engine.
//   --phase N,B,R,Q,LOW,HIGH
//                    the game phase for --untie: each knight, bishop, rook
//                    and queen on the board adds N, B, R or Q; the sum is
//                    mapped from LOW (endgame) to HIGH (middlegame) and
//                    clamped (default 1,1,2,4,0,24)
//   --untied-range MIN,MAX
//                    the range of the untied values (default: the weight's)
//   --k K            uses this K for Petir instead of fitting it, for a second
//                    fit that starts from the first one's weights
//   --anchor FILE    the ridge pulls toward the values in FILE (written like
//                    weights.tsv) instead of the start: in a second fit, the
//                    engine's own weights, so that it does not drift further
//   --steps N        at most N Gauss-Newton steps (default 30)
//   --threads N      (default 1)
//   --report FILE    one line per weight: start, fitted, change, positions
#include <algorithm>
#include <charconv>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {
constexpr double ln10Over400 = 2.302585092994046 / 400.0;

[[noreturn]] void fail(const std::string& message) {
    std::cerr << "texel_fit: " << message << '\n';
    std::exit(2);
}

double expectedScore(double eval, double k) {
    return 1.0 / (1.0 + std::exp(-k * ln10Over400 * eval));
}

std::vector<std::string> split(const std::string& text, char separator) {
    std::vector<std::string> parts;
    std::string part;
    std::istringstream stream(text);
    while (std::getline(stream, part, separator))
        if (!part.empty()) parts.push_back(part);
    return parts;
}

bool startsWithAny(const std::string& name, const std::vector<std::string>& prefixes) {
    return std::any_of(prefixes.begin(), prefixes.end(),
                       [&](const std::string& prefix) { return name.rfind(prefix, 0) == 0; });
}

// How the game phase is measured (--phase).
struct PhaseFormula {
    int knight = 1, bishop = 1, rook = 2, queen = 4, low = 0, high = 24;
};

// The game phase from a FEN's piece placement: 1 in the middlegame, 0 in the
// endgame (pawns do not count).
double phaseOf(const std::string& fen, const PhaseFormula& formula) {
    int material = 0;
    for (const char square : fen.substr(0, fen.find(' '))) {
        switch (square) {
        case 'N': case 'n': material += formula.knight; break;
        case 'B': case 'b': material += formula.bishop; break;
        case 'R': case 'r': material += formula.rook; break;
        case 'Q': case 'q': material += formula.queen; break;
        }
    }
    return std::clamp(material - formula.low, 0, formula.high - formula.low) / static_cast<double>(formula.high - formula.low);
}

std::uint64_t mixed(std::uint64_t value) {
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

// Golden-section search for the minimum of a function of K.
template<class Error> double fitK(Error error, double low = 0.05, double high = 4.0) {
    const double ratio = (std::sqrt(5.0) - 1) / 2;
    double a = low, b = high;
    double c = b - ratio * (b - a), d = a + ratio * (b - a);
    for (int step = 0; step < 80; ++step) {
        if (error(c) < error(d)) b = d;
        else a = c;
        c = b - ratio * (b - a);
        d = a + ratio * (b - a);
    }
    return (a + b) / 2;
}

template<class Work> void inParallel(int threads, std::size_t count, Work work) {
    if (threads <= 1) {
        work(0, std::size_t{0}, count);
        return;
    }
    std::vector<std::thread> pool;
    for (int thread = 0; thread < threads; ++thread)
        pool.emplace_back([&, thread] { work(thread, count * thread / threads, count * (thread + 1) / threads); });
    for (auto& thread : pool) thread.join();
}

struct Weight {
    std::string name;
    int value = 0, minimum = 0, maximum = 0;
    long count = 0;           // training positions whose evaluation depends on it
    int middlegame = -1;      // parameter indices; equal when tied, -1 when fixed
    int endgame = -1;
    std::string fixedBecause; // empty when fitted
};

// A fitted number: a weight's change from its start, or the change of its
// middlegame or endgame value when untied.
struct Parameter {
    int weight;
    double minimum, maximum; // the change the weight's range allows
};

struct Data {
    std::vector<float> eval, result, stockfish, phase;
    std::vector<std::int8_t> mate; // +1 or -1 when Stockfish found a mate
    std::vector<std::uint8_t> heldOut;
    std::vector<std::uint32_t> rowStart{0};
    std::vector<std::uint16_t> weight;
    std::vector<float> coefficient;
    std::size_t size() const { return eval.size(); }
};

std::vector<Weight> readWeights(const std::string& fileName) {
    std::ifstream file(fileName);
    if (!file) fail("cannot open " + fileName);
    std::vector<Weight> weights;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line == "\r") continue;
        std::istringstream fields(line);
        std::size_t index;
        Weight weight;
        if (!(fields >> index >> weight.name >> weight.value >> weight.minimum >> weight.maximum) || index != weights.size())
            fail(fileName + ": bad line " + std::to_string(weights.size() + 1));
        weights.push_back(weight);
    }
    return weights;
}

Data readData(const std::string& dir, int holdout, std::size_t weightCount, const PhaseFormula& phase) {
    std::ifstream positions(dir + "/positions.tsv"), trace(dir + "/trace.txt");
    if (!positions || !trace) fail("cannot open positions.tsv and trace.txt in " + dir);
    Data data;
    std::string positionLine, traceLine;
    while (std::getline(positions, positionLine)) {
        if (!std::getline(trace, traceLine)) fail("trace.txt has fewer lines than positions.tsv");
        const auto fields = split(positionLine, '\t');
        if (fields.size() != 5) fail("positions.tsv: bad line " + std::to_string(data.size() + 1));
        const auto game = std::stoull(fields[0]);
        data.heldOut.push_back(holdout > 0 && mixed(game) % holdout == 0);
        data.result.push_back(std::stof(fields[2]));
        if (fields[3][0] == '#') {
            data.mate.push_back(std::stoi(fields[3].substr(1)) > 0 ? 1 : -1);
            data.stockfish.push_back(0);
        } else {
            data.mate.push_back(0);
            data.stockfish.push_back(std::stof(fields[3]));
        }
        data.phase.push_back(static_cast<float>(phaseOf(fields[4], phase)));

        if (!traceLine.empty() && traceLine.back() == '\r') traceLine.pop_back();
        const char* at = traceLine.data();
        const char* end = at + traceLine.size();
        int eval;
        auto parsed = std::from_chars(at, end, eval);
        if (parsed.ec != std::errc()) fail("trace.txt: bad line " + std::to_string(data.size() + 1));
        data.eval.push_back(static_cast<float>(eval));
        at = parsed.ptr;
        while (at < end) {
            unsigned index;
            float coefficient;
            if (*at != '\t') fail("trace.txt: bad line " + std::to_string(data.size()));
            parsed = std::from_chars(at + 1, end, index);
            if (parsed.ec != std::errc() || parsed.ptr >= end || *parsed.ptr != ':' || index >= weightCount)
                fail("trace.txt: bad line " + std::to_string(data.size()));
            parsed = std::from_chars(parsed.ptr + 1, end, coefficient);
            if (parsed.ec != std::errc()) fail("trace.txt: bad line " + std::to_string(data.size()));
            data.weight.push_back(static_cast<std::uint16_t>(index));
            data.coefficient.push_back(coefficient);
            at = parsed.ptr;
        }
        data.rowStart.push_back(static_cast<std::uint32_t>(data.weight.size()));
    }
    if (std::getline(trace, traceLine) && !traceLine.empty()) fail("trace.txt has more lines than positions.tsv");
    return data;
}

// The training positions in compressed rows over the fitted parameters.
struct Model {
    std::vector<double> target;
    std::vector<std::uint32_t> rowStart{0};
    std::vector<std::uint16_t> parameter;
    std::vector<float> coefficient;
};

struct Errors {
    double trainTarget = 0, heldTarget = 0, heldResult = 0;
};

class Fit {
public:
    Fit(const Data& data, const Model& model, std::size_t parameters, double k, int threads)
        : data_(data), model_(model), parameters_(parameters), k_(k), threads_(std::max(1, threads)) {
        for (auto held : data_.heldOut) training_ += !held;
    }

    // The errors at the given changes. With a gradient and a matrix, also the
    // gradient of the training error and its Gauss-Newton matrix (the
    // curvature without the term that the target's distance makes small),
    // parameters x parameters, row by row.
    Errors measure(const std::vector<double>& change, std::vector<double>* gradient, std::vector<double>* matrix) const {
        const bool derivatives = gradient && matrix;
        const std::size_t count = parameters_;
        std::vector<Errors> partial(threads_);
        std::vector<std::vector<double>> partialGradient(derivatives ? threads_ : 0, std::vector<double>(count));
        std::vector<std::vector<double>> partialMatrix(derivatives ? threads_ : 0, std::vector<double>(count * count));
        inParallel(threads_, data_.size(), [&](int thread, std::size_t first, std::size_t last) {
            Errors& errors = partial[thread];
            for (std::size_t row = first; row < last; ++row) {
                const auto begin = model_.rowStart[row], end = model_.rowStart[row + 1];
                double eval = data_.eval[row];
                for (auto entry = begin; entry < end; ++entry)
                    eval += model_.coefficient[entry] * change[model_.parameter[entry]];
                const double expected = expectedScore(eval, k_);
                const double miss = expected - model_.target[row];
                if (data_.heldOut[row]) {
                    errors.heldTarget += miss * miss;
                    const double resultMiss = expected - data_.result[row];
                    errors.heldResult += resultMiss * resultMiss;
                    continue;
                }
                errors.trainTarget += miss * miss;
                if (!derivatives) continue;
                const double slope = expected * (1 - expected);
                auto& sums = partialGradient[thread];
                auto& curvature = partialMatrix[thread];
                for (auto entry = begin; entry < end; ++entry) {
                    const double scaled = slope * model_.coefficient[entry];
                    const std::size_t parameter = model_.parameter[entry];
                    sums[parameter] += miss * scaled;
                    double* line = &curvature[parameter * count];
                    for (auto other = begin; other < end; ++other)
                        line[model_.parameter[other]] += scaled * slope * model_.coefficient[other];
                }
            }
        });
        Errors total;
        for (const auto& errors : partial) {
            total.trainTarget += errors.trainTarget;
            total.heldTarget += errors.heldTarget;
            total.heldResult += errors.heldResult;
        }
        const long held = static_cast<long>(data_.size()) - training_;
        total.trainTarget /= std::max(1L, training_);
        total.heldTarget /= std::max(1L, held);
        total.heldResult /= std::max(1L, held);
        if (derivatives) {
            // d expected / d eval = expected (1 - expected) K ln(10) / 400.
            const double unit = k_ * ln10Over400, training = static_cast<double>(std::max(1L, training_));
            gradient->assign(count, 0.0);
            matrix->assign(count * count, 0.0);
            for (const auto& sums : partialGradient)
                for (std::size_t index = 0; index < count; ++index) (*gradient)[index] += sums[index] * 2 * unit / training;
            for (const auto& curvature : partialMatrix)
                for (std::size_t index = 0; index < count * count; ++index)
                    (*matrix)[index] += curvature[index] * 2 * unit * unit / training;
        }
        return total;
    }

private:
    const Data& data_;
    const Model& model_;
    std::size_t parameters_;
    double k_;
    int threads_;
    long training_ = 0;
};

// Solves matrix * x = right in place of `right`, for a symmetric positive
// definite n x n matrix stored row by row. False when it is not positive
// definite.
bool solveCholesky(std::vector<double> matrix, std::vector<double>& right, std::size_t n) {
    for (std::size_t column = 0; column < n; ++column) {
        double diagonal = matrix[column * n + column];
        for (std::size_t inner = 0; inner < column; ++inner) diagonal -= matrix[column * n + inner] * matrix[column * n + inner];
        if (!(diagonal > 0)) return false;
        const double root = std::sqrt(diagonal);
        matrix[column * n + column] = root;
        for (std::size_t row = column + 1; row < n; ++row) {
            double sum = matrix[row * n + column];
            for (std::size_t inner = 0; inner < column; ++inner) sum -= matrix[row * n + inner] * matrix[column * n + inner];
            matrix[row * n + column] = sum / root;
        }
    }
    for (std::size_t row = 0; row < n; ++row) {
        for (std::size_t inner = 0; inner < row; ++inner) right[row] -= matrix[row * n + inner] * right[inner];
        right[row] /= matrix[row * n + row];
    }
    for (std::size_t row = n; row-- > 0;) {
        for (std::size_t inner = row + 1; inner < n; ++inner) right[row] -= matrix[inner * n + row] * right[inner];
        right[row] /= matrix[row * n + row];
    }
    return true;
}

std::string percentText(double part, double whole) {
    char text[32];
    std::snprintf(text, sizeof text, "%.1f%%", 100.0 * part / whole);
    return text;
}
}

int main(int argc, char** argv) {
    std::vector<std::string> positional;
    double lambda = 0.5, ridge = 100, splitRidge = -1, fixedK = 0;
    int holdout = 10, steps = 30, threads = 1;
    long minimumCount = 100;
    std::vector<std::string> freeze, untie;
    std::string reportName, anchorName;
    PhaseFormula phase;
    bool untiedRangeGiven = false;
    int untiedMinimum = 0, untiedMaximum = 0;
    for (int index = 1; index < argc; ++index) {
        const std::string arg = argv[index];
        if (arg.rfind("--", 0) != 0) {
            positional.push_back(arg);
            continue;
        }
        if (index + 1 >= argc) fail(arg + " needs a value");
        const std::string value = argv[++index];
        if (arg == "--lambda") lambda = std::stod(value);
        else if (arg == "--holdout") holdout = std::stoi(value);
        else if (arg == "--min-count") minimumCount = std::stol(value);
        else if (arg == "--freeze") freeze = split(value, ',');
        else if (arg == "--untie") untie = split(value, ',');
        else if (arg == "--k") fixedK = std::stod(value);
        else if (arg == "--ridge") ridge = std::stod(value);
        else if (arg == "--split-ridge") splitRidge = std::stod(value);
        else if (arg == "--steps") steps = std::stoi(value);
        else if (arg == "--threads") threads = std::stoi(value);
        else if (arg == "--report") reportName = value;
        else if (arg == "--anchor") anchorName = value;
        else if (arg == "--phase") {
            const auto parts = split(value, ',');
            if (parts.size() != 6) fail("--phase needs N,B,R,Q,LOW,HIGH");
            phase = {std::stoi(parts[0]), std::stoi(parts[1]), std::stoi(parts[2]), std::stoi(parts[3]),
                     std::stoi(parts[4]), std::stoi(parts[5])};
            if (phase.high <= phase.low) fail("--phase: HIGH must be above LOW");
        } else if (arg == "--untied-range") {
            const auto parts = split(value, ',');
            if (parts.size() != 2) fail("--untied-range needs MIN,MAX");
            untiedRangeGiven = true;
            untiedMinimum = std::stoi(parts[0]);
            untiedMaximum = std::stoi(parts[1]);
        }
        else fail("unknown option " + arg);
    }
    if (positional.size() != 2) {
        std::cerr << "usage: texel_fit DIR OUT [--lambda L] [--holdout N] [--min-count N] [--freeze PREFIXES]\n"
                     "                 [--untie PREFIXES] [--phase N,B,R,Q,LOW,HIGH] [--untied-range MIN,MAX]\n"
                     "                 [--k K] [--ridge R] [--split-ridge S] [--steps N] [--threads N]\n"
                     "                 [--anchor FILE] [--report FILE]\n";
        return 2;
    }
    const std::string dir = positional[0];
    auto weights = readWeights(dir + "/weights.tsv");
    const Data data = readData(dir, holdout, weights.size(), phase);
    long training = 0;
    for (auto held : data.heldOut) training += !held;
    const long held = static_cast<long>(data.size()) - training;
    if (training == 0) fail("no training positions");
    std::printf("%zu positions: %ld for training, %ld held out (one game in %d, by game number)\n", data.size(),
                training, held, holdout);

    // Stockfish's scale, from the training positions' game results.
    auto stockfishExpected = [&](std::size_t row, double k) {
        if (data.mate[row]) return data.mate[row] > 0 ? 1.0 : 0.0;
        return expectedScore(data.stockfish[row], k);
    };
    auto trainingMean = [&](auto value) {
        double sum = 0;
        for (std::size_t row = 0; row < data.size(); ++row)
            if (!data.heldOut[row]) sum += value(row);
        return sum / training;
    };
    const double stockfishK = fitK([&](double k) {
        return trainingMean([&](std::size_t row) { return std::pow(stockfishExpected(row, k) - data.result[row], 2); });
    });

    Model model;
    model.target.resize(data.size());
    for (std::size_t row = 0; row < data.size(); ++row)
        model.target[row] = lambda * data.result[row] + (1 - lambda) * stockfishExpected(row, stockfishK);
    const double k = fixedK > 0 ? fixedK : fitK([&](double kTried) {
        return trainingMean([&](std::size_t row) { return std::pow(expectedScore(data.eval[row], kTried) - model.target[row], 2); });
    });

    // Which weights are fitted, and as how many parameters.
    for (std::size_t row = 0; row < data.size(); ++row)
        if (!data.heldOut[row])
            for (auto entry = data.rowStart[row]; entry < data.rowStart[row + 1]; ++entry) ++weights[data.weight[entry]].count;
    std::vector<Parameter> parameters;
    for (std::size_t index = 0; index < weights.size(); ++index) {
        auto& weight = weights[index];
        if (startsWithAny(weight.name, freeze)) weight.fixedBecause = "frozen";
        else if (weight.count < minimumCount) weight.fixedBecause = "few positions";
        if (!weight.fixedBecause.empty()) continue;
        Parameter parameter{static_cast<int>(index), static_cast<double>(weight.minimum - weight.value),
                            static_cast<double>(weight.maximum - weight.value)};
        const bool untied = startsWithAny(weight.name, untie);
        if (untied && untiedRangeGiven) {
            weight.minimum = untiedMinimum;
            weight.maximum = untiedMaximum;
            parameter.minimum = untiedMinimum - weight.value;
            parameter.maximum = untiedMaximum - weight.value;
        }
        weight.middlegame = weight.endgame = static_cast<int>(parameters.size());
        parameters.push_back(parameter);
        if (untied) {
            weight.endgame = static_cast<int>(parameters.size());
            parameters.push_back(parameter);
        }
    }
    if (parameters.size() > 65535) fail("too many parameters");
    for (std::size_t row = 0; row < data.size(); ++row) {
        for (auto entry = data.rowStart[row]; entry < data.rowStart[row + 1]; ++entry) {
            const auto& weight = weights[data.weight[entry]];
            const float coefficient = data.coefficient[entry];
            if (weight.middlegame < 0) continue;
            if (weight.middlegame == weight.endgame) {
                model.parameter.push_back(static_cast<std::uint16_t>(weight.middlegame));
                model.coefficient.push_back(coefficient);
            } else {
                model.parameter.push_back(static_cast<std::uint16_t>(weight.middlegame));
                model.coefficient.push_back(coefficient * data.phase[row]);
                model.parameter.push_back(static_cast<std::uint16_t>(weight.endgame));
                model.coefficient.push_back(coefficient * (1 - data.phase[row]));
            }
        }
        model.rowStart.push_back(static_cast<std::uint32_t>(model.parameter.size()));
    }
    long fixedCount = 0;
    for (const auto& weight : weights) fixedCount += !weight.fixedBecause.empty();
    std::printf("%zu weights: %zu parameters fitted, %ld weights kept fixed (fewer than %ld positions or frozen)\n",
                weights.size(), parameters.size(), fixedCount, minimumCount);
    std::printf("lambda %.2f; Stockfish K %.3f; Petir K %.3f%s\n", lambda, stockfishK, k, fixedK > 0 ? " (given)" : "");
    if (!untie.empty())
        std::printf("phase: knight %d, bishop %d, rook %d, queen %d, from %d to %d\n", phase.knight, phase.bishop, phase.rook,
                    phase.queen, phase.low, phase.high);

    // Reference errors on the held-out positions.
    if (held) {
        double constantError = 0, stockfishError = 0, mean = trainingMean([&](std::size_t row) { return data.result[row]; });
        for (std::size_t row = 0; row < data.size(); ++row) {
            if (!data.heldOut[row]) continue;
            constantError += std::pow(mean - data.result[row], 2);
            stockfishError += std::pow(stockfishExpected(row, stockfishK) - data.result[row], 2);
        }
        std::printf("held-out error against the result: constant %.5f, Stockfish %.5f\n", constantError / held,
                    stockfishError / held);
    }

    const Fit fit(data, model, parameters.size(), k, threads);
    const std::size_t count = parameters.size();
    std::vector<double> change(count, 0.0), gradient, matrix;
    Errors current = fit.measure(change, &gradient, &matrix);
    std::printf("\n%10s %14s %14s %14s\n", "", "train target", "held target", "held result");
    std::printf("%10s %14.6f %14.6f %14.6f\n", "start", current.trainTarget, current.heldTarget, current.heldResult);

    // Gauss-Newton steps on the training error plus the ridge, which adds
    // ridgeCurvature / 2 * change^2 per parameter: as much curvature as
    // `ridge` training positions at an even score with coefficient 1. It pulls
    // weights that few positions depend on toward their start, and it keeps
    // the changes that leave every evaluation the same (raising the pawn value
    // while lowering every pawn square, or a whole count table on both sides)
    // at zero. A parameter at a limit of its range that the gradient pushes
    // further out sits the step out; each step is halved until it lowers the
    // objective.
    // With --anchor the ridge pulls toward the anchor's values instead.
    std::vector<double> anchor(count, 0.0);
    if (!anchorName.empty()) {
        const auto anchorWeights = readWeights(anchorName);
        if (anchorWeights.size() != weights.size()) fail(anchorName + " does not list the same weights");
        for (std::size_t index = 0; index < weights.size(); ++index) {
            const auto& weight = weights[index];
            if (anchorWeights[index].name != weight.name) fail(anchorName + " does not list the same weights");
            if (weight.middlegame < 0) continue;
            anchor[weight.middlegame] = anchor[weight.endgame] = anchorWeights[index].value - weight.value;
        }
    }
    const double evenSlope = 0.25 * k * ln10Over400;
    const double ridgeCurvature = ridge * 2 * evenSlope * evenSlope / training;
    // --split-ridge: with x and y the middlegame and endgame changes, the
    // ridge's x^2 + y^2 is 2 level^2 + 2 half-split^2. The extra
    // splitCurvature / 2 * (x - y)^2 raises the pull on the split alone to
    // that of ridge S.
    if (splitRidge < 0) splitRidge = ridge;
    const double splitCurvature = (splitRidge - ridge) * evenSlope * evenSlope / training;
    std::vector<std::pair<int, int>> pairs;
    for (std::size_t index = 0; index < weights.size(); ++index) {
        const auto& weight = weights[index];
        if (weight.middlegame >= 0 && weight.middlegame != weight.endgame) pairs.push_back({weight.middlegame, weight.endgame});
        const auto& name = weight.name;
        if (weight.middlegame < 0 || name.size() < 3 || name.compare(name.size() - 3, 3, ".mg") != 0) continue;
        for (const auto& other : weights)
            if (other.middlegame >= 0 && other.name == name.substr(0, name.size() - 3) + ".eg")
                pairs.push_back({weight.middlegame, other.middlegame});
    }
    if (splitRidge != ridge)
        std::printf("split ridge %g on %zu middlegame/endgame pairs (ridge %g)\n", splitRidge, pairs.size(), ridge);
    auto objective = [&](const Errors& errors, const std::vector<double>& changes) {
        double squares = 0, splits = 0;
        for (std::size_t index = 0; index < count; ++index)
            squares += (changes[index] - anchor[index]) * (changes[index] - anchor[index]);
        for (const auto& [middle, end] : pairs) {
            const double split = (changes[middle] - anchor[middle]) - (changes[end] - anchor[end]);
            splits += split * split;
        }
        return errors.trainTarget + ridgeCurvature / 2 * squares + splitCurvature / 2 * splits;
    };
    double currentObjective = objective(current, change);
    for (int step = 1; step <= steps; ++step) {
        std::vector<std::size_t> free;
        for (const auto& [middle, end] : pairs) {
            const double split = (change[middle] - anchor[middle]) - (change[end] - anchor[end]);
            gradient[middle] += splitCurvature * split;
            gradient[end] -= splitCurvature * split;
            matrix[middle * count + middle] += splitCurvature;
            matrix[end * count + end] += splitCurvature;
            matrix[middle * count + end] -= splitCurvature;
            matrix[end * count + middle] -= splitCurvature;
        }
        for (std::size_t index = 0; index < count; ++index) {
            gradient[index] += ridgeCurvature * (change[index] - anchor[index]);
            matrix[index * count + index] += ridgeCurvature;
            const bool pinned = (change[index] <= parameters[index].minimum && gradient[index] > 0)
                             || (change[index] >= parameters[index].maximum && gradient[index] < 0);
            if (!pinned) free.push_back(index);
        }
        std::vector<double> system(free.size() * free.size()), direction(free.size());
        for (std::size_t row = 0; row < free.size(); ++row) {
            direction[row] = -gradient[free[row]];
            for (std::size_t column = 0; column < free.size(); ++column)
                system[row * free.size() + column] = matrix[free[row] * count + free[column]];
        }
        if (!solveCholesky(system, direction, free.size())) fail("the curvature is not positive definite; raise --ridge");
        std::vector<double> tried;
        Errors triedErrors;
        double triedObjective = currentObjective, size = 1;
        for (int halving = 0; halving < 30 && triedObjective >= currentObjective; ++halving, size /= 2) {
            tried = change;
            for (std::size_t row = 0; row < free.size(); ++row) {
                const auto index = free[row];
                tried[index] = std::clamp(change[index] + size * direction[row], parameters[index].minimum,
                                          parameters[index].maximum);
            }
            triedErrors = fit.measure(tried, nullptr, nullptr);
            triedObjective = objective(triedErrors, tried);
        }
        if (triedObjective >= currentObjective) break;
        double largest = 0;
        for (std::size_t index = 0; index < count; ++index) largest = std::max(largest, std::abs(tried[index] - change[index]));
        const double gain = currentObjective - triedObjective;
        change = tried;
        currentObjective = triedObjective;
        std::fprintf(stderr, "  step %2d: train %.6f, held %.6f, objective %.7f, largest change %.2f\n", step,
                     triedErrors.trainTarget, triedErrors.heldTarget, triedObjective, largest);
        if (largest < 0.01 || gain < 1e-10) break;
        current = fit.measure(change, &gradient, &matrix);
    }
    const Errors fitted = fit.measure(change, nullptr, nullptr);
    std::vector<double> rounded(count);
    for (std::size_t index = 0; index < count; ++index) rounded[index] = std::round(change[index]);
    const Errors roundedErrors = fit.measure(rounded, nullptr, nullptr);
    std::printf("%10s %14.6f %14.6f %14.6f\n", "fitted", fitted.trainTarget, fitted.heldTarget, fitted.heldResult);
    std::printf("%10s %14.6f %14.6f %14.6f\n", "rounded", roundedErrors.trainTarget, roundedErrors.heldTarget,
                roundedErrors.heldResult);

    // The fitted values.
    auto valueOf = [&](const Weight& weight, int parameter) {
        return parameter < 0 ? weight.value : weight.value + static_cast<int>(rounded[parameter]);
    };
    std::ofstream out(positional[1]);
    if (!out) fail("cannot write " + positional[1]);
    for (std::size_t index = 0; index < weights.size(); ++index) {
        const auto& weight = weights[index];
        out << index << '\t' << weight.name << '\t' << valueOf(weight, weight.middlegame);
        if (!untie.empty()) out << '\t' << valueOf(weight, weight.endgame);
        out << '\t' << weight.minimum << '\t' << weight.maximum << '\n';
    }

    // The largest changes, weighted by how many positions they touch.
    std::vector<std::size_t> order(weights.size());
    std::iota(order.begin(), order.end(), 0);
    auto moved = [&](const Weight& weight) {
        return std::max(std::abs(valueOf(weight, weight.middlegame) - weight.value),
                        std::abs(valueOf(weight, weight.endgame) - weight.value));
    };
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b) {
        return moved(weights[a]) * std::sqrt(static_cast<double>(weights[a].count))
             > moved(weights[b]) * std::sqrt(static_cast<double>(weights[b].count));
    });
    std::printf("\nlargest changes (by change x sqrt(positions)):\n");
    for (std::size_t rank = 0; rank < std::min<std::size_t>(25, order.size()); ++rank) {
        const auto& weight = weights[order[rank]];
        std::printf("  %-40s %5d -> %5d", weight.name.c_str(), weight.value, valueOf(weight, weight.middlegame));
        if (weight.endgame != weight.middlegame) std::printf(" / %5d", valueOf(weight, weight.endgame));
        std::printf("  in %s of positions\n", percentText(static_cast<double>(weight.count), training).c_str());
    }

    if (!reportName.empty()) {
        std::ofstream report(reportName);
        report << "index\tname\tstart\tfitted" << (untie.empty() ? "" : "\tendgame") << "\tchange\tpositions\tnote\n";
        for (std::size_t index = 0; index < weights.size(); ++index) {
            const auto& weight = weights[index];
            const int value = valueOf(weight, weight.middlegame);
            report << index << '\t' << weight.name << '\t' << weight.value << '\t' << value;
            if (!untie.empty()) report << '\t' << valueOf(weight, weight.endgame);
            report << '\t' << value - weight.value << '\t' << weight.count << '\t';
            if (!weight.fixedBecause.empty()) report << weight.fixedBecause;
            else if (value == weight.minimum || value == weight.maximum) report << "at its limit";
            report << '\n';
        }
    }
    return 0;
}
