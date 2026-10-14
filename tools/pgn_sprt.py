"""Score, Elo and SPRT log-likelihood ratio of a cutechess match PGN, at any point
of the match (finished games only), for the [0, 10] Elo SPRT with alpha = beta = 0.05.

Usage: pgn_sprt.py FILE.pgn CANDIDATE_NAME
W/L/D are from CANDIDATE_NAME's side. The LLR is the trinomial GSPRT
approximation on logistic Elo, close to what cutechess-cli prints.
"""
import math
import re
import sys

path, candidate = sys.argv[1], sys.argv[2]
text = open(path, encoding="utf-8", errors="replace").read()
wins = losses = draws = 0
for game in re.split(r"\n(?=\[Event )", text):
    white = re.search(r'\[White "([^"]+)"\]', game)
    result = re.search(r'\[Result "([^"]+)"\]', game)
    if not (white and result) or result.group(1) == "*":
        continue
    if result.group(1) == "1/2-1/2":
        draws += 1
    elif (result.group(1) == "1-0") == (white.group(1) == candidate):
        wins += 1
    else:
        losses += 1
games = wins + losses + draws
if games == 0:
    sys.exit("no finished games")
score = (wins + 0.5 * draws) / games
variance = (wins * (1 - score) ** 2 + losses * score ** 2 + draws * (0.5 - score) ** 2) / games
error = math.sqrt(variance / games)


def elo(s):
    return -400 * math.log10(1 / s - 1) if 0 < s < 1 else math.copysign(math.inf, s - 0.5)


def expected(e):
    return 1 / (1 + 10 ** (-e / 400))


llr = (expected(10) - expected(0)) * (2 * score - expected(0) - expected(10)) / (2 * variance / games) if variance > 0 else 0
bound = math.log(0.95 / 0.05)
print(f"games {games}  W {wins} L {losses} D {draws}  score {score:.4f}  elo {elo(score):+.1f} "
      f"[{elo(score - 1.96 * error):+.1f}, {elo(score + 1.96 * error):+.1f}]  LLR {llr:+.2f} ({-bound:+.2f}, {bound:+.2f})")
