// What evaluate() saw at the lazy checkpoint, written by the probed copies of
// ueval.cpp that prepare_trees.py makes (src/ has no probe).
#pragma once
struct LazyProbe {
    bool reached;   // the checkpoint ran (null window, no draw scaling needed)
    int partial;    // side-relative score at the checkpoint (termScore)
    int margin;     // base (280 in 4ce0793) + lazyMargin
    int phase;      // 0 middlegame, 1 early endgame, 2 endgame
    int trades;     // lazyMarginForTrades (0 .. -120)
};
