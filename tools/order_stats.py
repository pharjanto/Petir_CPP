"""Move-ordering statistics from a PETIR_ORDER_STATS build.

Searches each suite position from a new game to a fixed depth, reads the
engine's "orderstats" counters after every search, and prints a Markdown
summary. The raw per-position counters can be saved as JSON for comparing
two builds. With --mirror, each position is also searched colour-mirrored.
--positions replaces the built-in suite with a JSON list of [name, group,
position command, depth], and --shard K/N searches every Nth of its positions
so several drivers can share one suite. --warmup N first searches the position
N plies earlier in the same game and then the suite position without a new
game, as in a real game; only the second search is counted.

Usage: order_stats.py ENGINE_EXE [--depth-offset N] [--groups G,..] [--mirror]
                      [--positions FILE] [--shard K/N] [--warmup N]
                      [--hash MB] [--json OUT] [--report OUT]
"""
import argparse
import json
import queue
import subprocess
import threading
import time
from pathlib import Path

# (name, group, position command, depth); the search regression suite.
SUITE = [
    ('start', 'middlegame', 'startpos', 11),
    ('ruy-lopez', 'middlegame', 'startpos moves e2e4 e7e5 g1f3 b8c6 f1b5 a7a6 b5a4 g8f6 e1g1 f8e7 f1e1 b7b5 a4b3 d7d6', 10),
    ('nimzo', 'middlegame', 'startpos moves d2d4 g8f6 c2c4 e7e6 b1c3 f8b4', 10),
    ('najdorf', 'middlegame', 'startpos moves e2e4 c7c5 g1f3 d7d6 d2d4 c5d4 f3d4 g8f6 b1c3 a7a6', 10),
    ('kiwipete', 'middlegame', 'fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1', 9),
    ('qgd-middlegame', 'middlegame', 'fen r1bq1rk1/pp2bppp/2n2n2/3p4/3P4/2NBPN2/PP3PPP/R2Q1RK1 w - - 0 10', 10),
    ('hedgehog', 'middlegame', 'fen 2rq1rk1/pb1nbppp/1p2pn2/2pp4/2PP4/1PN1PN2/PB2BPPP/2RQ1RK1 w - - 0 11', 10),
    ('middlegame', 'middlegame', 'fen r3r1k1/pp1bqppp/2n2n2/2pp4/3P4/2PB1N2/PPQN1PPP/R3R1K1 w - - 0 1', 10),
    ('italian', 'middlegame', 'fen r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10', 10),
    ('perft5-promo', 'middlegame', 'fen rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8', 9),
    ('perft4', 'middlegame', 'fen r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1', 8),
    ('perft4-mirror', 'middlegame', 'fen r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1', 8),
    ('wac001', 'tactical', 'fen 2rr3k/pp3pp1/1nnqbN1p/3pN3/2pP4/2P3Q1/PP3PPP/R4RK1 w - - 0 1', 10),
    ('wac002', 'tactical', 'fen 8/7p/5k2/5p2/p1p2P2/Pr1pPK2/1P1R3P/8 b - - 0 1', 12),
    ('wac003', 'tactical', 'fen 5rk1/1ppb3p/p1pb4/6q1/3P1p1r/2P1R2P/PP1BQ1P1/5RKN w - - 0 1', 10),
    ('wac004', 'tactical', 'fen r1bq2rk/pp3pbp/2p1p1pQ/7P/3P4/2PB1N2/PP3PP1/R3K2R w KQ - 0 1', 10),
    ('wac005', 'tactical', 'fen 5k2/6pp/p1qN4/1p1p4/3P4/2PKP2Q/PP3r2/3R4 b - - 0 1', 10),
    ('mate-in-1', 'tactical', 'fen r1bqkb1r/pppp1ppp/2n2n2/4p2Q/2B1P3/8/PPPP1PPP/RNB1K1NR w KQkq - 4 4', 8),
    ('pgn-mate-defense', 'tactical', 'fen 7k/1q5p/5r1P/4Np2/R2p1P2/2bP2PK/1p6/1Q6 w - - 4 68', 10),
    ('in-check', 'endgame', 'fen 4k3/8/8/8/8/8/4q3/4K3 w - - 0 1', 12),
    ('ep-evasion', 'endgame', 'fen 4r2k/8/8/3pP3/4K3/8/8/8 w - d6 0 1', 12),
    ('ep-available', 'middlegame', 'fen rnbqkbnr/ppp1p1pp/8/3pPp2/8/8/PPPP1PPP/RNBQKBNR w KQkq f6 0 3', 10),
    ('castling-white', 'endgame', 'fen r3k2r/8/8/8/8/8/8/R3K2R w KQkq - 0 1', 10),
    ('castling-black', 'endgame', 'fen r3k2r/8/8/8/8/8/8/R3K2R b KQkq - 0 1', 10),
    ('promotion-race', 'endgame', 'fen 8/P7/8/8/8/8/p7/k6K w - - 0 1', 14),
    ('underpromotion', 'endgame', 'fen 1rr4k/P7/8/8/8/8/8/K4B2 w - - 0 1', 12),
    ('many-queens', 'special','fen kn6/pp1Q4/1Q4Q1/4Q3/Q5Q1/2Q5/1Q3Q2/K2Q4 w - - 0 1', 5),
    ('rook-ending-ep', 'endgame', 'fen 8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1', 14),
    ('rook-endgame', 'endgame', 'fen 8/5pk1/6p1/4p2p/4P2P/5PP1/5K2/2R4r w - - 0 1', 12),
    ('pawn-endgame', 'endgame', 'fen 8/pp6/2p1k3/3p1pp1/3P1PP1/2P1K3/PP6/8 w - - 0 1', 13),
    ('fine70', 'endgame', 'fen 8/k7/3p4/p2P1p2/P2P1P2/8/8/K7 w - - 0 1', 16),
    ('kpk-win', 'endgame', 'fen 8/8/8/4k3/8/8/4P3/4K3 w - - 0 1', 14),
    ('kpk-opposition', 'endgame', 'fen 8/4k3/8/4K3/4P3/8/8/8 b - - 0 1', 14),
    ('krk', 'endgame', 'fen 8/8/8/4k3/8/8/8/R3K3 w - - 0 1', 12),
    ('krk-halfmove95', 'endgame', 'fen 8/8/4k3/8/2R5/8/4K3/8 w - - 95 120', 12),
    ('krk-halfmove99', 'endgame', 'fen 7k/8/8/8/8/8/8/KR6 w - - 99 1', 8),
    ('kqk', 'endgame', 'fen 2k5/8/8/8/8/8/8/2KQ4 w - - 0 1', 10),
    ('stalemated-black', 'endgame', 'fen 7k/5Q2/6K1/8/8/8/8/8 b - - 0 1', 6),
    ('stalemated-white', 'endgame', 'fen 8/8/8/8/8/6k1/6p1/6K1 w - - 0 1', 6),
    ('back-rank', 'endgame', 'fen 6k1/5ppp/8/8/8/8/5PPP/3R2K1 w - - 0 1', 12),
    ('pawn-breakthrough', 'endgame', 'fen 8/8/1p6/1P1k4/3p4/3K4/8/8 w - - 0 1', 16),
    ('rook-vs-pawns', 'endgame', 'fen 6k1/5pp1/7p/8/8/7P/5PP1/3R2K1 w - - 0 1', 12),
    ('blocked-halfmove99', 'endgame', 'fen 8/5k2/3p4/1p1Pp2p/pP2Pp1P/P4P1K/8/8 b - - 99 50', 14),
    ('queen-trade', 'middlegame', 'fen 3r1rk1/p4ppp/1q2p3/2n5/8/1P3NP1/P3QPBP/2R2RK1 b - - 0 20', 10),
    ('two-knights', 'middlegame', 'fen r1bqkbnr/pppp1ppp/2n5/4p3/4P3/5N2/PPPP1PPP/RNBQKB1R w KQkq - 2 3', 10),
    ('repetition-knights', 'middlegame', 'startpos moves g1f3 g8f6 f3g1 f6g8 g1f3 g8f6 f3g1 f6g8 g1f3', 10),
    ('repetition-white', 'middlegame', 'startpos moves g1f3 g8f6 f3g1 f6g8 g1f3 g8f6 f3g1 f6g8', 10),
    ('repetition-ending', 'endgame', 'fen 8/8/4k3/8/8/2K5/3Q4/8 w - - 0 1 moves d2d4 e6e7 d4d2 e7e6 d2d4 e6e7 d4d2', 10),
]
# 'special' holds the nine-queen position, whose huge forced-mate tree would
# otherwise dominate any group it joined.
GROUPS = ['middlegame', 'tactical', 'endgame', 'special']

# Counter layouts; these follow src/order_stats.hpp.
PHASES = ['hash move', 'good capture', 'killer 1 (this ply)', 'killer 2 (this ply)',
          'killer 3 (ply-2)', 'killer 4 (ply-2)', 'quiet', 'bad capture',
          'evasion: hash move', 'evasion: other']
DEPTHS = ['0-1', '2', '3', '4-5', '6-8', '9+']
DEEP = range(3, len(DEPTHS))  # four or more remaining plies
INDEXES = ['1', '2', '3', '4', '5', '6-10', '11-20', '21+']
RANKS = ['1', '2', '3', '4-5', '6-10', '11-20', '21+']
TYPES = ['PV', 'cut', 'all']


class Engine:
    def __init__(self, exe, hash_mb):
        self.proc = subprocess.Popen([str(exe)], cwd=Path(exe).parent, stdin=subprocess.PIPE,
                                     stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                     text=True, bufsize=1)
        self.lines = queue.Queue()
        threading.Thread(target=self._reader, daemon=True).start()
        self.send('uci'); self.until(lambda line: line == 'uciok')
        self.send('setoption name OwnBook value false')
        self.send(f'setoption name Hash value {hash_mb}')

    def _reader(self):
        for line in self.proc.stdout:
            self.lines.put(line.rstrip())

    def send(self, command):
        self.proc.stdin.write(command + '\n'); self.proc.stdin.flush()

    def until(self, wanted, timeout=900):
        """Returns the first line accepted by wanted, and the last 'info depth' line before it."""
        deadline = time.monotonic() + timeout
        last_info = None
        while True:
            line = self.lines.get(timeout=max(0.001, deadline - time.monotonic()))
            if line.startswith(('Error', 'info string')):
                raise RuntimeError('engine reported: ' + line)
            if line.startswith('info depth'):
                last_info = line
            if wanted(line):
                return line, last_info

    def fen(self, position):
        self.send('position ' + position)
        self.send('d')
        return self.until(lambda line: line.count('/') == 7)[0]

    def search(self, position, depth, new_game=True):
        if new_game:
            self.send('ucinewgame')
        self.send('position ' + position)
        self.send('isready'); self.until(lambda line: line == 'readyok')
        self.send(f'go depth {depth}')
        best, info = self.until(lambda line: line.startswith('bestmove'))
        self.send('orderstats')
        line, _ = self.until(lambda line: line.startswith('orderstats '))
        words = info.split() if info else []
        nodes = int(words[words.index('nodes') + 1]) if 'nodes' in words else 0
        score = ' '.join(words[words.index('score') + 1:words.index('score') + 3]) if 'score' in words else ''
        return {'bestmove': best.split()[1], 'nodes': nodes, 'score': score,
                'stats': json.loads(line[len('orderstats '):])}

    def close(self):
        if self.proc.poll() is None:
            self.send('quit')
            self.proc.wait(timeout=5)


def mirror_fen(fen):
    board, side, castling, ep, *counters = fen.split()
    board = '/'.join(reversed(board.split('/'))).swapcase()
    castling = ''.join(c for c in 'KQkq' if c.swapcase() in castling) or '-'
    if ep != '-':
        ep = ep[0] + ('3' if ep[1] == '6' else '6')
    return ' '.join([board, 'b' if side == 'w' else 'w', castling, ep] + counters)


def mirror_move(move):
    if len(move) < 4 or move == '0000':
        return move
    return move[0] + str(9 - int(move[1])) + move[2] + str(9 - int(move[3])) + move[4:]


def combine(a, b):
    if isinstance(a, dict):
        return {k: combine(a[k], b[k]) for k in a}
    if isinstance(a, list):
        return [combine(x, y) for x, y in zip(a, b)]
    return a + b


def total(results):
    out = None
    for r in results:
        out = r['stats'] if out is None else combine(out, r['stats'])
    return out


def pct(part, whole):
    return f'{100 * part / whole:.1f}%' if whole else '-'


def ratio(part, whole, digits=2):
    return f'{part / whole:.{digits}f}' if whole else '-'


def table(header, rows):
    out = ['| ' + ' | '.join(header) + ' |', '|' + '|'.join('---' for _ in header) + '|']
    out += ['| ' + ' | '.join(str(x) for x in row) + ' |' for row in rows]
    return '\n'.join(out)


def cuts(c):
    return sum(map(sum, c['cut_index']))


def first_cuts(c):
    return sum(row[0] for row in c['cut_index'])


def summary_row(label, results):
    c = total(results)
    n = cuts(c)
    late = sum(sum(row[5:]) for row in c['cut_index'])
    return [label, len(results), f'{sum(r["nodes"] for r in results):,}', f'{n:,}',
            pct(first_cuts(c), n), ratio(sum(c['cut_move_sum']), n), pct(late, n),
            pct(c['q_cut_index'][0], sum(c['q_cut_index']))]


def overview(results):
    rows = [summary_row(g, [r for r in results if r['group'] == g])
            for g in dict.fromkeys(r['group'] for r in results)]
    rows.append(summary_row('**all**', results))
    return ['## Overview', '',
            'Cut node: a main-search move loop that failed high. First-move rate: share of cut nodes '
            'whose first move caused the cutoff. Last column: the same rate in quiescence nodes not in check.', '',
            table(['Group', 'Positions', 'Nodes', 'Cut nodes', 'First-move rate', 'Avg cutoff move',
                   'Cutoff at move 6+', 'Quiescence first-move'], rows), '']


def details(c):
    out = ['## By remaining depth', '',
           'Wasted share: nodes spent on moves tried before the cutoff move, as a share of all nodes '
           'under that cut node. A bucket\'s nodes include deeper-nested cut nodes, so read each bucket alone.', '']
    rows = []
    for d, label in enumerate(DEPTHS):
        n = sum(c['cut_index'][d])
        if n:
            rows.append([label, f'{n:,}', pct(c['cut_index'][d][0], n), ratio(c['cut_move_sum'][d], n),
                         pct(sum(c['cut_index'][d][5:]), n), pct(c['cut_wasted'][d], c['cut_nodes'][d])])
    out += [table(['Remaining plies', 'Cut nodes', 'First-move rate', 'Avg cutoff move',
                   'Cutoff at move 6+', 'Wasted share'], rows), '',
            'Cutoff move number:', '',
            table(['Remaining plies'] + INDEXES,
                  [[label] + [pct(x, sum(c['cut_index'][d])) for x in c['cut_index'][d]]
                   for d, label in enumerate(DEPTHS) if sum(c['cut_index'][d])]), '']

    out += ['## Where cutoffs come from', '',
            'Tried: moves searched from that phase in any node. Hit rate: cutoffs / tried. '
            'Wasted (4+ plies): share of the nodes spent before the cutoff move, by the phase that '
            'finally cut off.', '']
    n_all, n_deep = cuts(c), sum(sum(c['cut_phase'][d]) for d in DEEP)
    wasted_deep = sum(sum(c['phase_wasted'][d]) for d in DEEP)
    rows = []
    for p, label in enumerate(PHASES):
        tried = sum(c['tried_phase'][d][p] for d in range(len(DEPTHS)))
        hit = sum(c['cut_phase'][d][p] for d in range(len(DEPTHS)))
        hit_deep = sum(c['cut_phase'][d][p] for d in DEEP)
        tried_deep = sum(c['tried_phase'][d][p] for d in DEEP)
        rows.append([label, f'{tried:,}', f'{hit:,}', pct(hit, tried), pct(hit, n_all),
                     pct(hit_deep, tried_deep), pct(hit_deep, n_deep),
                     pct(sum(c['phase_wasted'][d][p] for d in DEEP), wasted_deep)])
    out += [table(['Phase', 'Tried', 'Cutoffs', 'Hit rate', 'Share of cutoffs',
                   'Hit rate (4+ plies)', 'Share (4+ plies)', 'Wasted (4+ plies)'], rows), '']

    quiet = [sum(row) for row in c['quiet_rank']]
    tried = [sum(row) for row in c['quiet_tried_rank']]
    nq = sum(quiet)
    both_cut = [w + b for w, b in zip(*c['quiet_rank'])]
    both_tried = [w + b for w, b in zip(*c['quiet_tried_rank'])]
    out += ['## Quiet moves', '',
            f'Quiet cutoffs: {nq:,} ({pct(nq, n_all)} of all cutoffs). '
            f'Average quiet list length at those nodes: {ratio(c["quiet_list_moves"], nq, 1)}. '
            f'Cutoff move had no history (score 0): {pct(c["quiet_zero_score"], nq)}. '
            'Only the first 20 quiet moves are sorted.', '',
            'Hit rate by position in the quiet list (cutoffs / moves tried at that position):', '',
            table(['Side to move', 'Tried'] + ['Rank ' + r for r in RANKS],
                  [[side, f'{tried[w]:,}'] + [pct(x, t) for x, t in zip(c['quiet_rank'][w], c['quiet_tried_rank'][w])]
                   for w, side in ((1, 'white'), (0, 'black'))]
                  + [['both', f'{sum(tried):,}'] + [pct(x, t) for x, t in zip(both_cut, both_tried)]]), '',
            'Where quiet cutoffs happen in the list:', '',
            table(['Side to move', 'Quiet cutoffs'] + ['Rank ' + r for r in RANKS],
                  [[side, f'{quiet[w]:,}'] + [pct(x, quiet[w]) for x in c['quiet_rank'][w]]
                   for w, side in ((1, 'white'), (0, 'black'))]), '']

    out += ['## Side to move and check', '',
            'Side-to-move rows are confounded by which side owns the cut nodes; see the mirror section for a fair colour test.', '',
            table(['Nodes', 'Cut nodes', 'First-move rate'],
                  [[label, f'{sum(c[key][i]):,}', pct(c[key][i][1], sum(c[key][i]))]
                   for label, key, i in (('white to move', 'cut_by_side', 1), ('black to move', 'cut_by_side', 0),
                                         ('not in check', 'cut_by_check', 0), ('in check', 'cut_by_check', 1))]), '']

    rows = []
    for label, buckets in (('all depths', range(len(DEPTHS))), ('4+ plies', DEEP)):
        for h, avail in ((1, 'hash move'), (0, 'no hash move')):
            n = sum(sum(c['cut_by_hash'][d][h]) for d in buckets)
            all_n = sum(sum(sum(c['cut_by_hash'][d][x]) for x in (0, 1)) for d in buckets)
            first = sum(c['cut_by_hash'][d][h][1] for d in buckets)
            wasted = sum(c['hash_wasted'][d][h] for d in buckets)
            nodes = sum(c['hash_nodes'][d][h] for d in buckets)
            rows.append([label, avail, f'{n:,}', pct(n, all_n), pct(first, n), pct(wasted, nodes)])
    out += ['## Hash move', '',
            table(['Remaining plies', 'Cut nodes with', 'Count', 'Share', 'First-move rate', 'Wasted share'], rows), '',
            'Expected-PV move loops without a hash move:', '',
            table(['Remaining plies', 'PV loops', 'Without hash move'],
                  [[label, f'{sum(c["pv_hash"][d]):,}', pct(c['pv_hash'][d][0], sum(c['pv_hash'][d]))]
                   for d, label in enumerate(DEPTHS) if sum(c['pv_hash'][d])]), '']

    o = c['outcome']
    out += ['## Node types and re-searches', '',
            table(['Expected type', 'Loops', 'Actual PV', 'Actual cut', 'Actual all'],
                  [[t, f'{sum(o[i]):,}'] + [pct(x, sum(o[i])) for x in o[i]] for i, t in enumerate(TYPES)]), '',
            f'- Reduced moves searched: {c["reduced"]:,}. Re-searched after failing high: '
            f'{c["lmr_research"]:,} ({pct(c["lmr_research"], c["reduced"])}); the full-depth re-search '
            f'still failed high in {pct(c["lmr_held"], c["lmr_research"])}.',
            f'- PVS re-searches (null window beat alpha in a PV node): {c["pvs_research"]:,}; '
            f'confirmed by the full window: {pct(c["pvs_raised"], c["pvs_research"])}.',
            '- Best move position in nodes that ended as PV: ' + ', '.join(
                f'{label}: {pct(x, sum(c["pv_best_index"]))}' for label, x in zip(INDEXES, c['pv_best_index'])), '']

    out += ['## Root', '',
            f'- Root searches (iterations and aspiration re-searches): {c["root_calls"]:,}; '
            f'no move raised alpha: {c["root_fail_low"]:,}.',
            '- Final best move position: ' + ', '.join(
                f'{label}: {x}' for label, x in zip(INDEXES, c['root_best_index']) if x),
            f'- New best moves found after the first move: {c["root_best_changes"]:,}; '
            f'root re-searches: {c["root_research"]:,}, confirmed {pct(c["root_raised"], c["root_research"])}.',
            f'- Root nodes spent on the first move: {pct(c["root_nodes_first"], c["root_nodes"])}.', '']
    return out


def per_position(results):
    return ['## Per position', '',
            table(['Position', 'Group', 'Depth', 'Best', 'Score', 'Nodes', 'Cut nodes', 'First-move rate',
                   'Avg cutoff move', 'Quiet share of cutoffs'],
                  [[r['name'], r['group'], r['depth'], r['bestmove'], r['score'], f'{r["nodes"]:,}',
                    f'{cuts(r["stats"]):,}', pct(first_cuts(r['stats']), cuts(r['stats'])),
                    ratio(sum(r['stats']['cut_move_sum']), cuts(r['stats'])),
                    pct(sum(row[6] for row in r['stats']['cut_phase']), cuts(r['stats']))] for r in results]), '']


def mirror_section(results):
    pairs = [(r, m) for r in results if r['variant'] == 'original'
             for m in results if m['variant'] == 'mirrored' and m['name'] == r['name']]
    same = sum(1 for r, m in pairs if r['nodes'] == m['nodes'])
    same_result = sum(1 for r, m in pairs if r['score'] == m['score'] and r['bestmove'] == mirror_move(m['bestmove']))
    a, b = sum(r['nodes'] for r, _ in pairs), sum(m['nodes'] for _, m in pairs)
    rows = [[r['name'], r['depth'], f'{r["nodes"]:,}', f'{m["nodes"]:,}', pct(m['nodes'] - r['nodes'], r['nodes']),
             r['score'], m['score'], r['bestmove'], mirror_move(m['bestmove'])] for r, m in pairs]
    return ['## Colour mirror', '',
            'Each position searched from its FEN and again with colours swapped (board flipped, side to move '
            'swapped). A colour-symmetric engine would search identical trees. Mirrored best moves are flipped '
            'back for comparison.', '',
            f'- Pairs: {len(pairs)}; identical node counts: {same}; same score and best move: {same_result}.',
            f'- Total nodes: original {a:,}, mirrored {b:,} ({pct(b - a, a)}).', '',
            table(['Position', 'Depth', 'Nodes', 'Mirrored nodes', 'Difference', 'Score', 'Mirrored score',
                   'Best', 'Mirrored best'], rows), '']


def report(results, depth_offset, detail_groups):
    main = [r for r in results if r['variant'] == 'original']
    detail = [r for r in main if r['group'] in detail_groups]
    out = [f'# Move-ordering statistics (suite depths {depth_offset:+d})', '']
    out += overview(main)
    if detail:
        out += [f'# Details: {", ".join(detail_groups)} positions', '']
        out += details(total(detail))
    if any(r['variant'] == 'mirrored' for r in results):
        out += mirror_section(results)
    out += per_position(main)
    return '\n'.join(out)


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument('engine', type=Path)
    parser.add_argument('--depth-offset', type=int, default=0)
    parser.add_argument('--hash', type=int, default=16)
    parser.add_argument('--groups', help='groups to search (default: all)')
    parser.add_argument('--detail-groups', default='middlegame,tactical', help='groups summarised in detail')
    parser.add_argument('--mirror', action='store_true',
                        help='search from FEN and again colour-mirrored (skips positions that need move history)')
    parser.add_argument('--positions', type=Path, help='JSON suite to search instead of the built-in one')
    parser.add_argument('--shard', default='1/1', help='K/N: search positions K, K+N, K+2N, ...')
    parser.add_argument('--warmup', type=int, default=0,
                        help='first search the position this many plies earlier (startpos moves positions only)')
    parser.add_argument('--json', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    suite = json.loads(args.positions.read_text(encoding='utf-8')) if args.positions else SUITE
    shard, shards = map(int, args.shard.split('/'))
    suite = suite[shard - 1::shards]
    groups = args.groups.split(',') if args.groups else [group for _, group, _, _ in suite]
    engine = Engine(args.engine, args.hash)
    results = []
    try:
        for name, group, position, depth in suite:
            if group not in groups or (args.mirror and name.startswith('repetition')):
                continue
            depth = max(1, depth + args.depth_offset)
            variants = [('original', position)]
            if args.mirror:
                fen = engine.fen(position)
                variants = [('original', 'fen ' + fen), ('mirrored', 'fen ' + mirror_fen(fen))]
            for variant, command in variants:
                started = time.monotonic()
                moves = command.split()
                if args.warmup and command.startswith('startpos moves') and len(moves) - 2 >= args.warmup:
                    engine.search(' '.join(moves[:len(moves) - args.warmup]), depth)
                    result = engine.search(command, depth, new_game=False)
                else:
                    result = engine.search(command, depth)
                result.update(name=name, group=group, depth=depth, variant=variant,
                              seconds=round(time.monotonic() - started, 2))
                results.append(result)
                print(f'{name} {variant} depth={depth} nodes={result["nodes"]} best={result["bestmove"]}', flush=True)
    finally:
        engine.close()
    if args.json:
        args.json.write_text(json.dumps(results, indent=1), encoding='utf-8')
    text = report(results, args.depth_offset, args.detail_groups.split(','))
    if args.report:
        args.report.write_text(text + '\n', encoding='utf-8')
    print(text)


if __name__ == '__main__':
    main()
