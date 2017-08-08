#system
import argparse

#local
import win10_portal_rest_api as dp_api

parser = argparse.ArgumentParser(description="Win10dpa stop app")

parser.add_argument("package_full_name", \
                    nargs="?", \
                    default=None, \
                    help="Package full name to stop.")

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

def stop():
    args = parser.parse_args()
    
    dp_api.set_url(args.url)
    dp_api.set_timeout(args.timeout)

    installed_package = dp_api.InstalledPackage("PackageFullName", args.package_full_name)
    if not installed_package.is_installed:
        print "Package with full name {} is not installed.".format(args.package_full_name)
        print "Nothing to do."
        return True
    return installed_package.stop()

if __name__ == "__main__":
    exit(not stop())
