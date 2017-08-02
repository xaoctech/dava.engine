#system
import os
import argparse

class Split(argparse.Action):
    def __call__(self, parser, args, values, option_string = None):
        res = []
        for value in values.split(","):
            res.append(value.strip())
        setattr(args, self.dest, res)


class ParseDeps(argparse.Action):
    def __call__(self, parser, args, values, option_string = None):
        res = []
        list = values.split(",")
        for value in list:
            value.strip()
            if os.path.isdir(value):
                for f in os.listdir(value):
                    p = os.path.join(value, f)
                    if os.path.isfile(p):
                        res.append(p)
            else:
                res.append(value)
        setattr(args, self.dest, res)


# Main parser
parser = argparse.ArgumentParser(description="UWPPyRunner")
subparsers = parser.add_subparsers(dest = "s_name", help = "TEST")

# Shared parent
shared_parent = argparse.ArgumentParser(add_help = False)

# Global arguments
shared_parent.add_argument('-version', \
                           action='version', \
                           version='%(prog)s 1.0.0')

shared_parent.add_argument("-url",\
                           nargs = "?",\
                           dest = "url",\
                           default = "127.0.0.1:10080",\
                           help = "Target device url" \
                            "(default is usb: \"127.0.0.1:10080\").")

shared_parent.add_argument("-timeout",\
                           nargs = "?",\
                           dest = "timeout",\
                           type = float,\
                           default = 10.0,\
                           help = "Requests timeout. (default: 10.0).")


# 'Deploy' command parser
deploy_parser = subparsers.add_parser("deploy", \
                                      help = "App deploy.", \
                                      parents = [shared_parent])

deploy_parser.add_argument("-app_path",\
                           nargs = "?",\
                           dest = "app_path", \
                           required = True, \
                           help = "Path to uwp .appx[bundle] file.")

deploy_parser.add_argument("-deps_paths",\
                           nargs = "?",\
                           dest = "deps_paths",\
                           action = ParseDeps,\
                           default = [],\
                           help = "Comma separated list of paths to" \
                                   "dependencies files or dependencies dirs.")

deploy_parser.add_argument("-cer_path",\
                           nargs = "?",\
                           dest = "cer_path", \
                           default = None, \
                           help = "Path to app .cer file.")

deploy_parser.add_argument("-force",\
                           dest = "force",\
                           action = "store_true",\
                           help = "Uninstall any installed version with" \
                                  "same PackageFamilyName.")


# 'Uninstall' command parser
uninstall_parser = subparsers.add_parser("uninstall", \
                                         help = "App uninstall.", \
                                         parents = [shared_parent])

uninstall_parser.add_argument("package_full_name",\
                              nargs = "?",\
                              help = "Package full name.")

# Parent parser for 'run' and 'attach' parsers
run_attach_parent = argparse.ArgumentParser(add_help = False, \
                                            parents = [shared_parent])

run_attach_parent.add_argument("-guids",\
                           nargs = "?",\
                           dest = "guids", \
                           action = Split,\
                           default = ["4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a"],\
                           help = "Comma separated list of providers GUIDs" \
                         "(default: \"4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a\").")

run_attach_parent.add_argument("-channels",\
                             nargs = "?",\
                             dest = "channels",\
                             action = Split,\
                             default = ["DAVALogProvider","Test-log-service"],\
                             help = "Comma separated list of providers GUIDs" \
                             "(default: \"DAVALogProvider,Test-log-service\").")

run_attach_parent.add_argument("-log_levels",\
                               nargs = "?",\
                               dest = "log_levels",\
                               action = Split,\
                               default = [1,2,3,4,5],\
                               help = "Comma separated list of log levels to" \
                                      "be shown (default: \"1,2,3,4,5\").")

run_attach_parent.add_argument("-no_timestamp",\
                               dest = "no_timestamp",\
                               action = "store_true",\
                               help = "Disable timestamp.")

run_attach_parent.add_argument("-wait_time",\
                               nargs = "?",\
                               dest = "wait_time",\
                               type = float, \
                               default = 20.0,\
                               help = "Time app monitor will wait app to" \
                                      "start before session cancelling" \
                                      "(default: 20.0).")

run_attach_parent.add_argument("-stop_on_close",\
                               dest = "stop_on_close",\
                               action = "store_true",\
                               help = "Stop the session if app is minimized.")


# 'Run' command parser
run_parser = subparsers.add_parser("run", \
                                   help = "Run the app and listen to logs.", \
                                   parents = [run_attach_parent])

run_parser.add_argument("package_full_name",\
                        help = "Package full name.")
          

# 'Attach' command parser
attach_parser = subparsers.add_parser("attach", \
                                help = "Attach to event tracer without run.", \
                                parents = [run_attach_parent])

attach_parser.add_argument("-app_to_monitor",\
               nargs = "?",\
               dest = "package_full_name",\
               default = None, \
               help = "Monitor app activity with specified package full name.")

# 'Stop' command parser
stop_parser = subparsers.add_parser("stop", \
                                    help = "App stop.", \
                                    parents = [shared_parent])

stop_parser.add_argument("package_full_name",\
                         nargs = "?",\
                         help = "Package full name.")

# 'List' command parser
list_parser = subparsers.add_parser("list", \
                                    help = "List installed or running apps.", \
                                    parents = [shared_parent])

list_parser.add_argument("what",\
                 help = "Specify what to list: 'running' or 'installed' apps.")