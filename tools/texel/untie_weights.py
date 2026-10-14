"""Turns fitted weights into the weights of the tapered engine.

FIT is written by texel_fit: index, name, value, minimum, maximum, or with
--untie index, name, middlegame value, endgame value, minimum, maximum.
TAPERED_LIST is `eval_trace --list` of the tapered tree, whose tapered
weights are name.mg and name.eg. OUT gets the tapered list's weights, in its
order and with its ranges: name.mg and name.eg take name's middlegame and
endgame values from FIT (its single value for both when FIT has one), every
other weight its value.

Usage: untie_weights.py FIT TAPERED_LIST OUT
"""
import argparse
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument('fit')
    parser.add_argument('tapered_list')
    parser.add_argument('out')
    args = parser.parse_args()

    fitted = {}
    with open(args.fit, encoding='utf-8') as rows:
        for row in rows:
            fields = row.rstrip('\r\n').split('\t')
            if len(fields) == 5:
                fitted[fields[1]] = (int(fields[2]), int(fields[2]))
            elif len(fields) == 6:
                fitted[fields[1]] = (int(fields[2]), int(fields[3]))
            else:
                sys.exit(f'{args.fit}: unexpected line {row.strip()}')
    used = set()
    lines = []
    with open(args.tapered_list, encoding='utf-8') as rows:
        for index, row in enumerate(rows):
            listed, name, _, low, high = row.rstrip('\r\n').split('\t')
            base, half = (name[:-3], name[-2:]) if name.endswith(('.mg', '.eg')) else (name, None)
            if base not in fitted:
                sys.exit(f'{base} is not in {args.fit}')
            middlegame, endgame = fitted[base]
            if half is None and middlegame != endgame:
                sys.exit(f'{base} has two values in {args.fit} but is not tapered')
            value = endgame if half == 'eg' else middlegame
            if not int(low) <= value <= int(high):
                sys.exit(f'{name} = {value} is outside {low}..{high}')
            used.add(base)
            lines.append(f'{listed}\t{name}\t{value}\t{low}\t{high}\n')
    if unused := set(fitted) - used:
        sys.exit(f'{len(unused)} weights of {args.fit} are not in {args.tapered_list}, such as {sorted(unused)[0]}')
    with open(args.out, 'w', encoding='utf-8', newline='') as out:
        out.writelines(lines)
    print(f'{len(lines)} weights written from {len(fitted)} fitted')


if __name__ == '__main__':
    main()
