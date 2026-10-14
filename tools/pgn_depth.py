"""Average search depth per engine from a cutechess PGN (move comments {score/depth time}).

Usage: pgn_depth.py FILE.pgn
Reads cutechess move comments {score/depth time}. Skips book moves and
zero-depth moves. Splits by move number (1-20, 21-40, 41+); pgn_phases.py
splits by material instead.
"""
import re
import statistics
import sys
from collections import defaultdict

text = open(sys.argv[1], encoding="utf-8", errors="replace").read()
games = re.split(r"\n(?=\[Event )", text)
depths = defaultdict(list)
byBand = defaultdict(lambda: defaultdict(list))
timeUsed = defaultdict(list)
completed = 0
for game in games:
    white = re.search(r'\[White "([^"]+)"\]', game)
    black = re.search(r'\[Black "([^"]+)"\]', game)
    result = re.search(r'\[Result "([^"]+)"\]', game)
    if not (white and black and result) or result.group(1) == "*":
        continue
    completed += 1
    body = game[game.find("\n\n"):]
    # Walk tokens: move numbers set the side; each {comment} belongs to the last move.
    side, moveNo = 0, 1
    for token in re.finditer(r"(\d+)\.(\.\.)?|\{([^}]*)\}|(\S+)", body):
        if token.group(1):
            moveNo = int(token.group(1))
            side = 1 if token.group(2) else 0
            continue
        if token.group(3) is not None:
            c = token.group(3).strip()
            m = re.match(r"([+-]?(?:M)?\d+(?:\.\d+)?)/(\d+)\s+([\d.]+)s", c)
            engine = (white if side == 0 else black).group(1)
            if m and int(m.group(2)) > 0:
                d = int(m.group(2))
                depths[engine].append(d)
                band = "1-20" if moveNo <= 20 else "21-40" if moveNo <= 40 else "41+"
                byBand[engine][band].append(d)
                timeUsed[engine].append(float(m.group(3)))
            side ^= 1
            continue
        tok = token.group(4)
        if tok in ("1-0", "0-1", "1/2-1/2", "*"):
            continue

print(f"{completed} completed games")
print(f"{'engine':<14} {'moves':>7} {'mean':>6} {'median':>6} {'1-20':>6} {'21-40':>6} {'41+':>6} {'s/move':>7}")
for engine in sorted(depths):
    d = depths[engine]
    bands = [statistics.mean(byBand[engine][b]) if byBand[engine][b] else float("nan") for b in ("1-20", "21-40", "41+")]
    print(f"{engine:<14} {len(d):>7} {statistics.mean(d):>6.2f} {statistics.median(d):>6.1f} "
          + " ".join(f"{x:>6.2f}" for x in bands) + f" {statistics.mean(timeUsed[engine]):>7.3f}")
