#system
from threading import Lock, Condition

class SyncStart:
    def __init__(self):
        self.condition = Condition()
        self.ready = False
        self.status = False

    def notify(self, status):
        self.condition.acquire()
        self.ready = True
        self.status = status
        self.condition.notify()
        self.condition.release()

    def wait(self):
        self.condition.acquire()
        while not self.ready:
            self.condition.wait()
        self.ready = False  
        self.condition.release()