"""Downloads standard EPD test suites and converts them to one JSON format.

Each suite is saved as SUITES_DIR/NAME.json with its positions and their
expected moves in UCI notation, so epd_test.py needs no chess library. The raw
download is kept in SUITES_DIR/raw/. SAN moves are converted with python-chess,
which is needed only here: it is imported normally, or from --python-chess DIR
(default: build/kpk-audit/python-chess-master).

The parser accepts the loose EPD found in the wild: move counters before the
operations, missing semicolons ("bm Rc8+ dm 26"), split check marks
("bm Qd1 +") and annotations ("Nf5!").

Usage: epd_suites.py fetch [NAME,..] [--dir DIR] [--python-chess DIR]
       epd_suites.py convert FILE.epd NAME [--dir DIR] [--python-chess DIR]
       epd_suites.py list [--dir DIR]
"""
import argparse
import json
import re
import shlex
import sys
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SUITES_DIR = ROOT / 'build' / 'suites'
ARASAN = 'https://raw.githubusercontent.com/jdart1/arasan-chess/master/tests/'
WHITTINGTON = 'https://raw.githubusercontent.com/ChrisWhittington/Chess-EPDs/master/'

# The Chess Programming Wiki's null-move test positions, where zugzwang makes
# a null move (or any "a move can only help" pruning) fail.
ZUGZWANG = '''\
8/8/p1p5/1p5p/1P5p/8/PPP2K1p/4R1rk w - - 0 1 bm Rf1; id "zugzwang.001";
1q1k4/2Rr4/8/2Q3K1/8/8/8/8 w - - 0 1 bm Kh6; id "zugzwang.002";
7k/5K2/5P1p/3p4/6P1/3p4/8/8 w - - 0 1 bm g5; id "zugzwang.003";
8/6B1/p5p1/Pp4kp/1P5r/5P1Q/4q1PK/8 w - - 0 32 bm Qxh4; id "zugzwang.004";
8/8/1p1r1k2/p1pPN1p1/P3KnP1/1P6/8/3R4 b - - 0 1 bm Nxd5; id "zugzwang.005";
'''

# name: (source URL or None for built-in text, description)
SUITES = {
    'wac': (ARASAN + 'wacnew.epd', 'Win at Chess, 300 tactical positions (corrected wacnew). Easy: a sanity check that pruning does not hide simple shots.'),
    'arasan': (ARASAN + 'arasan2026.epd', 'Arasan 2026 suite: hard tactical and positional positions from recent games.'),
    'ecm': (ARASAN + 'ecmgcp.epd', 'Encyclopedia of Chess Middlegames, GCP subset: middlegame tactics.'),
    'bt2630': (ARASAN + 'bt2630.epd', 'BT-2630: 30 hard positions.'),
    'eet': (ARASAN + 'eet.epd', 'Eigenmann Endgame Test: 100 endgame positions.'),
    'sbd': (WHITTINGTON + 'silent-but-deadly.epd', 'Silent but Deadly: quiet best moves, a test for quiet-move pruning and reductions.'),
    'bk': (WHITTINGTON + 'bratko-kopec.epd', 'Bratko-Kopec: 24 classic tactical and positional positions.'),
    'kaufman': (WHITTINGTON + 'kaufman.epd', 'Kaufman test: 25 positions.'),
    'lct2': (WHITTINGTON + 'lct2.epd', 'LCT II: 35 positional, tactical and endgame positions.'),
    'hard-mates': (WHITTINGTON + 'hard-mates-blass-uri.epd', 'Hard mates (Blass/Uri): long forced mates.'),
    'zugzwang': (None, 'Chess Programming Wiki null-move test positions: zugzwang, where null move and similar pruning fail.'),
}

OPCODES = {'bm', 'am', 'id', 'dm', 'ce', 'pv', 'sm', 'acd', 'acn', 'acs', 'hmvc', 'fmvn',
           'c0', 'c1', 'c2', 'c3', 'c4', 'c5', 'c6', 'c7', 'c8', 'c9', 'Bm', 'Am'}


def import_chess(path):
    try:
        import chess
    except ImportError:
        sys.path.insert(0, str(path))
        try:
            import chess
        except ImportError:
            sys.exit(f'python-chess not found; install it (pip install chess) or pass --python-chess DIR (tried {path})')
    return chess


def parse_operations(text):
    """Returns {opcode: [operand, ..]} from the text after the position fields."""
    try:
        words = shlex.split(text.replace(';', ' ; '), posix=True)
    except ValueError:  # an unbalanced quote
        words = text.replace(';', ' ; ').split()
    ops, current = {}, None
    for word in words:
        # c0-c9 are also pawn moves (bm c5): inside a move list they are moves.
        is_opcode = word in OPCODES and not (current in ('bm', 'am', 'pv', 'sm') and re.fullmatch(r'c\d', word))
        if word == ';':
            current = None
        elif current is None and not ops and word not in OPCODES and re.fullmatch(r'[KQRBNa-h][a-h1-8x=QRBN+#!?]+|O-O(-O)?[+#]?', word):
            # A move with its "bm" left out ("... 1 1 Be2 dm 21").
            current = 'bm'
            ops[current] = [word]
        elif current is None or is_opcode:
            current = word.lower()
            ops.setdefault(current, [])
        else:
            ops[current].append(word)
    return ops


def move_words(words):
    """Joins split check marks ("Qd1", "+") and drops annotations."""
    moves = []
    for word in words:
        if word in ('+', '#', '++') and moves:
            moves[-1] += '+'
        else:
            moves.extend(part for part in word.split(',') if part)
    return [re.sub(r'[!?]+$', '', move) for move in moves]


def convert(text, chess):
    """Returns (positions, problems) for EPD text."""
    positions, problems = [], []
    for number, line in enumerate(text.splitlines(), 1):
        line = line.replace('\x00', '').strip()
        if not line or line.startswith(('#', '%')):
            continue
        # A lost space between the en passant field and the move counters ("- -1 12").
        line = re.sub(r'^(\S+ [wb] \S+) -(\d+) ', r'\1 - \2 ', line)
        fields = line.split(None, 4)
        if len(fields) < 4:
            problems.append(f'line {number}: too few fields')
            continue
        rest = fields[4] if len(fields) > 4 else ''
        # Optional half-move clock and move number written as plain FEN fields.
        counters = re.match(r'\s*(\d+)\s+(\d+)\s*(.*)$', rest, re.S)
        halfmove, fullmove = ('0', '1')
        if counters:
            halfmove, fullmove, rest = counters.groups()
        ops = parse_operations(rest)
        halfmove = (ops.get('hmvc') or [halfmove])[0]
        fullmove = (ops.get('fmvn') or [fullmove])[0]
        fen = ' '.join(fields[:4] + [halfmove, fullmove])
        try:
            board = chess.Board(fen)
        except ValueError as error:
            problems.append(f'line {number}: bad position ({error})')
            continue
        expected = {}
        for opcode in ('bm', 'am'):
            for san in move_words(ops.get(opcode, [])):
                try:
                    move = board.parse_san(san)
                except ValueError:
                    try:
                        move = chess.Move.from_uci(san)
                        if move not in board.legal_moves:
                            raise ValueError
                    except ValueError:
                        problems.append(f'line {number}: cannot read {opcode} {san}')
                        continue
                expected.setdefault(opcode, []).append(move.uci())
        if not expected:
            problems.append(f'line {number}: no bm or am')
            continue
        position = {'id': ' '.join(ops.get('id', [])) or f'#{number}', 'fen': fen,
                    'bm': expected.get('bm', []), 'am': expected.get('am', [])}
        if ops.get('dm'):
            position['dm'] = int(ops['dm'][0])
        positions.append(position)
    return positions, problems


def save(name, source, description, text, directory, chess):
    positions, problems = convert(text, chess)
    (directory / 'raw').mkdir(parents=True, exist_ok=True)
    (directory / 'raw' / f'{name}.epd').write_text(text, encoding='utf-8')
    suite = {'name': name, 'source': source, 'description': description, 'positions': positions}
    (directory / f'{name}.json').write_text(json.dumps(suite, indent=1), encoding='utf-8')
    print(f'{name}: {len(positions)} positions' + (f', {len(problems)} skipped or partial' if problems else ''))
    for problem in problems:
        print('  ' + problem)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('command', choices=['fetch', 'convert', 'list'])
    parser.add_argument('args', nargs='*')
    parser.add_argument('--dir', type=Path, default=SUITES_DIR)
    parser.add_argument('--python-chess', type=Path, default=ROOT / 'build' / 'kpk-audit' / 'python-chess-master')
    options = parser.parse_args()
    if options.command == 'list':
        for name, (_, description) in SUITES.items():
            path = options.dir / f'{name}.json'
            count = len(json.loads(path.read_text(encoding='utf-8'))['positions']) if path.exists() else None
            print(f'{name:11} {"not fetched" if count is None else f"{count} positions":>14}  {description}')
        return
    chess = import_chess(options.python_chess)
    if options.command == 'convert':
        if len(options.args) != 2:
            parser.error('convert needs FILE.epd NAME')
        file, name = options.args
        save(name, str(Path(file).resolve()), 'converted from a local file',
             Path(file).read_text(encoding='utf-8', errors='replace'), options.dir, chess)
        return
    names = options.args[0].split(',') if options.args else list(SUITES)
    for name in names:
        if name not in SUITES:
            sys.exit(f'unknown suite {name}; known: {", ".join(SUITES)}')
        url, description = SUITES[name]
        if url is None:
            text, source = ZUGZWANG, 'https://www.chessprogramming.org/Null_Move_Test-Positions'
        else:
            with urllib.request.urlopen(url, timeout=60) as response:
                text = response.read().decode('utf-8', errors='replace')
            source = url
        save(name, source, description, text, options.dir, chess)


if __name__ == '__main__':
    main()
