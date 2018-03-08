import sys, os
import argparse
from pprint import pprint
from collections import defaultdict

def makeNames(cls):
    return {v : k for k, v in cls.__dict__.iteritems() if isinstance(v, int)}

class PING_COMPENSATION_MECHANICS:
    NONE = 0
    REWIND_ENEMY = 1
    PREDICT_ENEMY = 2

class VIEWS:
    OBSERVER = 0
    SHOOTER = 1
    TARGET = 2
    SERVER = 3

    RANGE = (SHOOTER, SERVER, TARGET, OBSERVER) # sorted in order corresponding to input propagation, do not change it
    ROLES = (OBSERVER, SHOOTER, TARGET)

VIEW_NAMES = makeNames(VIEWS)

class SCENARIOS:
    STILL = 0
    SLIDING_TARGET = 1
    SLIDING_SHOOTER = 2
    SLIDING_BOTH = 3
    WAGGING_TARGET = 4
    WAGGING_SHOOTER = 5
    WAGGING_BOTH = 6
    DODGING_TARGET = 7

    REQUIRED = {
        PING_COMPENSATION_MECHANICS.NONE : {
            VIEWS.OBSERVER : {STILL,                 SLIDING_SHOOTER,                 WAGGING_SHOOTER                },
            VIEWS.SHOOTER :  {STILL, SLIDING_TARGET, SLIDING_SHOOTER, WAGGING_TARGET, WAGGING_SHOOTER                },
            VIEWS.TARGET :   {STILL,                 SLIDING_SHOOTER,                 WAGGING_SHOOTER                },
            VIEWS.SERVER :   {STILL,                 SLIDING_SHOOTER,                 WAGGING_SHOOTER                },
        },
        PING_COMPENSATION_MECHANICS.REWIND_ENEMY : {
            VIEWS.OBSERVER : {STILL,                 SLIDING_SHOOTER,                 WAGGING_SHOOTER                },
            VIEWS.SHOOTER :  {STILL, SLIDING_TARGET, SLIDING_SHOOTER, WAGGING_TARGET, WAGGING_SHOOTER                },
            VIEWS.TARGET :   {STILL,                 SLIDING_SHOOTER,                 WAGGING_SHOOTER                },
            VIEWS.SERVER :   {STILL, SLIDING_TARGET, SLIDING_SHOOTER, WAGGING_TARGET, WAGGING_SHOOTER                },
        },
        PING_COMPENSATION_MECHANICS.PREDICT_ENEMY : {
            VIEWS.OBSERVER : {STILL                                                                                  },
            VIEWS.SHOOTER :  {STILL, SLIDING_TARGET, SLIDING_SHOOTER,                 WAGGING_SHOOTER, DODGING_TARGET},
            VIEWS.TARGET :   {STILL                                                                                  },
            VIEWS.SERVER :   {STILL, SLIDING_TARGET, SLIDING_SHOOTER,                 WAGGING_SHOOTER                },
        }
    }

    UNEXPECTED = {
        PING_COMPENSATION_MECHANICS.NONE : {
            VIEWS.OBSERVER : {       SLIDING_TARGET,                                                   DODGING_TARGET},
            VIEWS.SHOOTER :  {                                                                         DODGING_TARGET},
            VIEWS.TARGET :   {       SLIDING_TARGET,                                                   DODGING_TARGET},
            VIEWS.SERVER :   {       SLIDING_TARGET,                                                   DODGING_TARGET},
        },
        PING_COMPENSATION_MECHANICS.REWIND_ENEMY : {
            VIEWS.OBSERVER : {       SLIDING_TARGET,                                                   DODGING_TARGET},
            VIEWS.SHOOTER :  {                                                                         DODGING_TARGET},
            VIEWS.TARGET :   {       SLIDING_TARGET,                                                   DODGING_TARGET},
            VIEWS.SERVER :   {                                                                         DODGING_TARGET},
    },
        PING_COMPENSATION_MECHANICS.PREDICT_ENEMY : {
            VIEWS.OBSERVER : {       SLIDING_TARGET, SLIDING_SHOOTER,                                                },
            VIEWS.SHOOTER :  set(),
            VIEWS.TARGET :   {       SLIDING_TARGET, SLIDING_SHOOTER,                                                },
            VIEWS.SERVER :   {                                                                         DODGING_TARGET},
        }
    }


SCENARIO_NAMES = makeNames(SCENARIOS)


def parseFile(path, requiredView, strikes, damages):
    print 'reading %s ...' % path
    match = False
    with open(path) as f:
        for logLine in f:
            words = logLine.strip().split(' ')
            frameID = int(words[4])
            sync = int(words[6])
            scenario = getattr(SCENARIOS, words[8], None)
            view = getattr(VIEWS, words[10])
            shooterRole, shooterID = getattr(VIEWS, words[12], None), int(words[13])
            targetRole, targetID = getattr(VIEWS, words[15], None), int(words[16])

            if view != requiredView:
                assert not match, "'%s' wrong view: %s expected, %s gotten" % (path, VIEW_NAMES[requiredView], VIEW_NAMES[view])
                return False

            match = True

            if view != VIEWS.SERVER:
                assert shooterRole == VIEWS.SHOOTER, "'%s': wrong shooter role: SHOOTER expected, %s gotten" % (path, VIEW_NAMES[shooterRole])
                assert targetRole == VIEWS.TARGET, "'%s': wrong target role: TARGET expected, %s gotten" % (path, VIEW_NAMES[targetRole])

            # shooter client shares 'frameID' with server and 'sync' with other clients
            if view == VIEWS.SHOOTER:
                parseFile.syncToFrameID[sync] = frameID
                parseFile.frameIDToSync[frameID] = sync
                fdiff = 0
            elif view == VIEWS.SERVER:
                sync = parseFile.frameIDToSync.get(frameID)
                if sync is not None:
                    scenario = strikes[sync][VIEWS.SHOOTER][0]
                else:
                    damages.setdefault(frameID, {})[view] = (scenario, shooterID, targetID)
                    continue
                fdiff = 0
            else:
                fdiff = frameID - parseFile.syncToFrameID[sync]


            strikesViews = strikes.setdefault(sync, {})
            for (_, prevShooterID, prevTargetID, _) in strikesViews.itervalues():
                assert prevShooterID == shooterID, "'%s': strike is not consistent: shooter id disagreement" % path
                assert prevTargetID == targetID, "'%s': strike is not consistent: target id disagreement" % path

            strikesViews[view] = (scenario, shooterID, targetID, fdiff)

    return True

parseFile.syncToFrameID = {}
parseFile.frameIDToSync = {}

def expandPath(path):
    if not os.path.isabs(path):
        if path[0] == '~':
            path = os.path.expanduser(path)
        else:
            path = os.path.join(os.path.split(os.path.abspath(__file__))[0], path)
    return os.path.normpath(path)

def analyzeLogs(args):
    # {
    #     sync : {
    #         view : (scenario, shooterID, targetID, fdiff),
    #     },
    # }
    coupledStrikes = {}

    # {
    #     frameID : {
    #         view : (scenario, shooterID, targetID, fdiff),
    #     },
    # }
    looseDamages = {}

    # read log files in order corresponding to input propagation
    logsDir = args.logs_dir
    files = [f for f in os.listdir(logsDir) if os.path.isfile(os.path.join(logsDir, f)) and f.endswith('.log')]
    missingViews = list(VIEWS.RANGE)
    for view in VIEWS.RANGE:
        for f in files:
            if parseFile(os.path.join(logsDir, f), view, coupledStrikes, looseDamages):
                missingViews.remove(view)
                files.remove(f)

    assert not missingViews, '%s views missing' % ', '.join(VIEW_NAMES[v] for v in missingViews)

    # process data read
    strikesSum = {view : 0 for view in VIEWS.RANGE}
    scenariosWithStrikes = {view : defaultdict(int) for view in VIEWS.RANGE}
    fdiffSum = {role : (0, 0) for role in VIEWS.ROLES}

    for sync, views in coupledStrikes.iteritems():
        for view, (scenario, shooterID, targetID, fdiff) in views.iteritems():
            strikesSum[view] += 1
            scenariosWithStrikes[view][scenario] += 1
            if view in VIEWS.ROLES:
                sum, cnt = fdiffSum[view]
                fdiffSum[view] = (sum + fdiff, cnt + 1)

    trueStrikesCnt = strikesSum[VIEWS.SERVER]

    # print some stats
    print '\nstrikes seen'
    print '------------'
    for view, cnt in strikesSum.iteritems():
        print '%10s\t%.1f%% (%d)' % (VIEW_NAMES[view], 100. * float(cnt) / trueStrikesCnt, cnt)

    print '\nscenarios with strikes'
    print '----------------------'
    for view, scenariosCnt in scenariosWithStrikes.iteritems():
        print '%10s\t%s' % (VIEW_NAMES[view], ', '.join("%s (%d)" % (SCENARIO_NAMES[s], cnt) for s, cnt in scenariosCnt.iteritems()))

    print '\navg fdiff'
    print '---------'
    for role, (sum, cnt) in fdiffSum.iteritems():
        print '%10s\t%.2f' % (VIEW_NAMES[role], float(sum)/cnt)

    print ''

    # collect fails
    print '\nPerforming consistency check for %s ping compensation mechanic...' % args.ping_comp
    fails = []

    if looseDamages:
        fails.append('loose damages registered: %.1f%% of all damages' %
                     (100. * len(looseDamages) / (len(looseDamages) + trueStrikesCnt)))

    pingCompensation = getattr(PING_COMPENSATION_MECHANICS, args.ping_comp)
    requiredScenarios = SCENARIOS.REQUIRED[pingCompensation]
    unexpectedScenarios = SCENARIOS.UNEXPECTED[pingCompensation]

    for view in VIEWS.RANGE:
        scenarios = scenariosWithStrikes[view]

        missing = requiredScenarios[view].difference(scenarios)
        if missing:
            fails.append("%s's view is incorrect, missing scenarios: %s" %
                         (VIEW_NAMES[view], ', '.join(SCENARIO_NAMES[s] for s in missing)))

        unexpected = unexpectedScenarios[view].intersection(scenarios)
        if unexpected:
            fails.append("%s's view is incorrect, unexpected scenarios: %s" %
                         (VIEW_NAMES[view], ', '.join(SCENARIO_NAMES[s] for s in unexpected)))

        # check that amount of required on server is not very different from registered on client
        requiredScenariosOnServer = requiredScenarios[VIEWS.SERVER]
        strikesOnServer = scenariosWithStrikes[VIEWS.SERVER]
        if view in VIEWS.ROLES:
            for scenario, cnt in scenarios.iteritems():
                if scenario not in requiredScenarios[view].intersection(requiredScenariosOnServer):
                    continue

                serverCnt = strikesOnServer.get(scenario)
                if serverCnt is None:
                    fails.append("%s's view is incorrect: strikes seen in scenario %s missing on server" %
                                 (VIEW_NAMES[view], SCENARIO_NAMES[scenario]))
                    continue

                deviation = args.max_deviation / 100. * serverCnt
                if not (serverCnt - deviation <= cnt <= serverCnt + deviation):
                    fails.append("%s's view is incorrect: %.1f%% of server strikes were seen in scenario %s (acceptable deviation is %.1f%%)" %
                                 (VIEW_NAMES[view], 100. * float(cnt) / serverCnt, SCENARIO_NAMES[scenario], args.max_deviation))

    if not fails:
        print 'OK'

    return fails

if __name__ == '__main__':
    description = '''Inspects logs for comparing strikes correctness (view-damage correspondence) 
                     on server, shooter client, target client, and observer client side.
                     Ensuring correctness on all sides at once is impossible due to non-zero RTT. 
                     Depending on gameplay we may be interested in strike correctness
                     in one of above mentioned views.'''
    parser = argparse.ArgumentParser(description=description)

    parser.add_argument('--logs-dir', help='path to directory with logs', type=str, default='logs')
    parser.add_argument('--ping-comp', help='ping compensation mechanic', type=str, default='REWIND_ENEMY')
    parser.add_argument('--max-deviation', help='accepted deviation in strikes count on client comparing to server, percent',
                        type=float, default=50.0)

    args = parser.parse_args()

    fails = analyzeLogs(args)

    if len(fails) > 0:
        for msg in fails:
            print >>sys.stderr, '[FAIL] %s' % msg
        sys.exit(1)
