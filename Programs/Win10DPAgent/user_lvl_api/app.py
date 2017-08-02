#system
import sys
from time import sleep
from json import loads
from base64 import b64encode
from datetime import datetime

#local
sys.path.append("../")
from dp_api import app_deploy, task_manager, performance_data


full_name_tag = "PackageFullName"


def get_installed_package_info(key, value):
    response = app_deploy.get_installed_apps()
    installed_packages = loads(response.text)["InstalledPackages"]
    result = None
    for package in installed_packages:
        if package[key] == value:
            if result is None:
                result = package
            else:
                raise ValueError("Key - value pair {}:{} is not unique."\
                                                           .format(key, value))
    return result
    

def is_installed(key, value):
    return get_installed_package_info(key, value) is not None

# Check if app process is active
def is_active(key, value):
    package_info = get_installed_package_info(key, value)
    if package_info is None:
        return False
    package_full_name = package_info[full_name_tag]
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    for process in processes:
        process_full_name = process.get(full_name_tag)
        if process_full_name is not None and \
                                    process_full_name == package_full_name:
            return True
    return False

# Check if app process is running (on screen)
def is_running(key, value):
    package_info = get_installed_package_info(key, value)
    if package_info is None:
        return False
    package_full_name = package_info[full_name_tag]
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    for process in processes:
        process_full_name = process.get(full_name_tag)
        if process_full_name is not None and \
                                    process_full_name == package_full_name:
            running = process.get("IsRunning")
            return running is not None and running
    return False


def start(key, value):
    status_msg = "\rStarting app: "
    print status_msg + "...",
    
    package_info = get_installed_package_info(key, value)
    if package_info is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

    package_relative_id = package_info["PackageRelativeId"]
    package_full_name = package_info[full_name_tag]

    response = task_manager.start_modern_app(b64encode(package_relative_id), \
                                             b64encode(package_full_name))
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def stop(key, value, force_stop = "yes"):
    status_msg = "\rStopping app: "
    print status_msg + "...",

    package_info = get_installed_package_info(key, value)
    if package_info is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

    package_full_name = package_info[full_name_tag]
    response = task_manager.stop_modern_app(b64encode(package_full_name), \
                                            force_stop)
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def deploy(app_package, deps_packages, cer_path):
    status_msg = "\rUploading app: "

    callback = _UploadCallback(_calc_total_upload_size_in_bytes(app_package, \
                                                    deps_packages), status_msg)
    
    try:
        response = app_deploy.install_app(app_package, deps_packages, cer_path, \
                                          callback)
    except:
        print "Make sure that there is enough free space left on the device."
        raise

    # 'response' may be 'None' here for a short period of time right after return.
    # Just a small fix. Consider rework.
    loop_timeout = 5.0 # seconds
    loop_step = 0.25
    loop_time = 0.0
    while response is None and loop_time < loop_timeout:
        sleep(loop_step)
        loop_time += loop_step

    # According to API docs status code should be 200, but it's 202, so check both.
    if response.status_code in {200, 202} and callback.current_percents == 100:
        print status_msg + "Done."
        return _show_app_installation_status()

    print status_msg + "Failed."
    _print_response_reason_if_exists(response)
    return False


def uninstall(key, value):
    status_msg = "\rUninstalling app: "
    print status_msg + "...",

    package_info = get_installed_package_info(key, value)
    if package_info is None:
        print status_msg + "Not needed."
        print "App with {} == {} is not installed.".format(key, value)
        return True

    if not package_info["CanUninstall"]:
        print "App with {} == {} can't be uninstalled.".format(key, value)
        return False

    package_full_name = package_info[full_name_tag]
    response = app_deploy.uninstall_app(package_full_name)
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def list_installed():
    response = app_deploy.get_installed_apps()
    installed_packages = loads(response.text)["InstalledPackages"]
    fmt = u"{{\"Name\": \"{}\", \"Version\": \"{}\", \"PackageFullName\": \"{}\"}}"
    for package in installed_packages:
        v = package["Version"]
        version = [v["Major"], v["Minor"], v["Build"], v["Revision"]]
        print fmt.format(package["Name"], str(version), package[full_name_tag])


def list_running():
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    fmt = u"{{\"Name\": \"{}\", \"Version\": \"{}\", \"PackageFullName\": \"{}\"}}"
    for process in processes:
        # "IsRunning" tag is not specified in API doc, but exists for running apps
        running = process.get("IsRunning")
        if running is not None and running:
            # "AppName" and "PackageFullName" tags are not specified in API doc, 
            # but exists and used in original web device-portal.
            app_name = process.get("AppName")
            package_full_name = process.get(full_name_tag)
            if app_name is not None and package_full_name is not None:
                v = process["Version"]
                version = [v["Major"], v["Minor"], v["Build"], v["Revision"]]
                print fmt.format(app_name, version, package_full_name)

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
        print status_msg,
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
            print self.status_msg + str(total_percents) + "%",
