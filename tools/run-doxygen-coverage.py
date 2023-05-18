#!/usr/bin/env python

import argparse
import os
import sys
import xml.etree.ElementTree as XMLTree

stats = {}
kinds = {}


def is_excluded(kind):
    return kind == 'dir' or not kinds[kind]


def parse(tree):
    definitions = {}

    kind = tree.get('kind')
    if is_excluded(kind):
        return None, None

    documented = False
    if tree.findall('./detaileddescription/'):
        documented = True

    name = tree.find('./name')
    sourcefile = tree.find('./location').get('file')
    definition = tree.find('./definition')
    argsstring = tree.find('./argsstring')
    compoundname = tree.find('./compoundname')

    if kind == 'struct' or kind == 'class' and compoundname is not None:
        name = compoundname.text
    elif definition is not None:
        name = definition.text
        if argsstring is not None and argsstring.text is not None:
            name = name + argsstring.text
    elif name is not None:
        name = name.text
    else:
        name = tree.get('id')

    definitions[name] = kind, documented
    return sourcefile, definitions


def stats_append_definitions(sourcefile, definitions):
    global stats
    if sourcefile is None:
        return

    if sourcefile not in stats:
        stats[sourcefile] = []

    stats[sourcefile].append(definitions)


def parse_compounddef(tree):
    for compounddef in tree.findall('compounddef'):
        sourcefile, definitions = parse(compounddef)
        stats_append_definitions(sourcefile, definitions)

        for memberdef in tree.findall('./compounddef//memberdef'):
            sourcefile, definitions = parse(memberdef)
            stats_append_definitions(sourcefile, definitions)


def parse_file(file):
    tree = XMLTree.parse(file)
    parse_compounddef(tree)


def parse_files(args):
    index = os.path.join(args.dir, 'index.xml')
    if not os.path.exists(index):
        sys.exit('index.xml not found in path: %s' % args.dir)

    tree = XMLTree.parse(index)

    for entry in tree.findall('compound'):
        kind = entry.get('kind')
        if is_excluded(kind):
            continue

        file = os.path.join(args.dir, '%s.xml' % entry.get('refid'))
        parse_file(file)


def compute_stats():
    count_stats = {}
    count_total = 0
    docstrcount = 0

    for (sourcefile, definitions) in stats.items():
        print('Stats for source file: %s' % sourcefile)
        count_stats[sourcefile] = {'total': 0, 'documented': 0}

        for definition in definitions:
            for (name, (kind, documented)) in definition.items():
                count_total += 1
                count_stats[sourcefile]['total'] += 1
                if documented:
                    docstrcount += 1
                    count_stats[sourcefile]['documented'] += 1
                else:
                    print(' --- No docstring for type <%s>: %s' % (kind, name))

        documented, total = count_stats[sourcefile]['documented'], count_stats[sourcefile]['total']
        print(' +++ %s: %s/%s documented' % (sourcefile, documented, total))

    print('Documented code: %s/%s (%s%%)' % (docstrcount, count_total, round(docstrcount * 100.0 / count_total)))


def display_stats():
    for (sourcefile, definitions) in stats.items():
        print('Source file: %s' % sourcefile)

        for definition in definitions:
            for (name, (kind, documented)) in definition.items():
                print(' -- Symbol: %s: %s, %s' % (name, kind, documented))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('dir', action='store', help='Path to Doxygen XML directory')

    # By default, we always check for the existence of any docstring in basically all possible locations.
    # We can narrow down the locations to search docstrings in by specifying a '--no-*' argument that can
    # disable the check for that specific type of symbol.
    parser.add_argument('--no-files', action='store_false', help='Exclude file docstrings')
    parser.add_argument('--no-variables', action='store_false', help='Exclude variable docstrings')
    parser.add_argument('--no-namespaces', action='store_false', help='Exclude namespace docstrings')
    parser.add_argument('--no-functions', action='store_false', help='Exclude variable docstrings')
    parser.add_argument('--no-classes', action='store_false', help='Exclude class docstrings')
    parser.add_argument('--no-structs', action='store_false', help='Exclude struct docstrings')
    parser.add_argument('--no-enums', action='store_false', help='Exclude enum docstrings')
    parser.add_argument('--no-typedefs', action='store_false', help='Exclude typedef docstrings')
    parser.add_argument('--no-defines', action='store_false', help='Exclude define docstrings')

    # Parse arguments and analyze docstring coverage for the specified directory
    args = parser.parse_args()

    global kinds
    kinds['file'] = args.no_files
    kinds['variable'] = args.no_variables
    kinds['namespace'] = args.no_namespaces
    kinds['function'] = args.no_functions
    kinds['class'] = args.no_classes
    kinds['struct'] = args.no_structs
    kinds['enum'] = args.no_enums
    kinds['typedef'] = args.no_typedefs
    kinds['define'] = args.no_defines

    parse_files(args)

    # Compute and print percentage of documented code
    compute_stats()


if __name__ == '__main__':
    main()
