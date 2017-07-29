#system
from datetime import datetime
from ssl import CERT_NONE

#pip
from websocket import WebSocketApp

#local
from request import GET
import options

# https://docs.microsoft.com/en-us/windows/uwp/debug-test-perf/device-portal-api-core#etw

class ApiPaths:
    create_session =        "/api/etw/session/realtime"
    registered_providers =  "/api/etw/providers"
    custom_providers =      "/api/etw/customproviders"


def create_etw_session(on_open, on_close, on_message, on_error):
    return WebSocketApp(options.WS_URL + ApiPaths.create_session, \
                        on_open = on_open, \
                        on_close = on_close, \
                        on_message = on_message, \
                        on_error = on_error)

def loop(etw_session):
    etw_session.run_forever(sslopt={"cert_reqs": CERT_NONE})

    
def get_registered_providers():
    return GET(ApiPaths.registered_providers)


def get_custom_providers():
    return GET(ApiPaths.custom_providers)

        
def enable_provider(etw_session, provider_guid):
    etw_session.send("provider " + provider_guid + " enable")
