#system
from threading import Lock
from sys import stdout

#local
from dp_api import options

from user_lvl_api import app

from user_lvl_api.etw_session import ETWSession
from user_lvl_api.app_monitor import AppMonitor

from utils.args_parser import parser
from utils.package import Package
from utils.log_parser import LogParser
from utils.sync_start import SyncStart

def print_args_values(args):
    print "\n###### Args values ######"
    print "### App path: " + str(args.app_path)
    print "### Deps paths: "
    for dep_path in args.deps_paths:
        print "- " + dep_path
    print "### Url: " + str(args.url)
    print "### Stop on close: " + str(args.stop_on_close)
    print "### Show timestamp: " + str(args.show_timestamp)
    print "### Log levels: " + str(args.log_levels)
    print "### Providers GUIDs: "
    for p_guid in args.providers_guids:
        print "- " + str(p_guid)
    print "### Providers names: "
    for p_name in args.providers_names:
        print "- " + str(p_name)
    print "### Deploy: " + str(args.deploy)
    print "### Uninstall: " + str(args.uninstall)
    print "### Start: " + str(args.start)
    print "### Key: " + str(args.key)
    print "### Value: " + str(args.value)
    print "### Force: " + str(args.force)
    print "### Verbose: " + str(args.verbose)


def print_package_values(package):
    print "\n###### Package values ######"
    print "### Path: " + str(package.path)
    print "### File name: " + str(package.file_name)
    print "### Version: " + str(package.version)
    print "### Size in bytes: " + str(package.size_in_bytes)


def stop_app_monitor_and_etw_sessions(app_monitor_session, etw_session):
    if app_monitor_session is not None:
        app_monitor_session.session_ws.close()
    if etw_session is not None:
        etw_session.session_ws.close()


family_name_tag = "PackageFamilyName"
name_tag = "Name"


def uninstall(key, value):
    if app.is_installed(key, value):
        return app.uninstall(key, value)
    else:
        print "Failed to uninstall the app. App is not installed."
        return False


def start(key, value):
    if app.is_installed(key, value):
        if app.is_active(key, value):
            if not app.stop(key, value):
                print "App is already running and failed to stop."
                return False
        return app.start(key, value)
    else:
        print "Failed to start the app. App is not installed."
        return False


def stop(key, value):
    if app.is_installed(key, value):
        if app.is_active(key, value):
            return app.stop(key, value)
        else:
            print "App is not active. Skipping 'stop' step."
            return True
    else:
        print "Failed to stop the app. App is not installed."
        return False


def args_check(args):
    if args.key is not None and args.value is None or \
       args.value is not None and args.key is None:
        print "Both flags ('-key' and '-value') must be provided."
        exit()

    if args.deploy and args.app_path is None:
        print "App path ('-app_path' flag) must be provided to deploy the app." 
        exit()
    
    if args.app_path is None and args.key is None and args.value is None:
        print "'-app_path' flag or '-key' and '-value' flags must be provided."
        exit()

def args_prepare(args):
    args.log_levels = [int(level) for level in args.log_levels]

    if args.ds:
        args.deploy = True
        args.start = True

    if args.verbose:
        print_args_values(args)


def main():
    args = parser.parse_args()
    options.set_url(args.url)
    options.set_timeout(args.timeout)
    
    args_prepare(args)    
    args_check(args)
        
    key = None
    value = None

    app_package = None
    deps_packages = []

    if args.app_path is not None:
        app_package = Package(args.app_path)
    
        if args.verbose:
            print_package_values(app_package)    

        for dep_path in args.deps_paths:
            package = Package(dep_path)
            deps_packages.append(package)
            if args.verbose:
                print_package_values(package)

        key = family_name_tag
        value = app_package.identity_name

        if args.force and not app.is_installed(key, value):
            key = name_tag
            value = app_package.get_display_name()

    if args.key is not None and args.value is not None:
        key = args.key
        value = args.value

    fine = True

    if args.stop:
        fine = stop(key, value)

    if args.uninstall:
        fine = uninstall(key, value)
    
    if fine and args.deploy:
        if app.is_installed(key, value, cached = False) and app.get_version(key, value) >= app_package.version:
            print "Installed version: " + str(app.get_version(key, value))
            print "Deploying version: " + str(app_package.version)
            print "Specify '-uninstall' flag."
            exit()
        fine = app.deploy(app_package, deps_packages, args.cer_path)
        status_msg = "\rChecking installation result: "
        stdout.write(status_msg + "...")
        if fine and not (app.is_installed(family_name_tag, app_package.identity_name, cached = False) and \
                         app.get_version(family_name_tag, app_package.identity_name) == app_package.version):
            print status_msg + "Failed."
            print "Installation failed. Make sure that .appx[bundle] file is fine and all dependencies are provided."
            fine = False
        else:
            print status_msg + "Done."
    
    if fine and args.start:
        sync_start = SyncStart()
        print_lock = Lock()

        log_parser = LogParser(args.log_levels, args.providers_names, \
                               args.show_timestamp)

        app_monitor = None
        
        try:
            etw_session = ETWSession(app_monitor, sync_start, print_lock, \
                                     args.providers_guids, log_parser)
            # Loop on a new thread
            etw_session.start()
            # Wait etw session to start before starting the app
            sync_start.wait()
        
            if not sync_start.status:
                print "ETW session failed to start."
                return

            if args.deploy:
                key = family_name_tag
                value = app_package.identity_name
                fine = app.start(family_name_tag, app_package.identity_name)
            else:
                fine = start(key, value)
                    
            if fine:
                pname = app.get_praid_and_pname(key, value)[1]
                app_monitor = AppMonitor(pname, etw_session, \
                                         stop_app_monitor_and_etw_sessions, \
                                         print_lock, args.stop_on_close)
                # Loop on the main thread
                app_monitor.start()
            else:
                etw_session.cancel()
        except:
            stop_app_monitor_and_etw_sessions(app_monitor, etw_session)
            raise

if __name__ == "__main__":
    main()