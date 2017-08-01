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


cache = {}


# Update cache and test if given key-value pair is unique
def update_cache(key, value):
    global cache
    response = app_deploy.get_installed_apps()
    installed_packages = loads(response.text)["InstalledPackages"]
    check = True
    for package in installed_packages:
        if package[key] == value:
            if check:
                cache.update({key + value: package})
                check = False
            else:
                raise ValueError("Key - value pair {}:{} is not unique."\
                                                           .format(key, value))

# Get specific tag for package with given key-value pair
def get_tag(key, value, tag, cached = True):
    if not cached or key + value not in cache:
        update_cache(key, value)
    return cache[key + value][tag]


def get_package_full_name(key, value, cached = True):
    if key == full_name_tag:
        return value
    else:
        return get_tag(key, value, full_name_tag, cached)


def get_package_relative_id(key, value, cached = True):
    return get_tag(key, value, "PackageRelativeId", cached)


def get_version(key, value, cached = True):
    v = get_tag(key, value, "Version", cached)
    return [v["Major"], v["Minor"], v["Build"], v["Revision"]]


def is_installed(key, value, cached = True):
    return get_package_full_name(key, value, cached) is not None

# Check if app process is active
def is_active(key, value):
    package_full_name = get_package_full_name(key, value)
    response = performance_data.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    for process in processes:
        process_full_name = process.get(full_name_tag)
        if process_full_name is not None and \
                                    process_full_name == package_full_name:
            return True
    return False

# Check if app process is running (is active and not sleeping)
def is_running(key, value):
    package_full_name = get_package_full_name(key, value)
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
    sys.stdout.write(status_msg + "...")

    package_full_name = get_package_full_name(key, value)
    package_relative_id = get_package_relative_id(key, value)

    if package_full_name is None or package_relative_id is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

    response = task_manager.start_modern_app(b64encode(package_relative_id), \
                                             b64encode(package_full_name))
    return _check_response_code_and_show_msg(response, {200}, status_msg)


def stop(key, value, force_stop = "yes"):
    status_msg = "\rStopping app: "
    sys.stdout.write(status_msg + "...")

    package_full_name = get_package_full_name(key, value)

    if package_full_name is None:
        print status_msg + "Failed."
        print "App with {} == {} is not installed.".format(key, value)
        return False

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

    # 'response' may be 'None' for a short period of time right after return.
    # Just a small fix. Consider rework.
    loop_timeout = 5.0 # seconds
    loop_step = 0.25
    loop_time = 0.0
    while response is None and loop_time < loop_timeout:
        sleep(loop_step)
        loop_time += loop_step

    # According to API docs status code should be 200, but it's is 202, so check both
    if response.status_code in {200, 202} and callback.current_percents == 100:
        print status_msg + "Done."
        return _show_app_installation_status()

    print status_msg + "Failed."
    _print_response_reason_if_exists(response)
    return False


def uninstall(key, value):
    status_msg = "\rUninstalling app: "
    sys.stdout.write(status_msg + "...")

    package_full_name = get_package_full_name(key, value)

    if package_full_name is None:
        print status_msg + "Not needed."
        print "App with {} == {} is not installed.".format(key, value)
        return True

    if not get_tag(key, value,"CanUninstall"):
        print "App with {} == {} can't be uninstalled.".format(key, value)
        return False

    response = app_deploy.uninstall_app(package_full_name)
    return _check_response_code_and_show_msg(response, {200}, status_msg)


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
