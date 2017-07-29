URL = "127.0.0.1:10080" # USB connection as default one

RQ_PROTO = "http://"
WS_PROTO = "ws://"

RQ_URL = RQ_PROTO + URL
WS_URL = WS_PROTO + URL

TIMEOUT = 10.0 # seconds
    

def set_url(url):
    global URL
    URL = url
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


# There are many troubles with 'httpS' and 'wsS' and certificates.
# Omit these functions for now, since everything works fine with 
# unsecured connections.
#
#def set_rq_proto(rq_proto):
#    global RQ_PROTO
#    RQ_PROTO = rq_proto
#    __update_rq_url()

    
#def set_ws_proto(ws_proto):
#    global WS_PROTO
#    WS_PROTO = ws_proto
#    __update_ws_url()