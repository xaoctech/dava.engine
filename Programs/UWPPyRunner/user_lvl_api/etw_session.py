#system
import sys

#local
sys.path.append("../")
from dp_api import etw


class ETWSession:
    def __init__(self, app_monitor, guids_list, log_parser, callback, *args):
        self.session_ws = etw.create_etw_session(self.on_open, self.on_close, \
                                                self.on_message, self.on_error)
        self.app_monitor = app_monitor
        self.guids_list = guids_list
        self.log_parser = log_parser
        self.callback = callback
        self.args = args

    def start(self):
        etw.loop(self.session_ws)

    def cancel(self):
        self.session_ws.close()

    def enable_guid(self, guid):
        # In case of running on separate thread this will fix grabled output
        print "Enabling provider with guid: " + guid + "\n",
        return etw.enable_provider(self.session_ws, guid)

    def on_open(self, ws):
        # In case of running on separate thread this will fix grabled output
        print "Websocket connection for logging session opened.\n",
        for guid in self.guids_list:
            self.enable_guid(guid)
        if self.callback is not None:
            self.callback(*self.args)
        print "Listening"
    
    def on_close(self, ws):
        # In case of running on separate thread this will fix grabled output
        print "Websocket connection for logging session closed.\n",
        if self.app_monitor is not None:
            self.app_monitor.session_ws.close()

    def on_message(self, ws, msg):
        self.log_parser(msg)

    def on_error(self, ws, error):
        print error
        ws.close()