"""Writes fitted evaluation weights into Petir's source (src/petir.hpp).

Takes a weights file written like `eval_trace --list` (a fit's output) and sets
each weight's value in the declarations of petir.hpp, in every stored copy
that the weight ties together:
  - pawn, knight, bishop and queen SquareBonus[sq], kingSquarePenalty[sq],
    kingEndgameBothWingsBonus[sq] and the outpost tables: white...[sq] and
    black...[sq ^ 56]
  - kingEndgameKingsideBonus[sq]: also the queenside tables (sq ^ 7), and
    Black's copies of both
  - rookFileBonus[f]: rookSquareBonus on every rank of file f
  - passedPawnRankBonus[r], kingSupportedPasserBonus[r]: white...[r] and
    black...[7 - r]
  - other name[i]: the table entry; name: the scalar (the piece values are
    petir_header's, which search uses too)
  - a name ending in .mg or .eg: the middlegame or endgame value of the
    tapered entries above, written taper(middlegame, endgame)
Entries whose value does not change are left as they are written, so writing
the engine's own weights leaves the file unchanged.

Check the result by building eval_trace from the tree and comparing its
`--list` with the weights file: eval_trace also checks that the tied copies
agree.

Usage: write_weights.py WEIGHTS HEADER
"""
import argparse
import re
import sys

ARRAY = re.compile(r'^(inline pas::Array<std::(\w+), ([^,]+), ([^>]+)> (\w+) = \{)(.*)(\};)$')
C_ARRAY = re.compile(r'^(PETIR_EVAL_WEIGHT int (\w+)\[\] = \{)(.*)(\};)$')
SCALAR = re.compile(r'^((?:PETIR_EVAL_WEIGHT|inline constexpr) auto (\w+) = )([^;]+)(;)$')
NEGATED = re.compile(r'pas::negate\((\d+)\)')
TAPERED = re.compile(r'taper\((-?\d+), (-?\d+)\)')
ENTRY = re.compile(r'\s*taper\([^)]*\)|[^,]+')
RANGES = {'int8_t': (-128, 127), 'uint8_t': (0, 255), 'int16_t': (-32768, 32767), 'uint16_t': (0, 65535),
          'int32_t': (-2 ** 31, 2 ** 31 - 1)}


def number(text):
    text = text.strip()
    negated = NEGATED.fullmatch(text)
    return -int(negated.group(1)) if negated else int(text)


def array_text(value):
    return f'pas::negate({-value})' if value < 0 else str(value)


class Header:
    """The declarations of petir.hpp that hold weights, by name."""

    def __init__(self, path):
        with open(path, encoding='utf-8', newline='') as source:
            self.lines = source.read().split('\n')
        self.tables, self.scalars = {}, {}
        for line_number, line in enumerate(self.lines):
            body = line.rstrip('\r')
            if match := ARRAY.match(body):
                self.add_table(match.group(5), line_number, match.group(1), match.group(6), match.group(7),
                               number(match.group(3)), match.group(2))
            elif match := C_ARRAY.match(body):
                self.add_table(match.group(2), line_number, match.group(1), match.group(3), match.group(4), 0, 'int32_t')
            elif match := SCALAR.match(body):
                if match.group(2) in self.scalars:
                    sys.exit(f'{match.group(2)} is declared twice')
                self.scalars[match.group(2)] = {'line': line_number, 'head': match.group(1), 'value': match.group(3),
                                                'tail': match.group(4)}

    def add_table(self, name, line, head, body, tail, lowest, element):
        if name in self.tables:
            sys.exit(f'{name} is declared twice')
        self.tables[name] = {'line': line, 'head': head, 'tail': tail, 'lowest': lowest,
                             'entries': [entry.strip() for entry in ENTRY.findall(body)], 'range': RANGES[element]}

    def set(self, table, value, half=None):
        entry = self.tables.get(table[0])
        if entry is None:
            sys.exit(f'no table {table[0]} in the header')
        position = table[1] - entry['lowest']
        if not 0 <= position < len(entry['entries']):
            sys.exit(f'{table[0]}[{table[1]}] is outside the table')
        if half:
            tapered = TAPERED.fullmatch(entry['entries'][position])
            if not tapered:
                sys.exit(f'{table[0]}[{table[1]}] is not tapered')
            middlegame, endgame = int(tapered.group(1)), int(tapered.group(2))
            if not -32768 <= value <= 32767:
                sys.exit(f'{table[0]}[{table[1]}].{half} = {value} does not fit 16 bits')
            text = f'taper({value}, {endgame})' if half == 'mg' else f'taper({middlegame}, {value})'
            if text != entry['entries'][position]:
                entry['entries'][position] = text
                entry['changed'] = True
            return
        low, high = entry['range']
        if not low <= value <= high:
            sys.exit(f'{table[0]}[{table[1]}] = {value} does not fit the table')
        if number(entry['entries'][position]) != value:
            entry['entries'][position] = array_text(value)
            entry['changed'] = True

    def set_scalar(self, name, value):
        entry = self.scalars.get(name)
        if entry is None:
            sys.exit(f'no scalar {name} in the header')
        if number(entry['value']) != value:
            entry['value'] = str(value)
            entry['changed'] = True

    def write(self, path):
        changed = 0
        for entry in self.tables.values():
            if entry.get('changed'):
                ending = '\r' if self.lines[entry['line']].endswith('\r') else ''
                self.lines[entry['line']] = entry['head'] + ', '.join(entry['entries']) + entry['tail'] + ending
                changed += 1
        for entry in self.scalars.values():
            if entry.get('changed'):
                ending = '\r' if self.lines[entry['line']].endswith('\r') else ''
                self.lines[entry['line']] = entry['head'] + entry['value'] + entry['tail'] + ending
                changed += 1
        with open(path, 'w', encoding='utf-8', newline='') as source:
            source.write('\n'.join(self.lines))
        return changed


def square(text):
    return (int(text[1]) - 1) * 8 + ord(text[0]) - ord('a')


def capitalised(name):
    return name[0].upper() + name[1:]


def stored_entries(name):
    """The (table, index) entries a weight sets, or None for a scalar."""
    match = re.fullmatch(r'(\w+)\[(\w+)\]', name)
    if not match:
        return None
    base, key = match.groups()
    if base == 'kingEndgameKingsideBonus':
        at = square(key)
        return [('whiteKingEndgameKingsideBonus', at), ('blackKingEndgameKingsideBonus', at ^ 56),
                ('whiteKingEndgameQueensideBonus', at ^ 7), ('blackKingEndgameQueensideBonus', at ^ 63)]
    if base == 'rookFileBonus':
        file = ord(key) - ord('a')
        return [('rookSquareBonus', rank * 8 + file) for rank in range(8)]
    if base in ('passedPawnRankBonus', 'kingSupportedPasserBonus'):
        rank = int(key)
        return [('white' + capitalised(base), rank), ('black' + capitalised(base), 7 - rank)]
    if re.fullmatch(r'[a-h][1-8]', key):
        at = square(key)
        return [('white' + capitalised(base), at), ('black' + capitalised(base), at ^ 56)]
    return [(base, int(key))]


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('weights')
    parser.add_argument('header')
    args = parser.parse_args()

    header = Header(args.header)
    count = 0
    with open(args.weights, encoding='utf-8') as weights:
        for line in weights:
            fields = line.split('\t')
            if len(fields) != 5:
                sys.exit(f'{args.weights}: expected index, name, value, minimum, maximum: {line.strip()}')
            name, value = fields[1], int(fields[2])
            half = None
            if name.endswith(('.mg', '.eg')):
                name, half = name[:-3], name[-2:]
            entries = stored_entries(name)
            if entries is None:
                if half:
                    sys.exit(f'{fields[1]}: a scalar cannot be tapered')
                header.set_scalar(name, value)
            else:
                for entry in entries:
                    header.set(entry, value, half)
            count += 1
    changed = header.write(args.header)
    print(f'{count} weights written; {changed} declarations changed')


if __name__ == '__main__':
    main()
