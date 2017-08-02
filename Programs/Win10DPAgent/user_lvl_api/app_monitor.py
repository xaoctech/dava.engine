#system
import sys
from json import loads
from threading import Timer

#local
sys.path.append("../")
from dp_api import performance_data


class AppMonitor:
    def __init__(self, etw_session, package_full_name, deadline_callback, \
                 delay, stop_on_close, timeout = 5.0):
        self.session_ws = performance_data.create_performance_data_session(\
                                              self.on_open, self.on_close, \
                                              self.on_message, self.on_error)
        self.etw_session = etw_session
        self.package_full_name = package_full_name
        self.deadline_callback = deadline_callback
        self.delay = delay
        self.stop_on_close = stop_on_close
        self.timeout = timeout

    def start(self):
        performance_data.loop(self.session_ws)

    def on_open(self, ws):
        # In case of running on separate thread this will fix grabled output
        print "Websocket connection for app process monitor opened.\n",
        self.deadline = Timer(self.delay, self.deadline_callback, \
                              args=[self, self.etw_session])
        self.deadline.start()

    def cancel(self):
        self.deadline.run()

    def on_close(self, ws):
        # In case of running on separate thread this will fix grabled output
        print "App was stopped/closed or session was interrupted.\n",
        print "Websocket connection for app process monitor closed.\n",
        if self.etw_session is not None:
            self.etw_session.session_ws.close()

    def on_error(self, ws, error):
        print error
        ws.close()

    def on_message(self, ws, msg):
        processes = loads(msg)["Processes"]
        for process in reversed(processes):
            pname = process.get("PackageFullName")
            if pname is not None and pname == self.package_full_name:
                is_running = process.get("IsRunning")
                if (is_running is not None and is_running) or \
                                                    not self.stop_on_close:
                    self.deadline.cancel()
                    self.deadline = \
                          Timer(self.timeout, self.deadline_callback,
                                args=[self, self.etw_session])
                    self.deadline.start()