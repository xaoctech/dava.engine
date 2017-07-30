#system
import sys

class Logger():
    def __init__(self, log_file = None):
        self.console = sys.stdout
        self.file = open(log_file, "w") if log_file is not None else None
        self.__pos = None
        self.__str = None

    def write(self, msg):
        self.console.write(msg)
        self.console.flush()
        if self.file is not None:
            if msg.startswith("\r"):
                if self.__pos is not None:
                    self.file.seek(self.__pos)
                else:
                    self.__pos = self.file.tell()
            if msg.endswith("\n"):
                self.__pos = None

            self.file.write(msg)    
            self.file.flush()
