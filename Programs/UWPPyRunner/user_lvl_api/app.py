#system
import sys
from time import sleep
from json import loads
from base64 import b64encode
from datetime import datetime

#local
sys.path.append("../")
from dp_api import app_deploy, task_manager, performance_data

#default_provider_name = "Microsoft-Windows-Diagnostics-LoggingChannel"
#default_provider_guid = "4bd2826e-54a1-4ba9-bf63-92b73ea1ac4a"


cache = {}


def get_praid_and_pname(key, value, cached = True):
    global cache
    if cached and key + value in cache:
        return [cache[key + value]["PackageRelativeId"], \
                cache[key + value]["PackageFullName"]]

    response = app_deploy.get_installed_apps()
    installed_apps = loads(response.text)["InstalledPackages"]
    for app in reversed(installed_apps):
        if app[key] == value:
            cache.update({key + value: app})
            return app["PackageRelativeId"], app["PackageFullName"]


def get_version(key, value, cached = True):
    global cache
    if cached and key + value in cache:
        v = cache[key + value]["Version"]
        return [v["Major"], v["Minor"], v["Build"], v["Revision"]]
    
    response = app_deploy.get_installed_apps()
    installed_apps = loads(response.text)["InstalledPackages"]
    for app in reversed(installed_apps):
        if app[key] == value:
            cache.update({cache + value: app})
            v = app["Version"]
            return [v["Major"], v["Minor"], v["Build"], v["Revision"]]


def is_installed(key, value, cached = True):
    pair = get_praid_and_pname(key, value, cached)
    return pair is not None


def start(key, value):
    status_msg = "\rStarting app: "
    sys.stdout.write(status_msg + "...")

    pair = get_praid_and_pname(key, value)

    if pair is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

    response = task_manager.start_modern_app(b64encode(pair[0]), \
                                             b64encode(pair[1]))
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def stop(key, value, force_stop = "yes"):
    status_msg = "\rStopping app: "
    sys.stdout.write(status_msg + "...")

    pair = get_praid_and_pname(key, value)

    if pair is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

    response = task_manager.stop_modern_app(b64encode(pair[1]), force_stop)
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def deploy(app_package, deps_packages, cer_path):
    status_msg = "\rUploading app: "

    callback = _UploadCallback(_calc_total_upload_size_in_bytes(app_package, \
                                                    deps_packages), status_msg)

    response = app_deploy.install_app(app_package, deps_packages, cer_path, \
                                      callback)
    # 'response' may be 'None' for a short period of time right after return.
    # Just a small fix. Consider rework.
    loop_timeout = 5.0 # seconds
    loop_step = 0.25
    loop_time = 0.0
    while response is None and loop_time < loop_timeout:
        sleep(loop_step)
        loop_time += loop_step

    if response.status_code in {200, 202} and callback.current_percents == 100:
        print status_msg + "Done."
        return _show_app_installation_status()

    print status_msg + "Failed."
    _print_response_reason_if_exists(response)
    return False


def uninstall(key, value):
    status_msg = "\rUninstalling app: "
    sys.stdout.write(status_msg + "...")

    pair = get_praid_and_pname(key, value)

    if pair is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

    if not cache[key + value]["CanUninstall"]:
        print "App with {} == {} can't be uninstalled.".format(key, value)
        return False

    response = app_deploy.uninstall_app(pair[1])
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def is_active(key, value):
    pair = get_praid_and_pname(key, value)
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    for process in processes:
        pname = process.get("PackageFullName")
        if pname is not None and pname == pair[1]:
            return True
    return False


def is_running(key, value):
    pair = get_praid_and_pname(key, value)
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    for process in processes:
        pname = process.get("PackageFullName")
        if pname is not None and pname == pair[1]:
            return process["IsRunning"]
    return False


def _print_response_reason_if_exists(response):
    if response.text is not None:
        print response.text
        reason = loads(response.text).get("Reason")
        if reason is not None:
            print "Reason: " + reason


def _check_response_code_and_show_msg(response, codes, status_msg):
    if response.status_code in codes:
        print status_msg + "Done."
        return True

    print status_msg + "Failed."
    _print_response_reason_if_exists(response)
    return False


def _calc_total_upload_size_in_bytes(app_package, deps_packages = []):
    deps_size = 0
    for dep_package in deps_packages:
        deps_size += dep_package.size_in_bytes
    return deps_size + app_package.size_in_bytes


def _show_app_installation_status():
    def show_dots(current_number, maximum_number):
        for i in xrange(0, current_number + 1):
            sys.stdout.write(".")
        for i in xrange(current_number, maximum_number):
            sys.stdout.write(" ")
    
    status_msg = "\rInstalling app: "
    current_dots_number = 0
    maximum_dots_number = 3

    response = app_deploy.installation_status()
    
    sleep_time = 0.5 # seconds
    while response.status_code == 204:
        current_dots_number = (current_dots_number + 1) % maximum_dots_number
        sys.stdout.write(status_msg)
        show_dots(current_dots_number, maximum_dots_number)
        sleep(sleep_time)
        response = app_deploy.installation_status()
    return _check_response_code_and_show_msg(response, {200}, status_msg)


class _UploadCallback:
    def __init__(self, total_size_in_bytes, status_msg):
        self.current_size_in_bytes = 0
        self.total_size_in_bytes = total_size_in_bytes
        self.current_percents = 0
        self.status_msg = status_msg

    def __call__(self, monitor):
        total_percents = monitor.bytes_read * 100 / self.total_size_in_bytes
        if total_percents > self.current_percents:
            self.current_percents = total_percents
            sys.stdout.write(self.status_msg + str(total_percents) + "%")
