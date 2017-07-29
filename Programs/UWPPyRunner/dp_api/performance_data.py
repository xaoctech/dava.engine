#system
from ssl import CERT_NONE

#pip
from websocket import WebSocketApp

#local
from request import GET
import options

# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#performance-data

class ApiPaths:
    running_processes = "/api/resourcemanager/processes"


def get_list_of_running_processes():
    return GET(ApiPaths.running_processes)


def create_performance_data_session(on_open, on_close, on_message, on_error):
    return WebSocketApp(options.WS_URL + ApiPaths.running_processes, \
                        on_open = on_open, \
                        on_close = on_close, \
                        on_message = on_message, \
                        on_error = on_error)


def loop(performance_data_session):
    performance_data_session.run_forever(sslopt={"cert_reqs": CERT_NONE})