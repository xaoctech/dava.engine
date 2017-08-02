#system
import sys
from json import loads
from threading import Thread, Timer, Lock
import logging

#local
sys.path.append("../")
from dp_api import etw

kernel_provider_name = "Microsoft-Windows-Kernel-Process"

#logging.basicConfig(level=logging.DEBUG)

class ETWSession:
    def __init__(self, package_full_name, wait_time, guids_list, \
                                            log_parser, callback, *args):
        self.session_ws = etw.create_etw_session(self.on_open, self.on_close, \
                                                self.on_message, self.on_error)
        self.package_full_name = package_full_name
        self.wait_time = wait_time
        self.guids_list = guids_list
        self.log_parser = log_parser
        self.callback = callback
        self.args = args
        
        self.deadline_timer = Timer(self.wait_time, self.__check_pid)
        self.pid_lock = Lock()
        self.pid = None

    def start(self):
        etw.loop(self.session_ws)

    def cancel(self):
        self.session_ws.close()

    def enable_guid(self, guid):
        print "Enabling provider with guid: " + guid
        return etw.enable_provider(self.session_ws, guid)

    def on_open(self, ws):
        print "Websocket connection for logging session opened."
        for guid in self.guids_list:
            self.enable_guid(guid)

        if self.package_full_name is not None:
            self.__enable_kernel_process()
            self.deadline_timer.start()

        if self.callback is not None:
            self.callback(*self.args)

        print "Listening"
    
    def on_close(self, ws):
        self.deadline_timer.cancel()
        if self.deadline_timer.isAlive():
            self.deadline_timer.join()
        print "Websocket connection for logging session closed."

    def on_message(self, ws, msg):
        events = loads(msg)["Events"]
        if events is None:
            return
        for event in events:
            if event["ProviderName"] == kernel_provider_name and \
                                                    event["ID"] in {1, 2}:
                self.__parse_kernel_event(event)
            else:
                self.log_parser(event)

    def on_error(self, ws, error):
        print error
        ws.close()

    def set_pid(self, pid):
        with self.pid_lock:
            self.pid = pid

    def __enable_kernel_process(self):
        response = etw.get_registered_providers()
        providers = loads(response.text)["Providers"]
        kernel_provider_guid = None
        for provider in providers:
            if provider["Name"] == kernel_provider_name:
                kernel_provider_guid = provider["GUID"]
                break
        return etw.enable_provider(self.session_ws, kernel_provider_guid)

    def __parse_kernel_event(self, event):
        t_pid = None
        with self.pid_lock:
            t_pid = self.pid

        if event["TaskName"] == "ProcessStart" and t_pid is None:
            package_full_name = event.get("PackageFullName")
            if package_full_name is not None and \
                                package_full_name == self.package_full_name:
                self.set_pid(event["ProcessID"])

        if event["TaskName"] == "ProcessStop":
            t_pid = None
            with self.pid_lock:
                t_pid = self.pid

            pid = event.get("ProcessID")

            if pid is not None and t_pid is not None and pid == t_pid:
                # App stoppped
                self.session_ws.close()

                self.deadline_timer.cancel()
                if self.deadline_timer.isAlive():
                    self.deadline_timer.join()

                exit_code = event["ExitCode"]
                exit_msg = event["Message"]
                # print event["Message"] failing because event["Message"] contains unprintable (in standart win cmd) characters
                print exit_msg.encode('cp437', 'ignore')
                exit(exit_code)

    def __check_pid(self):
        active = False
        with self.pid_lock:
            active = self.pid is not None
        if not active:
            print "App process is not active."
            self.session_ws.close()
