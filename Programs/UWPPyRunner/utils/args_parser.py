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


parser = argparse.ArgumentParser(description="UWP pyrunner")
parser.add_argument("-app_path",\
                    nargs = "?",\
                    dest = "app_path", \
                    default = None,\
                    help = "Path to uwp .appx[bundle] file.")

parser.add_argument("-deps_paths",\
                    nargs = "?",\
                    dest = "deps_paths",\
                    action = ParseDeps,\
                    default = [],\
                    help = "Comma separated list of paths to dependencies or single path to dependencies dir.")

parser.add_argument("-cer_path",\
                    nargs = "?",\
                    dest = "cer_path", \
                    default = None, \
                    help = "Path to app .cer file.")

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

parser.add_argument("-stop_on_close",\
                    dest = "stop_on_close",\
                    action = "store_true",\
                    help = "Stop the session if app is minimized.")

parser.add_argument("-show_timestamp",\
                    dest = "show_timestamp",\
                    action = "store_true",\
                    help = "Show log timestamp.")

parser.add_argument("-log_levels",\
                    nargs = "?",\
                    dest = "log_levels",\
                    action = Split,\
                    default = [1,2,3,4,5],\
                    help = "Comma separated list of log levels to be shown\
                            (default: \"1,2,3,4,5\").")

parser.add_argument("-providers_guids",\
                    nargs = "?",\
                    dest = "providers_guids",\
                    action = Split,\
                    default = ["4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a"],\
                    help = "Comma separated list of providers GUIDs \
                          (default: \"4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a\".")

parser.add_argument("-providers_names",\
                    nargs = "?",\
                    dest = "providers_names",\
                    action = Split,\
                    default = ["DAVALogProvider","Test-log-service"],\
                    help = "Comma separated list of providers GUIDs \
                            (default: \"DAVALogProvider,Test-log-service\".")

parser.add_argument("-deploy",\
                    dest = "deploy",\
                    action = "store_true",\
                    help = "Deploy the app. Will uninstall any prev. versions.")

parser.add_argument("-uninstall",\
                    dest = "uninstall",\
                    action = "store_true",\
                    help = "Uninstall the app.")

parser.add_argument("-start",\
                    dest = "start",\
                    action = "store_true",\
                    help = "Start the app.")

parser.add_argument("-stop",\
                    dest = "stop",\
                    action = "store_true",\
                    help = "Stop the app.")

parser.add_argument("-ds",\
                    dest = "ds",\
                    action = "store_true",\
                    help = "Deploy and start operations combined.")

parser.add_argument("-force",\
                    dest = "force",\
                    action = "store_true",\
                    help = "'display name' will be used as unique key for stop/start/uninstall opertaions.")

parser.add_argument("-key",\
                    dest = "key",\
                    nargs = "?", \
                    default = None,\
                    help = "Key for stop/start/uninstall options.")

parser.add_argument("-value",\
                    dest = "value",\
                    nargs = "?", \
                    default = None,\
                    help = "Value for stop/start/uninstall options.")

parser.add_argument("-to_file",\
                    dest = "to_file",\
                    nargs = "?", \
                    default = None,\
                    help = "Path to file to log in.")

parser.add_argument("-verbose",\
                    dest = "verbose",\
                    action = "store_true",\
                    help = "Print additional information.")

parser.add_argument('-version', \
                    action='version', \
                    version='%(prog)s 1.0.0')


