#system
import sys
import argparse
import subprocess

parser = argparse.ArgumentParser(description="Win10DPAgent (list, deploy, attach, uninstall, stop)")

parser.add_argument("action",\
                    choices=["list", "deploy", "attach", "uninstall", "stop"],\
                    help="Use '-h' on each command to get command options.")

def do():
    args = parser.parse_args(sys.argv[1:2])
    exec_string = "python Private/cmd_{}.py {}"
    # Grab all remaining arguments and wrap them into quotes to forward to another script
    args_string = " ".join("\"{}\"".format(arg) for arg in sys.argv[2:])
    return subprocess.call(exec_string.format(args.action, args_string))

if __name__ == "__main__":
    exit(do())
   

