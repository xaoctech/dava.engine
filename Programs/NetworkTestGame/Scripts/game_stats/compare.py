import argparse
import math
import sys
from collections import namedtuple
from itertools import izip

from game_stats_parser import *

INTERPOLATION_PERIOD = 1000


def comparePositions(pos1, pos2):
    posDiff = math.sqrt(sum((cord1 - cord2) * (cord1 - cord2) for cord1, cord2 in izip(pos1, pos2)))
    return posDiff

def compareHealth(health1, health2):
    return abs(health1 - health2)

def compare(paramLog, paramRefLog, interpolator, comparer):
    endTime = max(paramLog[-1].timestamp, paramRefLog[-1].timestamp)

    diff = 0.0
    interpLog = tuple(interpolateLog(paramLog, INTERPOLATION_PERIOD, endTime, interpolator))
    interpRefLog = interpolateLog(paramRefLog, INTERPOLATION_PERIOD, endTime, interpolator)
    for value, refValue in izip(interpLog, interpRefLog):
        diff += comparer(value, refValue)

    return diff / len(interpLog)

def checkMetrics(log, refLog, args):
    sumPosDiff = 0.0
    sumHealthDiff = 0.0
    for token, paramRefLog in refLog.iteritems():
        paramLog = log[token]
        posDiff = compare(paramLog['position'], paramRefLog['position'], interpolatePosition, comparePositions)
        sumPosDiff += posDiff
        healthDiff = compare(paramLog['health'], paramRefLog['health'], interpolateHealth, compareHealth)
        sumHealthDiff += healthDiff

    avgPosDiff = sumPosDiff / len(refLog)
    avgHealthDiff = sumHealthDiff / len(refLog)
    print 'Game stats compare: pos_diff=%f, health_diff=%f' % (avgPosDiff, avgHealthDiff)

    fails = []
    if avgPosDiff > args.max_pos_diff:
        fails.append('Position difference exceeded the limit: %f > %f' % (avgPosDiff, args.max_pos_diff))
    if avgHealthDiff > args.max_health_diff:
        fails.append('Health difference exceeded the limit: %f > %f' % (avgHealthDiff, args.max_health_diff))

    return fails


if __name__ == '__main__':
    description = "Compares game stats logfile against reference log"
    argParser = argparse.ArgumentParser(description=description)

    argParser.add_argument('path1', help="first logfile to evaluate", type=str)
    argParser.add_argument('path2', help="second logfile to evaluate", type=str)
    argParser.add_argument('--max-pos-diff', type=float, default=100.0,
                            help='max position difference in meters (default=100)')
    argParser.add_argument('--max-health-diff', type=float, default=1.5,
                            help='max health difference (default=1.5)')

    args = argParser.parse_args()

    refLog = parseLogFile(args.path1)
    log = parseLogFile(args.path2)
    
    fails = checkMetrics(log, refLog, args)
    if len(fails) > 0:
        for msg in fails:
            print >>sys.stderr, '[FAIL]: %s' % msg
        sys.exit(1)

