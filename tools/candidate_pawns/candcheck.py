"""Independent square-based pawn model plus cache and full-score checks.

usage: candcheck.py PYTHON_CHESS_DIR FENS EVALSHOW [SHOW]
The expected raw term is injected into a second full evaluation. Draw rules,
phase gating and rounding apply exactly, including a small term disappearing.
"""
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, sys.argv[1])
import chess

RANK_BONUS = [0, 4, 5, 8, 16, 30, 0, 0]
FILE_QUARTERS = [6, 5, 4, 4, 4, 4, 5, 6]
MATERIAL = {chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9}


def attacks(square, white):
    file, rank = chess.square_file(square), chess.square_rank(square)
    rank += 1 if white else -1
    return {chess.square(f, rank) for f in (file - 1, file + 1) if 0 <= f < 8 and 0 <= rank < 8}


def is_passed(square, enemy, white):
    file, rank = chess.square_file(square), chess.square_rank(square)
    step = 1 if white else -1
    return not any(abs(chess.square_file(p) - file) <= 1
                   and (chess.square_rank(p) - rank) * step > 0 for p in enemy)


def usable(square, helper, stopper, own, enemy, white):
    if square in attacks(helper, white) and square in attacks(stopper, not white):
        return True
    rank = chess.square_rank(square)
    step = 1 if white else -1
    helper_rank = chess.square_rank(helper)
    if (helper_rank - rank) * step > 1:
        return False
    occupied = own | enemy
    unsafe = set().union(*(attacks(p, not white) for p in enemy)) - set().union(*(attacks(p, white) for p in own))
    if (helper_rank - rank) * step == 1:
        return square + 8 * step not in unsafe
    for r in range(helper_rank + step, rank + step, step):
        target = chess.square(chess.square_file(helper), r)
        if target in occupied or target in unsafe:
            return False
    return True


def side_terms(board, white):
    own = set(board.pieces(chess.PAWN, white))
    enemy = set(board.pieces(chess.PAWN, not white))
    step = 1 if white else -1
    passed_files = {chess.square_file(p) for p in own if is_passed(p, enemy, white)}
    options = []
    for square in sorted(own):
        file, rank = chess.square_file(square), chess.square_rank(square)
        relative = rank if white else 7 - rank
        if not 1 <= relative <= 5:
            continue
        if any(chess.square_file(p) == file and (chess.square_rank(p) - rank) * step > 0 for p in own | enemy):
            continue
        stoppers = sorted(p for p in enemy if abs(chess.square_file(p) - file) == 1
                          and (chess.square_rank(p) - rank) * step > 0)
        if not 1 <= len(stoppers) <= 2:
            continue
        eligible = [{chess.square_file(p) for p in own if abs(chess.square_file(p) - file) == 1
                     and usable(square, p, stopper, own, enemy, white)} for stopper in stoppers]
        base = RANK_BONUS[relative] * FILE_QUARTERS[file] // 4
        if len(stoppers) == 1:
            choices = [{f} for f in eligible[0]]
        else:
            choices = [{a, b} for a in eligible[0] for b in eligible[1] if a != b]
        for helpers in {frozenset(c) for c in choices}:
            value = base // 2 if helpers & passed_files else base
            options.append((value, file, helpers, False))
    rank = 4 if white else 3
    for file in range(1, 7):
        trio = {chess.square(f, rank) for f in range(file - 1, file + 2)}
        targets = {chess.square(f, rank + 2 * step) for f in range(file - 1, file + 2)}
        gap = {chess.square(f, rank + step) for f in range(file - 1, file + 2)}
        if not trio <= own or not targets <= enemy or gap & (own | enemy):
            continue
        valid = True
        for flank in (-1, 1):
            remaining = enemy - {chess.square(file + flank, rank + 2 * step), chess.square(file, rank + 2 * step)}
            remaining |= {chess.square(file, rank + step), chess.square(file - flank, rank + step)}
            valid &= is_passed(chess.square(file + flank, rank + step), remaining, white)
        if valid:
            options.append((16, file, frozenset((file - 1, file + 1)), True))
    used, selected = set(), []
    options.sort(key=lambda item: (-item[0], item[1], sum(1 << f for f in item[2])))
    for value, file, helpers, breakthrough in options:
        resources = helpers | {file}
        if used & resources:
            continue
        used |= resources
        square = max((p for p in own if chess.square_file(p) == file), key=lambda p: chess.square_rank(p) * step)
        next_square = square + step * 8
        if not 0 <= next_square < 64:
            continue
        blocked = board.piece_at(next_square) is not None
        if breakthrough:
            king = board.king(not white)
            if king is not None and king in chess.SquareSet(chess.BB_KING_ATTACKS[next_square]):
                continue
            if blocked or board.piece_at(next_square - 1) or board.piece_at(next_square + 1):
                continue
        else:
            for helper_file in helpers:
                free = False
                for helper in own:
                    if chess.square_file(helper) != helper_file:
                        continue
                    if square in attacks(helper, white) and any(square in attacks(p, not white) for p in enemy):
                        free = True
                        break
                    helper_rank = chess.square_rank(helper)
                    if (helper_rank - chess.square_rank(square)) * step == 1:
                        free = True
                        break
                    if (helper_rank - chess.square_rank(square)) * step > 1:
                        continue
                    if all(board.piece_at(chess.square(helper_file, r)) is None
                           for r in range(helper_rank + step, chess.square_rank(square) + step, step)):
                        free = True
                        break
                blocked |= not free
        selected.append((square, value // 2 if blocked else value))
    return selected


def main():
    fens = [line.strip() for line in open(sys.argv[2]) if line.strip()]
    fens += fens[:]  # Revisit keys after later positions have evicted entries.
    expected = []
    for fen in fens:
        board = chess.Board(fen)
        expected.append(sum(v for _, v in side_terms(board, chess.WHITE))
                        - sum(v for _, v in side_terms(board, chess.BLACK)))
    payload = ''.join(f'{fen}|{term}\n' for fen, term in zip(fens, expected))
    run = subprocess.run([str(Path(sys.argv[3]).resolve())], input=payload, text=True, capture_output=True, check=True)
    lines = run.stdout.splitlines()
    if len(lines) != len(fens):
        raise RuntimeError(f'expected {len(fens)} output lines, got {len(lines)}: {run.stderr}')
    failures = 0
    show = int(sys.argv[4]) if len(sys.argv) > 4 else 10
    for fen, term, line in zip(fens, expected, lines):
        values = dict((key, int(value)) for key, value in (word.split('=') for word in line.split()))
        bad = values['term'] != term or values['fresh'] != term or not values['cache'] or values['white'] != values['reference']
        board = chess.Board(fen)
        pawnEnding = all(sum(v * len(board.pieces(piece, white)) for piece, v in MATERIAL.items()) == 0
                         for white in chess.COLORS)
        if not pawnEnding and values['white'] != values['without']:
            bad = True
        if bad:
            failures += 1
            if failures <= show:
                print(f'FAIL {fen}: expected raw {term:+}, {line}')
    print(f'{len(fens)} evaluations including revisits; {failures} failures (raw term, cache, full score including scaling)')
    return int(failures != 0)


if __name__ == '__main__':
    raise SystemExit(main())
