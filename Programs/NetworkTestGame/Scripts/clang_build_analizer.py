#!/usr/bin/python
import argparse, sys, subprocess

if __name__ == '__main__':
    arg_parser = argparse.ArgumentParser()
    arg_parser.add_argument('--error', type=bool, default=True,
                            help='error (default=true)')

    arg_parser.add_argument('--warning', type=bool, default=False,
                            help='warning (default=false)')
    args = arg_parser.parse_args()


    red_text = "\033[0;31m%s\033[0m"

    patterns = [": %s: " % k for (k,v) in args.__dict__.items() if v]
    code = 0
    blocks = []
    block = []
    separate = False

    proc = subprocess.Popen(['cmake','--build', '.'], stdout=subprocess.PIPE)
    while proc.poll() is None:
        sys.stderr.write('.')
        line = proc.stdout.readline()
        if 'In file included from' in line:
            if separate:
                blocks.append('\n'.join(block))
                block = []
                separate = False
            block.append(line)
        elif block:
            block.append(line)
            separate = True

    if separate:
        blocks.append('\n'.join(block))

    result = []
    for block in blocks:
        if result:
            if ": note: " in block:
                result[-1] += block
                continue
        result.append(block)

    for block in result:
        has_pattern = False
        for p in patterns:
            if p in block:
                block = block.replace(p, red_text % p)
                has_pattern = True
        if has_pattern:
            print block
            print
            code = 1

    exit(code)
