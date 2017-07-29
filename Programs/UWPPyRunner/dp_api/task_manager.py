#local
from request import POST, DELETE

# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#task-manager

class ApiPaths:
    start_app = "/api/taskmanager/app?appid={}&package={}"
    stop_app =  "/api/taskmanager/app?package={}&forcestop={}"


def start_modern_app(praid_in_hex64, package_full_name_in_hex64):
    return POST(ApiPaths.start_app.format(praid_in_hex64, \
                                          package_full_name_in_hex64))
    

def stop_modern_app(package_full_name_in_hex64, force_stop):
    return DELETE(ApiPaths.stop_app.format(package_full_name_in_hex64,\
                                           force_stop))