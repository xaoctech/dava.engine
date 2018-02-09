#!/usr/bin/env python2.7

import argparse
import os

all_types = {
    'component' : 'Components',
    'system' : 'Systems',
}

all_places =  {
    'game' : 'Programs/NetworkTestGame/Shared/Classes/',
    'network' : 'Modules/NetworkCore/Sources/Scene3D/',
}

all_sources = {
    'component': 'Component',
    'system': 'System',
}

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        'name',
        nargs='*',
        help='Components or systems name')
    parser.add_argument(
        '-r',
        '--root',
        default='../../../../',
        nargs='*')
    parser.add_argument(
        '-t',
        '--type',
        nargs='?',
        choices=all_types.keys())

    parser.add_argument(
        '-p',
        '--place',
        choices=all_places.keys(),
        default='game')

    return parser, parser.parse_args()

if __name__ == '__main__':
    parser, args = parse_args()
    try:
        for name in args.name:
            target = os.path.join(args.root, all_places[args.place.lower()], all_types[args.type.lower()])
            source = all_sources[args.type.lower()]
            for ext in ('cpp', 'h'):
                in_filename = 'TEMPLATE%s.%s' % (source, ext)
                out_filename = '%s%s.%s' % (name, source, ext)
                with open(in_filename, 'rt') as fin:
                    with open(out_filename, 'wt') as fout:
                        for line in fin:
                            fout.write(line.replace('TEMPLATE', name))

            os.system('mv %s%s.* %s' % (name, source, target))

    except:
        parser.print_help()
