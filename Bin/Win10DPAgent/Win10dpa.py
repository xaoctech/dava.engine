#system
import sys
import argparse
import subprocess

parser = argparse.ArgumentParser(description="Win10dpa (list, deploy, attach, uninstall, stop)")

parser.add_argument("action",\
                    choices=["list", "deploy", "attach", "uninstall", "stop"],\
                    help="Use '-h' on each command to get command options.")

def do():
    args = parser.parse_args(sys.argv[1:2])
    fmt = "python private/cmd_{}.py {}"
    return subprocess.call(fmt.format(args.action, " ".join("\"{}\"".format(arg) for arg in sys.argv[2:])))

if __name__ == "__main__":
    exit(do())
   

