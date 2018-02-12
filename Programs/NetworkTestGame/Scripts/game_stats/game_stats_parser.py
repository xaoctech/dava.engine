import math
from collections import namedtuple
from itertools import izip

LogRecord = namedtuple('LogRecord', ['timestamp', 'value'])


def parseLogFile(path, timestampOffset=0):
    f = open(path, "r")
    logLines = f.readlines()
    f.close()

    # { token : {'position': [posInfo, ...], 'health': [healthInfo, ...]} }, where
    #   posInfo: (timestamp, (x, y, z))
    #   healthInfo: (timestamp, healthValue)
    log = {}

    baseTimestamp = int(logLines[0].strip().split(' ')[4]) + timestampOffset

    for line in logLines:
        words = line.strip().split(' ')
        timestamp = int(words[4]) - baseTimestamp
        token = words[6]
        recordType = words[7]

        tokenLog = log.setdefault(token, {'position': [], 'health': []})
        if recordType == 'Position':
            tokenLog['position'].append(LogRecord(timestamp, map(float, words[8:])))
        elif recordType == 'Health':
            tokenLog['health'].append(LogRecord(timestamp, int(words[8])))

    return log

def interpolatePosition(pos1, pos2, t):
    return tuple(cord1 * (1.0 - t) + cord2 * t for cord1, cord2 in izip(pos1, pos2))

def interpolateHealth(health1, health2, t):
    return health1 if t < 1.0 else health2

def interpolateLog(paramLog, period, duration, interpolator):
    logIter = iter(paramLog)
    prev = logIter.next()
    cur = prev

    for i in xrange(int(duration / period) + 2):
        tm = i * period

        if tm < prev.timestamp:
            yield prev.value
            continue
        elif tm >= cur.timestamp:
            prev = cur
            try:
                cur = logIter.next()
            except StopIteration:
                yield cur.value
                continue

        t = float(tm - prev.timestamp) / (cur.timestamp - prev.timestamp)
        yield interpolator(prev.value, cur.value, t)
