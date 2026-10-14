"""Prepares the two source trees that lazystat compiles (see README.md).

Exports src/ of two commits into OUT/new/src and OUT/second/src, then:
  - both ueval.cpp files get the lazy probe (lazy_probe.hpp): what evaluate() saw
    at the lazy checkpoint, recorded just before the first exit test;
  - the second tree's petir.hpp and ueval.cpp get the inline names that clash
    with the new tree renamed (NameOld), unless --same-material;
  - the new tree's main.cpp calls lazyStatNewGame() on ucinewgame and
    lazyStatGo() on go, for game-like counting.

Usage: prepare_trees.py OUT [--new REF] [--second REF | --second-src DIR] [--same-material]
  --new REF          the engine that searches (default HEAD)
  --second REF       the second eval, compared call by call (default 08c0f11,
                     the last commit before Texel tuning)
  --second-src DIR   a src directory instead of a commit, for example a tree with
                     a changed ueval.cpp; use --same-material when it has the new
                     tree's piece values and inline tables
"""
import argparse
import io
import re
import shutil
import subprocess
import tarfile
from pathlib import Path

# Inline names in petir.hpp that differ between 08c0f11 and the Texel-tuned tree.
CLASHING_NAMES = ['badBishopPenalty', 'pawnValue', 'knightValue', 'bishopValue', 'rookValue', 'queenValue']


def export_src(ref, target):
    archive = subprocess.run(['git', 'archive', '--format=tar', ref, 'src'], check=True, capture_output=True).stdout
    with tarfile.open(fileobj=io.BytesIO(archive)) as tar:
        tar.extractall(target.parent, filter='data')


def edit(path, change):
    text = path.read_bytes().decode()
    newline = '\r\n' if '\r\n' in text else '\n'
    path.write_bytes(change(text.replace('\r\n', '\n')).replace('\n', newline).encode())


def insert_probe(text):
    namespace = 'namespace petir_ueval {\n'
    check = re.search(r'^( *)if \(\(pas::add\(pas::add\(termScore, (\d+)\), lazyMargin\) < alpha\)\) \{\n', text, re.M)
    assert namespace in text and check, 'lazy checkpoint not found: the probe expects the 4ce0793 lazy block'
    indent, base = check.group(1), check.group(2)
    probe = (f'{indent}lazyProbe.reached = true; lazyProbe.partial = termScore; lazyProbe.margin = pas::add({base}, lazyMargin);\n'
             f'{indent}lazyProbe.phase = endgame ? 2 : (earlyEndgame ? 1 : 0); lazyProbe.trades = lazyMarginForTrades(position);\n')
    text = text[:check.start()] + probe + text[check.start():]
    return text.replace(namespace, '#include "lazy_probe.hpp"\n' + namespace + 'LazyProbe lazyProbe;\n', 1)


def rename_clashing(text):
    for name in CLASHING_NAMES:
        text = re.sub(r'\b' + name + r'\b', name + 'Old', text)
    return text


def add_game_hooks(text):
    new_game = 'else if(command=="ucinewgame") {finish();'
    go = 'else if(command=="go")uciGo(tokens,receivedAt);'
    assert text.count(new_game) == 1 and text.count(go) == 1, 'UCI loop not found in main.cpp'
    text = text.replace('namespace {\n', 'void lazyStatNewGame();\nvoid lazyStatGo();\nnamespace {\n', 1)
    text = text.replace(new_game, 'else if(command=="ucinewgame") {lazyStatNewGame();finish();')
    return text.replace(go, 'else if(command=="go") {lazyStatGo();uciGo(tokens,receivedAt);}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('out')
    parser.add_argument('--new', default='HEAD')
    parser.add_argument('--second', default='08c0f11')
    parser.add_argument('--second-src')
    parser.add_argument('--same-material', action='store_true')
    args = parser.parse_args()
    out = Path(args.out).resolve()
    new_src, second_src = out / 'new' / 'src', out / 'second' / 'src'
    for tree in (new_src, second_src):
        if tree.exists():
            shutil.rmtree(tree)
        tree.parent.mkdir(parents=True, exist_ok=True)
    export_src(args.new, new_src)
    if args.second_src:
        shutil.copytree(args.second_src, second_src)
    else:
        export_src(args.second, second_src)
    edit(new_src / 'ueval.cpp', insert_probe)
    edit(new_src / 'main.cpp', add_game_hooks)
    edit(second_src / 'ueval.cpp', insert_probe)
    if not args.same_material:
        for name in ('petir.hpp', 'ueval.cpp'):
            edit(second_src / name, rename_clashing)
    print(f'new {args.new} -> {new_src}')
    print(f'second {args.second_src or args.second} -> {second_src}' + (' (same material)' if args.same_material else ''))


if __name__ == '__main__':
    main()
