#system
from json import loads
from ssl import CERT_NONE
from base64 import b64encode

#pip
import requests
from websocket import WebSocketApp
from requests_toolbelt.multipart import MultipartEncoder, MultipartEncoderMonitor

URL = "127.0.0.1:10080" # USB connection as default one
RQ_PROTO = "http://"
WS_PROTO = "ws://"
RQ_URL = RQ_PROTO + URL
WS_URL = WS_PROTO + URL
TIMEOUT = 10.0 # seconds
def set_url(url):
    if url.lower().startswith("http://"):
        url = url[len("http://"):]
    if url.lower().startswith("https://"):
        url = url[len("https://"):]
    global URL
    URL = url.rstrip("/")
    __update_rq_url()
    __update_ws_url()

def set_timeout(timeout):
    global TIMEOUT
    TIMEOUT = timeout

def __update_rq_url():
    global RQ_URL
    RQ_URL = RQ_PROTO + URL

def __update_ws_url():
    global WS_URL
    WS_URL = WS_PROTO + URL


TIMEOUT_ERROR_MSG = "Request timeout."
TIMEOUT_INFO_MSG = "Consider increasing the timeout if device is a low-end one."
CONNECTION_ERROR_MSG = "Connection failed."
ERROR_INFO_MSG = "Check the URL ({}), network/usb connection and make sure that device is not in a sleep mode."

def __req_helper(request, path, *args, **kwargs):
    try:
        response = request(RQ_URL + path, timeout = TIMEOUT, *args, **kwargs)
    except requests.exceptions.Timeout:
        print TIMEOUT_ERROR_MSG
        print ERROR_INFO_MSG.format(URL)
        raise
    except requests.exceptions.ConnectionError:
        print CONNECTION_ERROR_MSG
        print ERROR_INFO_MSG.format(URL)
        raise
    else:
        return response

def _GET(path, *args, **kwargs):
    return __req_helper(requests.get, path, *args, **kwargs)

def _POST(path, *args, **kwargs):
    return __req_helper(requests.post, path, *args, **kwargs)
    
def _DELETE(path, *args, **kwargs):
    return __req_helper(requests.delete, path, *args, **kwargs)


class InstalledPackage:
    class Process:
        def __init__(self, full_name):
            self.full_name = full_name
            self.is_running = False
            self.pid = None
            self.update()
        
        def update(self):
            self.is_running = False
            self.pid = None
            response = PerformanceData.get_list_of_running_processes()
            processes = loads(response.text)["Processes"]
            for process in processes:
                process_full_name = process.get("PackageFullName")
                if process_full_name is not None and process_full_name == self.full_name:
                    running = process.get("IsRunning")
                    if running is not None and running:
                        self.is_running = True
                        self.pid = process["ProcessId"]
                    break
    
    def __init__(self, key, value):
        self.key = key 
        self.value = value
        self.name = None
        self.full_name = None
        self.relative_id = None
        self.is_installed = False
        self.version = None
        self.is_uninstallable = False
        self.process = None
        self.update()
        
    def update(self):
        self.name = None
        self.full_name = None
        self.relative_id = None
        self.is_installed = False
        self.version = None
        self.is_uninstallable = False
        self.process = None
    
        response = AppDeployment.get_installed_apps()
        installed_packages = loads(response.text)["InstalledPackages"]
        result = None
        for package in installed_packages:
            if package[self.key] == self.value:
                if result is None:
                    result = package
                else:
                    raise ValueError("Key - value pair {}:{} is not unique.".format(self.key, self.value))
        if result is not None:
            self.name = result["Name"]
            self.full_name = result["PackageFullName"]
            self.relative_id = result["PackageRelativeId"]
            self.is_installed = True
            v = result["Version"]
            self.version = [v["Major"], v["Minor"], v["Build"], v["Revision"]]
            self.is_uninstallable = result["CanUninstall"]
            self.process = InstalledPackage.Process(self.full_name)

    def uninstall(self):
        status_msg = "\rUninstalling {} {}: ".format(self.name, self.version)
        print status_msg + "...",
        if not self.is_uninstallable:
            print status_msg + "Failed. {} cannot be uninstalled.".format(self.name)
            return False
        response = AppDeployment.uninstall_app(self.full_name)
        if response.status_code == 200:
            self.update()
            if not self.is_installed:
                print status_msg + "Done."
                return True
        print status_msg + "Failed."
        if response.text is not None:
            print response.text
        return False

    def stop(self):
        status_msg = "\rStopping {} {}: ".format(self.name, self.version)
        print status_msg + "...",
        self.process.update()
        if not self.process.is_running:
            print status_msg + "Not needed, {} is not running.".format(self.name)
            return True
        else:
            response = TaskManager.stop_modern_app(b64encode(self.full_name), "yes")
            if response.status_code == 200:
                self.process.update()
                if not self.process.is_running:
                    print status_msg + "Done."
                    return True
        print status_msg + "Failed."
        if response.text is not None:
            print response.text
        return False

    def run(self):
        status_msg = "\rStarting {} {}: ".format(self.name, self.version)
        if not self.stop():
            print status_msg + "Failed. {} will not be runned.".format(self.name)
            return False
        print status_msg + "...",
        response = TaskManager.start_modern_app(b64encode(self.relative_id), b64encode(self.full_name))
        if response.status_code == 200:
            self.process.update()
            if self.process.is_running:
                print status_msg + "Done."
                return True
        print status_msg + "Failed."
        if response.text is not None:
            print response.text
        return False


# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#app-deployment
class AppDeployment:
    @staticmethod
    def install_app(files, app_file_name, callback=None):
        encoder = MultipartEncoder(fields=files)
        monitor = None
        if callback is not None:
            monitor = MultipartEncoderMonitor(encoder, callback) 
        
        return _POST("/api/app/packagemanager/package?package=" + app_file_name,\
                      data=monitor, headers={"Content-Type": monitor.content_type})

    @staticmethod
    def installation_status():
        return _GET("/api/app/packagemanager/state")

    @staticmethod
    def uninstall_app(package_full_name):
        return _DELETE("/api/app/packagemanager/package?package=" + package_full_name)

    @staticmethod
    def get_installed_apps():
        return _GET("/api/app/packagemanager/packages")

# Unused
# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#os-information
class OsInformation:
    @staticmethod
    def machine_name():
        return _GET("/api/os/machinename")

    @staticmethod
    def info():
        return _GET("/api/os/info")

    @staticmethod
    def device_family():
        return _GET("/api/os/devicefamily")


# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#performance-data
class PerformanceData:
    @staticmethod
    def get_list_of_running_processes():
        return _GET("/api/resourcemanager/processes")


# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#task-manager
class TaskManager:
    @staticmethod
    def start_modern_app(praid_in_hex64, package_full_name_in_hex64):
        return _POST("/api/taskmanager/app?appid={}&package={}".\
                    format(praid_in_hex64, package_full_name_in_hex64))
    
    @staticmethod                                # 'yes' or 'no'
    def stop_modern_app(package_full_name_in_hex64, force_stop):
        return _DELETE("/api/taskmanager/app?package={}&forcestop={}".\
                    format(package_full_name_in_hex64, force_stop))


# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#etw
class ETW:
    @staticmethod
    def create_etw_session(on_open, on_close, on_message, on_error):
        return WebSocketApp(WS_URL + "/api/etw/session/realtime", \
                            on_open=on_open, \
                            on_close=on_close, \
                            on_message=on_message, \
                            on_error=on_error)

    @staticmethod
    def loop(etw_session):
        etw_session.run_forever(sslopt={"cert_reqs": CERT_NONE})

    @staticmethod
    def get_registered_providers():
        return _GET("/api/etw/providers")

    @staticmethod
    def get_custom_providers():
        return _GET("/api/etw/customproviders")

    @staticmethod
    def enable_provider(etw_session, provider_guid):
        etw_session.send("provider " + provider_guid + " enable")