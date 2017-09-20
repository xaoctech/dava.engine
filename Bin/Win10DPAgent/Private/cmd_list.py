#system
import argparse
from json import loads

#local
import win10_device_portal_rest_api as dp_api

parser = argparse.ArgumentParser(description="Win10DPAgent list installed/running apps")

parser.add_argument("what", \
                    choices=["installed", "running"], \
                 	help="What to list: installed or running apps.")

parser.add_argument("-url", \
                    nargs="?", \
                    dest="url", \
                    default="127.0.0.1:10080", \
                    help="Target device url " \
                    "(default is usb: \"127.0.0.1:10080\").")

parser.add_argument("-timeout", \
                    nargs="?", \
                    dest="timeout", \
                    type=float, \
                    default=10.0, \
                    help="Requests timeout. (default: 10.0).")


def list_installed():
    response = dp_api.AppDeployment.get_installed_apps()
    installed_packages = loads(response.text)["InstalledPackages"]
    fmt = u"{{\"Name\": \"{}\", \"Version\": \"{}\", \"PackageFullName\": \"{}\"}},"
    print "{\"InstalledPackages\": [",
    tmp = None
    for package in installed_packages:
        if tmp is not None:
            print tmp
        v = package["Version"]
        version = [v["Major"], v["Minor"], v["Build"], v["Revision"]]
        tmp = fmt.format(package["Name"].replace("\"", "\\\""), version, package["PackageFullName"])
    if tmp is not None:
        print tmp.rstrip(",")
    print "]}"
    return True


def list_running():
    response = dp_api.PerformanceData.get_list_of_running_processes()
    processes = loads(response.text)["Processes"]
    fmt = u"{{\"Name\": \"{}\", \"Version\": \"{}\", \"PackageFullName\": \"{}\"}},"
    print "{\"Processes\": ["
    tmp = None
    for process in processes:
        # "IsRunning" tag is not specified in API doc, but exists for running apps
        running = process.get("IsRunning")
        if running is not None and running:
            # "AppName" and "PackageFullName" tags are not specified in API doc, 
            # but exists and used in original web device-portal.
            app_name = process.get("AppName")
            package_full_name = process.get("PackageFullName")
            if app_name is not None and package_full_name is not None:
                if tmp is not None:
                    print tmp
                v = process["Version"]
                version = [v["Major"], v["Minor"], v["Build"], v["Revision"]]
                tmp = fmt.format(app_name, version, package_full_name)
    if tmp is not None:
        print tmp.rstrip(",")
    print "]}"
    return True


def list():
    args = parser.parse_args()
    
    dp_api.set_url(args.url)
    dp_api.set_timeout(args.timeout)
    
    if args.what == "installed":
        return list_installed()

    if args.what == "running":
        return list_running()
    
    return False
    
if __name__ == "__main__":
    exit(not list())

