#local
from request import GET

# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#os-information

class ApiPaths:
    machine_name =  "/api/os/machinename"
    info =          "/api/os/info"
    device_family = "/api/os/devicefamily"

def machine_name():
    return GET(ApiPaths.machine_name)

def info():
    return GET(ApiPaths.info)

def device_family():
    return GET(ApiPaths.device_family)