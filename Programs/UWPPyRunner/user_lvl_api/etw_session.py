#system
import sys
from threading import Thread, Timer

#local
sys.path.append("../")
from dp_api import etw


class ETWSession:
    def __init__(self, app_monitor, sync_start, print_lock, guids_list, \
                 log_parser, *args):
        self.session_ws = etw.create_etw_session(self.on_open, self.on_close, \
                                                self.on_message, self.on_error)
        self.app_monitor = app_monitor
        self.sync_start = sync_start
        self.print_lock = print_lock
        self.guids_list = guids_list
        self.log_parser = log_parser
        self.timer = None
        self.args = args

    def start(self):
        self.session_thread = Thread(target = etw.loop, args=[self.session_ws])
        # Start a timer to avoid deadlock 
        self.timer = Timer(5.0, self.sync_start.notify, args=[False])
        self.timer.start()
        self.session_thread.start()

    def join(self):
        self.session_thread.join()

    def cancel(self):
        self.session_ws.close()

    def enable_guid(self, guid):
        print "Enabling provider with guid: " + guid
        return etw.enable_provider(self.session_ws, guid)

    def on_open(self, ws):
        self.timer.cancel()
        print "Websocket connection for logging session opened."
        for guid in self.guids_list:
            self.enable_guid(guid)
        self.sync_start.notify(True)
    
    def on_close(self, ws):
        with self.print_lock:
            print "Websocket connection for logging session closed."

    def on_message(self, ws, msg):
        self.log_parser(msg, *self.args)

    def on_error(self, ws, error):
        print error
        if self.app_monitor is not None:
            self.app_monitor.session_ws.close()
        ws.close()