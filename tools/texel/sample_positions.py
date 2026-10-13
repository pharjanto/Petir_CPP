"""Samples labelled positions from a PGN database for Texel tuning.

Games are picked by a seeded hash of their index in the file, so a sample is
spread over the whole database and the same seed always picks the same games.
From each picked game up to --per-game positions are taken at random among the
eligible ones. A position is eligible when:

  - the game has a result (1-0, 0-1, 1/2-1/2) and starts from the normal
    initial position;
  - it comes after the first --skip-plies plies (CCRL's book is at most 12
    moves per side, so the default 24 skips it);
  - the side to move is not in check, the move that led to it was not a
    capture or a promotion, and the side to move has no capture of a more
    valuable piece and no capture of an undefended piece.

The last condition is only a cheap stand-in for a quiescence check: the real
filter (Petir's quiescence score equal to its static eval) comes later.
Duplicate positions (same pieces, side to move, castling and en passant) keep
their first occurrence.

Output is JSON lines, one position each, with the result from White's view:
  {"fen", "result", "game", "ply", "white", "black", "whiteElo", "blackElo"}

Usage: sample_positions.py PGN OUT --fraction F [--per-game N] [--seed S]
                           [--skip-plies N] [--min-elo E] [--limit N]
       sample_positions.py --mirror IN OUT   (colour-flipped copy of a sample)
"""
import argparse
import collections
import hashlib
import io
import json
import random
import sys
from pathlib import Path

PYTHON_CHESS = Path(__file__).resolve().parents[2] / 'build' / 'kpk-audit' / 'python-chess-master'
try:
    import chess
    import chess.pgn
except ImportError:
    sys.path.insert(0, str(PYTHON_CHESS))
    import chess
    import chess.pgn

RESULTS = {'1-0': 1.0, '0-1': 0.0, '1/2-1/2': 0.5}
VALUES = {chess.PAWN: 1, chess.KNIGHT: 3, chess.BISHOP: 3, chess.ROOK: 5, chess.QUEEN: 9, chess.KING: 100}


def game_texts(stream):
    """Yields the text of each game: a header block followed by its movetext."""
    lines, in_moves = [], False
    for line in stream:
        if line.startswith('['):
            if in_moves:
                yield ''.join(lines)
                lines, in_moves = [], False
        elif line.strip():
            in_moves = True
        lines.append(line)
    if in_moves:
        yield ''.join(lines)


def unit(seed, *parts):
    """A number in [0, 1) fixed by the seed and the parts."""
    digest = hashlib.blake2b(':'.join(map(str, (seed,) + parts)).encode(), digest_size=8).digest()
    return int.from_bytes(digest, 'little') / 2 ** 64


def position_key(board):
    return ' '.join(board.fen().split()[:4])


def has_winning_capture(board):
    """True when the side to move can take a more valuable or an undefended piece."""
    for move in board.generate_legal_captures():
        attacker = VALUES[board.piece_type_at(move.from_square)]
        if board.is_en_passant(move):
            victim, defended = 1, True
        else:
            victim = VALUES[board.piece_type_at(move.to_square)]
            defended = board.is_attacked_by(not board.turn, move.to_square)
        if victim > attacker or not defended:
            return True
    return False


def eligible_positions(game, skip_plies):
    """Yields (ply, fen) for every eligible position of a game."""
    board = game.board()
    for ply, move in enumerate(game.mainline_moves(), 1):
        noisy_move = board.is_capture(move) or move.promotion is not None
        board.push(move)
        if ply <= skip_plies or noisy_move or board.is_check() or board.is_game_over(claim_draw=False):
            continue
        if has_winning_capture(board):
            continue
        yield ply, board.fen()


def elo(headers, key):
    try:
        return int(headers.get(key, ''))
    except ValueError:
        return None


def sample(args):
    counts = collections.Counter()
    seen, rows = set(), []
    with open(args.pgn, encoding='utf-8', errors='replace') as stream:
        for index, text in enumerate(game_texts(stream)):
            counts['games'] += 1
            if unit(args.seed, 'game', index) >= args.fraction:
                continue
            counts['picked'] += 1
            game = chess.pgn.read_game(io.StringIO(text))
            if game is None or game.errors:
                counts['unreadable'] += 1
                continue
            headers = game.headers
            result = RESULTS.get(headers.get('Result'))
            if result is None:
                counts['no result'] += 1
                continue
            if headers.get('SetUp') == '1' or 'FEN' in headers or headers.get('Variant', 'Standard') != 'Standard':
                counts['not standard start'] += 1
                continue
            white_elo, black_elo = elo(headers, 'WhiteElo'), elo(headers, 'BlackElo')
            if args.min_elo and (white_elo is None or black_elo is None or min(white_elo, black_elo) < args.min_elo):
                counts['below min elo'] += 1
                continue
            candidates = list(eligible_positions(game, args.skip_plies))
            if not candidates:
                counts['no eligible position'] += 1
                continue
            counts['used'] += 1
            counts['eligible positions'] += len(candidates)
            chooser = random.Random(f'{args.seed}:pick:{index}')
            for ply, fen in chooser.sample(candidates, min(args.per_game, len(candidates))):
                key = position_key(chess.Board(fen))
                if key in seen:
                    counts['duplicates'] += 1
                    continue
                seen.add(key)
                rows.append({'fen': fen, 'result': result, 'game': index, 'ply': ply,
                             'white': headers.get('White', '?'), 'black': headers.get('Black', '?'),
                             'whiteElo': white_elo, 'blackElo': black_elo})
            if args.progress and counts['picked'] % args.progress == 0:
                print(f"  {counts['games']:,} games read, {counts['picked']:,} picked, {len(rows):,} positions",
                      file=sys.stderr, flush=True)
    if args.limit and len(rows) > args.limit:
        rows.sort(key=lambda row: unit(args.seed, 'limit', row['game'], row['ply']))
        counts['cut by limit'] = len(rows) - args.limit
        rows = sorted(rows[:args.limit], key=lambda row: (row['game'], row['ply']))
    with open(args.out, 'w', encoding='utf-8', newline='\n') as out:
        for row in rows:
            out.write(json.dumps(row) + '\n')
    counts['written'] = len(rows)
    for name, value in counts.items():
        print(f'{name:>22}: {value:,}')


def mirror(source, target):
    """Writes the colour-flipped copy of a sample: ranks flipped, colours and result swapped."""
    with open(source, encoding='utf-8') as rows, open(target, 'w', encoding='utf-8', newline='\n') as out:
        for line in rows:
            row = json.loads(line)
            row['fen'] = chess.Board(row['fen']).mirror().fen()
            row['result'] = 1.0 - row['result']
            row['mirrored'] = True
            out.write(json.dumps(row) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('pgn', nargs='?')
    parser.add_argument('out', nargs='?')
    parser.add_argument('--fraction', type=float, help='share of games to pick, 0..1')
    parser.add_argument('--per-game', type=int, default=10)
    parser.add_argument('--seed', default='texel1')
    parser.add_argument('--skip-plies', type=int, default=24)
    parser.add_argument('--min-elo', type=int, help='keep games whose two engines are both rated at least this')
    parser.add_argument('--limit', type=int, help='keep at most this many positions (seeded choice)')
    parser.add_argument('--progress', type=int, default=500, help='report every N picked games')
    parser.add_argument('--mirror', nargs=2, metavar=('IN', 'OUT'))
    args = parser.parse_args()
    if args.mirror:
        mirror(*args.mirror)
    elif args.pgn and args.out and args.fraction:
        sample(args)
    else:
        parser.error('give PGN OUT --fraction F, or --mirror IN OUT')


if __name__ == '__main__':
    main()
