#system
import sys
from threading import Timer
from json import loads

#local
sys.path.append("../")
from dp_api import performance_data


class AppMonitor:
    def __init__(self, pname, etw_session, deadline_callback, \
                 print_lock, stop_on_close, timeout = 5.0):
        self.session_ws = performance_data.create_performance_data_session(\
                                              self.on_open, self.on_close, \
                                              self.on_message, self.on_error)
        self.pname = pname
        self.etw_session = etw_session
        self.deadline_callback = deadline_callback
        self.print_lock = print_lock
        self.stop_on_close = stop_on_close
        self.timeout = timeout

    def start(self):
        performance_data.loop(self.session_ws)
        
        #self.session_thread = Thread(target = performance_data.loop,\
        #                             args=[self.session_ws])
        #self.session_thread.daemon = True
        #self.session_thread.start()

    def on_open(self, ws):
        print "Websocket connection for app process monitor opened."
        self.deadline = Timer(self.timeout, self.deadline_callback, \
                              args=[self, self.etw_session])
        self.deadline.start()

    def cancel(self):
        self.deadline.run()

    def on_close(self, ws):
        with self.print_lock:
            print "App was stopped/closed or session was interrupted."
            print "Websocket connection for app process monitor closed."

    def on_error(self, ws, error):
        print error
        self.etw_session.session_ws.close()
        ws.close()

    def on_message(self, ws, msg):
        processes = loads(msg)["Processes"]
        for process in reversed(processes):
            pname = process.get("PackageFullName")
            if pname is not None and pname == self.pname:
                is_running = process.get("IsRunning")
                if (is_running is not None and is_running) or \
                                                    not self.stop_on_close:
                    self.deadline.cancel()
                    self.deadline = \
                          Timer(self.timeout, self.deadline_callback,
                                args=[self, self.etw_session])
                    self.deadline.start()