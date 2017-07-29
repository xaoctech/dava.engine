#system
import sys
from json import loads
from datetime import datetime

class LogParser:
    def __init__(self, levels = [], providers = [], show_timestamp = True):
        self.levels = levels
        self.providers = providers
        self.show_timestamp = show_timestamp
    
    def __call__(self, msg):
        if msg is None:
            return
        events = loads(msg).get("Events")
        if events is not None:
            for event in events:
                level = event["Level"]
                provider = event["ProviderName"]
                if level in self.levels and provider in self.providers:
                    if self.show_timestamp:
                        # Convert webkit timestamp to unix timestamp
                        unix_timestamp = event["Timestamp"] / 1e7 - 11644473600
                        sys.stdout.write(datetime.fromtimestamp(unix_timestamp).\
                                         strftime("%Y-%m-%d %H:%M:%S.%f") + " ")

                    string_message = event["StringMessage"]
                    if string_message.endswith("\n"):
                        string_message = string_message[:-1]
                    print string_message