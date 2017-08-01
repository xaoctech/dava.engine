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


parser = argparse.ArgumentParser(description="UWPPyRunner")

#
# Global arguments
#
parser.add_argument("-url",\
                    nargs = "?",\
                    dest = "url",\
                    default = "127.0.0.1:10080",\
                    help = "Target device url \
                            (default is usb: \"127.0.0.1:10080\").")

parser.add_argument("-timeout",\
                    nargs = "?",\
                    dest = "timeout",\
                    type = float,\
                    default = 10.0,\
                    help = "Requests timeout. (default: 10.0).")

parser.add_argument('-version', \
                    action='version', \
                    version='%(prog)s 1.0.0')

#parser.add_argument("-to_file",\
#                    dest = "to_file",\
#                    nargs = "?", \
#                    default = None,\
#                    help = "Path to file to log in.")

subparsers = parser.add_subparsers(dest = "s_name", help = "TEST")

#
# 'Deploy' command parser
#

deploy_parser = subparsers.add_parser("deploy", help = "App deploy.")

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
                           help = "Comma separated list of paths to dependencies files or \
                                   dependencies dirs.")

deploy_parser.add_argument("-cer_path",\
                           nargs = "?",\
                           dest = "cer_path", \
                           default = None, \
                           help = "Path to app .cer file.")


#
# 'Uninstall' command parser
#

uninstall_parser = subparsers.add_parser("uninstall", help = "App uninstall.")

uninstall_parser.add_argument("package_full_name",\
                              nargs = "?",\
                              #dest = "package_full_name",\
                              #required = True, \
                              help = "Package full name.")

#
# 'Run' command parser
#
                    
run_parser = subparsers.add_parser("run", help = "Run the app and listen to logs.")

run_parser.add_argument("package_full_name",\
                        nargs = "?",\
                        #dest = "package_full_name",\
                        #required = True, \
                        help = "Package full name.")
          
run_parser.add_argument("-guids",\
                        nargs = "?",\
                        dest = "guids", \
                        action = Split,\
                        default = ["4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a"],\
                        help = "Comma separated list of providers GUIDs \
                          (default: \"4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a\".")

run_parser.add_argument("-channels",\
                        nargs = "?",\
                        dest = "channels",\
                        action = Split,\
                        default = ["DAVALogProvider","Test-log-service"],\
                        help = "Comma separated list of providers GUIDs \
                              (default: \"DAVALogProvider,Test-log-service\".")

run_parser.add_argument("-log_levels",\
                        nargs = "?",\
                        dest = "log_levels",\
                        action = Split,\
                        default = [1,2,3,4,5],\
                        help = "Comma separated list of log levels to be shown\
                                (default: \"1,2,3,4,5\").")

run_parser.add_argument("-no_timestamp",\
                        dest = "no_timestamp",\
                        action = "store_true",\
                        help = "Disable timestamp.")

run_parser.add_argument("-delay",\
                        nargs = "?",\
                        dest = "delay",\
                        type = float, \
                        default = 20.0,\
                        help = "Delay for app monitor to wait before the app start (deafult: 20.0).")

run_parser.add_argument("-stop_on_close",\
                        dest = "stop_on_close",\
                        action = "store_true",\
                        help = "Stop the session if app is minimized.")


#
# 'Attach' command parser
#

attach_parser = subparsers.add_parser("attach", help = "Attach to event tracer without run.")

attach_parser.add_argument("-app_to_monitor",\
                           nargs = "?",\
                           dest = "app_to_monitor",\
                           default = None, \
                           help = "Monitor app activity with specified package full name.")
         
attach_parser.add_argument("-guids",\
                           nargs = "?",\
                           dest = "guids", \
                           action = Split,\
                           default = ["4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a"],\
                           help = "Comma separated list of providers GUIDs \
                          (default: \"4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a\".")

attach_parser.add_argument("-channels",\
                           nargs = "?",\
                           dest = "channels",\
                           action = Split,\
                           default = ["DAVALogProvider","Test-log-service"],\
                           help = "Comma separated list of providers GUIDs \
                              (default: \"DAVALogProvider,Test-log-service\".")

attach_parser.add_argument("-log_levels",\
                           nargs = "?",\
                           dest = "log_levels",\
                           action = Split,\
                           default = [1,2,3,4,5],\
                           help = "Comma separated list of log levels to be shown\
                                   (default: \"1,2,3,4,5\").")

attach_parser.add_argument("-no_timestamp",\
                           dest = "no_timestamp",\
                           action = "store_true",\
                           help = "Disable timestamp.")

attach_parser.add_argument("-delay",\
                           nargs = "?",\
                           dest = "delay",\
                           type = float, \
                           default = 20.0,\
                           help = "Delay for app monitor to wait before the app start (deafult: 20.0).")

attach_parser.add_argument("-stop_on_close",\
                           dest = "stop_on_close",\
                           action = "store_true",\
                           help = "Stop the session if app is minimized.")

#
# 'Stop' command parser
#

stop_parser = subparsers.add_parser("stop", help = "App stop.")

stop_parser.add_argument("package_full_name",\
                         nargs = "?",\
                         #dest = "package_full_name",\
                         #required = True, \
                         help = "Package full name.")

#
# 'List' command parser
#

list_parser = subparsers.add_parser("list", help = "List installed or running apps.")

list_parser.add_argument("what",\
                         #dest = "installed",\
                         help = "Specify what to list: 'running' or 'installed' apps.")