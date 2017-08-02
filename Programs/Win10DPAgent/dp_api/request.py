import requests
import options

TIMEOUT_ERROR_MSG = "Request timeout."
TIMEOUT_INFO_MSG = "Consider increasing the timeout if device is a low-end one."
CONNECTION_ERROR_MSG = "Connection failed."
ERROR_INFO_MSG = "Check the URL ({}), network/usb connection and make sure that device is not in a sleep mode."

def __help(request, path, *args, **kwargs):
    try:
        response = request(options.RQ_URL + path, \
                           timeout = options.TIMEOUT, *args, **kwargs)
    except requests.exceptions.Timeout as timeout:
        print TIMEOUT_ERROR_MSG
        print ERROR_INFO_MSG.format(options.URL)
        raise
    except requests.exceptions.ConnectionError as connection_error:
        print CONNECTION_ERROR_MSG
        print ERROR_INFO_MSG.format(options.URL)
        raise
    else:
        return response

def GET(path, *args, **kwargs):
    return __help(requests.get, path, *args, **kwargs)

def POST(path, *args, **kwargs):
    return __help(requests.post, path, *args, **kwargs)
    
def DELETE(path, *args, **kwargs):
    return __help(requests.delete, path, *args, **kwargs)