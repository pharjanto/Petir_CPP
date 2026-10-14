"""Where a cutechess match's Elo was won or lost, by game phase.

Phase classes on the board at each ply (N/B 1, R 2, Q 4):
  Q   = a queen on the board
  MG  = no queens, phase >= 7
  EG  = no queens, phase <= 6
Expected score per ply = mean of both engines' last evals (candidate view),
mapped by 1/(1+10^(-cp/400)). Change of expected score inside each class
is summed over games (last segment ends at the game result).
Also mean search depth per class for each engine, and the class in which each
decisive game first reached an average eval of 200 cp or more ("decisive turns").

Needs python-chess: pass the directory it is unpacked in with --pylib DIR if it
is not installed (the portable Python has no pip; unzip the wheel).
Usage: pgn_phases.py FILE.pgn CANDIDATE_NAME [--pylib DIR]
"""
import sys, re, statistics
from collections import defaultdict

args = sys.argv[1:]
if "--pylib" in args:
    at = args.index("--pylib")
    sys.path.insert(0, args[at + 1])
    del args[at:at + 2]
import chess, chess.pgn

path, cand = args[0], args[1]
def phaseClass(b):
    q = len(b.pieces(chess.QUEEN, 1)) + len(b.pieces(chess.QUEEN, 0))
    if q: return "Q"
    ph = sum(len(b.pieces(t, c)) * w for t, w in ((chess.KNIGHT, 1), (chess.BISHOP, 1), (chess.ROOK, 2)) for c in (0, 1))
    return "MG" if ph >= 7 else "EG"
def es(cp): return 1 / (1 + 10 ** (-max(-2000, min(2000, cp)) / 400))
scoreRe = re.compile(r"^([+-]?)(M?)(\d+(?:\.\d+)?)/(\d+)")

drift = defaultdict(float); plies = defaultdict(int); depth = defaultdict(lambda: defaultdict(list))
turn = defaultdict(lambda: defaultdict(int))
games = 0; total = 0.0
with open(path, encoding="utf-8", errors="replace") as f:
    while (g := chess.pgn.read_game(f)) is not None:
        res = g.headers.get("Result")
        if res not in ("1-0", "0-1", "1/2-1/2"): continue
        candWhite = g.headers["White"] == cand
        candScore = {"1-0": 1.0, "0-1": 0.0, "1/2-1/2": 0.5}[res]
        if not candWhite: candScore = 1 - candScore
        games += 1; total += candScore
        b = g.board(); last = {True: None, False: None}  # eval by "is candidate", candidate view
        prevE = None; prevCls = None; turned = None
        for node in g.mainline():
            mover = b.turn; isCand = (mover == chess.WHITE) == candWhite
            cls = phaseClass(b)
            m = scoreRe.match(node.comment.strip())
            if m:
                sign = -1 if m.group(1) == "-" else 1
                cp = sign * (20000 if m.group(2) else float(m.group(3)) * 100)
                depth[cls][isCand].append(int(m.group(4)))
                last[isCand] = cp if isCand else -cp
            if last[True] is not None and last[False] is not None:
                e = es((last[True] + last[False]) / 2)
                if prevE is not None: drift[prevCls] += e - prevE
                prevE, prevCls = e, cls
                plies[cls] += 1
                avg = (last[True] + last[False]) / 2
                if turned is None and abs(avg) >= 200: turned = (cls, avg > 0)
            b.push(node.move)
        if prevE is not None: drift[prevCls] += candScore - prevE
        if candScore != 0.5 and turned is not None:
            turn[turned[0]]["win" if candScore == 1 else "loss"] += 1

print(f"{games} games, candidate score {total/games:.4f} ({total - games/2:+.1f} points vs 50%)")
print(f"{'class':<4} {'plies':>8} {'points drift':>13} {'depth cand':>11} {'depth base':>11} {'diff':>6} {'decisive turns W/L':>20}")
for cls in ("Q", "MG", "EG"):
    dc, db = depth[cls][True], depth[cls][False]
    mc = statistics.mean(dc) if dc else float('nan'); mb = statistics.mean(db) if db else float('nan')
    print(f"{cls:<4} {plies[cls]:>8} {drift[cls]:>+13.1f} {mc:>11.2f} {mb:>11.2f} {mc-mb:>+6.2f} {turn[cls]['win']:>9}/{turn[cls]['loss']}")
