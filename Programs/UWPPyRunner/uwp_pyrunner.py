#sys
from json import loads
from threading import Thread

#local
from dp_api import options, app_deploy, performance_data

from utils import logger, package, args_parser

from user_lvl_api import app
from user_lvl_api.app_monitor import AppMonitor
from user_lvl_api.etw_session import ETWSession

options.set_url("http://10.128.95.108")

full_name_tag = "PackageFullName"
family_name_tag = "PackageFamilyName"

def deploy(app_path, deps_paths, cer_path, url, timeout, force = False):
    app_package = package.Package(app_path)
    deps = []
    for dep_path in deps_paths:
        deps.append(package.Package(dep_path))
    
    if force:
        if not uninstall(app.get_package_full_name(family_name_tag, app_package.identity_name), url, timeout):
            print "Uninstall failed, deploy will not start."
            return False

    if app.is_installed(family_name_tag, app_package.identity_name, force):
        installed_version = app.get_version(family_name_tag, app_package.identity_name)
        if installed_version >= package.version:
            print "Installed version: " + str(installed_version)
            print "Deploying version: " + str(app_package.version)
            print "Deploy will have no effect. Uninstall current version first."
            return False

    fine = app.deploy(app_package, deps, cer_path)
    if fine and app.is_installed(family_name_tag, app_package.identity_name, cached = False) \
            and app.get_version(family_name_tag, app_package.identity_name) == app_package.identity_name:
        print "Deploy check: Done."
        return True
    else:
        print "Deploy check: Falied."
        return False


def uninstall(package_full_name, url, timeout):
    fine = app.uninstall(full_name_tag, package_full_name)
    if fine and not app.is_installed(full_name_tag, package_full_name, cached = False):
        print "Uninstall check: Done."
        return True
    else:
        print "Uninstall check: Failed."
        return False 


def run(package_full_name, guids, channels, levels, show_timestamp, delay, stop_on_close, url, timeout):
    if not stop(package_full_name, url, timeout):
        print "Failed to stop the app, run will not start."
        return False
    attach(package_full_name, guids, channels, levels, show_timestamp, delay, stop_on_close, url, timeout, app.start, full_name_tag, package_full_name)


def stop(package_full_name, url, timeout):
    fine = app.stop(full_name_tag, package_full_name)
    return fine and not app.is_running(full_name_tag, package_full_name)


def attach(package_full_name, guids, channels, levels, show_timestamp, delay, stop_on_close, url, timeout, callback = None, *args):

    def deadline_callback(etw_session, app_monitor):
        if etw_session is not None:
            etw_session.session_ws.close()
        if app_monitor is not None:
            app_monitor.session_ws.close()

    log_parser = logger.LogParser(levels, channels, show_timestamp)
    etw_session = None
    app_monitor = None

    try:
        etw_session = ETWSession(app_monitor, \
                                 guids, \
                                 log_parser, \
                                 callback, \
                                 *args)

        if package_full_name is not None:
            if not app.is_installed(full_name_tag, package_full_name):
                print "App is not installed. Install the app or use attach without app monitor."
                return
    
            app_monitor = AppMonitor(etw_session, \
                                     package_full_name, \
                                     deadline_callback, \
                                     delay, \
                                     stop_on_close,\
                                     timeout)
            Thread(target = app_monitor.start).start()

        etw_session.start()
    except:
        deadline_callback(etw_session, app_monitor)
        raise


def list_installed(url = None, timeout = None):
    response = app_deploy.get_installed_apps()
    installed_packages = loads(response.text)["InstalledPackages"]
    for installed_package in installed_packages:
        fmt = u"{:<17}: {}"
        print fmt.format("Name", installed_package["Name"])
        v = installed_package["Version"]
        print fmt.format("Version", str([v["Major"], v["Minor"], v["Build"], v["Revision"]]))
        print fmt.format("Package full name", installed_package[full_name_tag])
        print


def list_running(url = None, timeout = None):
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    for process in processes:
        fmt = u"{:<17}: {}"
        # This check is the same as original check in web device-portal in /js/controls.js
        app_name = process.get("AppName")
        package_full_name = process.get(full_name_tag)
        if package_full_name is not None and app_name is not None:
            print fmt.format("Name", app_name)
            v = process["Version"]
            print fmt.format("Version", str([v["Major"], v["Minor"], v["Build"], v["Revision"]]))
            print fmt.format("Package full name", package_full_name)
            print
        
def error():
    print "ARGS PARSING ERROR"

def f(args):
    return {
        "list": (list_running if args.what == "running" else (list_installed if args.what == "installed" else error)),
    }.get(args.s_name, error)
        
def main():
    args = args_parser.parser.parse_args()
    options.set_url(args.url)
    options.set_timeout(args.timeout)
    f(args)()
    return


if __name__ == "__main__":
    main()