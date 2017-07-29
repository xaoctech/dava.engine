#system
from os.path import split, basename

#pip
from requests_toolbelt.multipart import MultipartEncoder, MultipartEncoderMonitor

#local
from request import GET, POST, DELETE
import options

# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#app-deployment

def file_name_from_path(path):
    head, tail = split(path)
    return tail or basename(head)

class ApiPaths:
    install_app =           "/api/app/packagemanager/package?package={}"
    installation_status =   "/api/app/packagemanager/state"
    uninstall_app =         "/api/app/packagemanager/package?package={}"
    installed_apps =        "/api/app/packagemanager/packages"


def install_app(app_package, deps_packages, cer_path = None, callback = None):
    files = {app_package.file_name : (app_package.file_name, open(app_package.path, "rb"))}

    for dep_package in deps_packages:
        files.update({dep_package.file_name : \
                    (dep_package.file_name, open(dep_package.path, "rb"))})

    if cer_path is not None:
        files.update({file_name_from_path(cer_path) : \
                        (file_name_from_path(cer_path), open(cer_path, "rb"))})

    encoder = MultipartEncoder(fields = files)
    monitor = None
    if callback is not None:
        monitor = MultipartEncoderMonitor(encoder, callback) 
        
    return POST(ApiPaths.install_app.format(app_package.file_name),\
                        data = monitor, \
                        headers = {"Content-Type": monitor.content_type})


def installation_status():
    return GET(ApiPaths.installation_status)
        

def uninstall_app(package_full_name):
    return DELETE(ApiPaths.uninstall_app.format(package_full_name))


def get_installed_apps():
    return GET(ApiPaths.installed_apps)