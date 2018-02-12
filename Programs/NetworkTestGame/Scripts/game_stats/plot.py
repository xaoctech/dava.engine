import argparse
import numpy as np
import matplotlib
matplotlib.use('TKAgg')
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from game_stats_parser import *


ANINATION_FRAME_PERIOD = 1000
ANIMATION_STEPS = 60 * 10
MAP_SIZE = 3800


def plot(x, y, refX, refY):
    x, y, refX, refY  = ((1e6,) + l for l in (x, y, refX, refY))

    fig, ax = plt.subplots()
    fig.set_size_inches(12, 12, forward=True)

    halfMapSize = MAP_SIZE / 2
    plt.axis([-halfMapSize, halfMapSize, -halfMapSize, halfMapSize])

    timeText = plt.text(1400.0, 1400.0, '', fontsize=15)

    refAnimlist = plt.plot(refX[0], refY[0], 'bo', markersize=10)
    animlist = plt.plot(x[0], y[0], 'ro', markersize=8)

    def animate(i):
        bkg = plt.plot([0], [0], 'wo', markersize=1000)
        timeText.set_text(str(i) + ' s')
        refAnimlist = plt.plot(refX[i], refY[i], 'bo', markersize=10)
        animlist = plt.plot(x[i], y[i], 'ro', markersize=8)
        return bkg + refAnimlist + animlist + [timeText]

    def init():
        timeText.set_text('')
        return timeText,

    ani = animation.FuncAnimation(fig, animate, np.arange(0, ANIMATION_STEPS), init_func=init, interval=30, blit=True, repeat=True)
    plt.show()


def preparePlotData(log):
    interpLog = list(
        list(interpolateLog(
            tokenLog['position'],
            ANINATION_FRAME_PERIOD,
            ANIMATION_STEPS * ANINATION_FRAME_PERIOD,
            interpolatePosition))
        for tokenLog in log.itervalues())

    posBySteps = zip(*interpLog)
    
    cords = list(zip(*e) for e in posBySteps)
    x, y, _ = zip(*cords)
    return x, y


if __name__ == '__main__':
    description = "Plot game stats"
    argParser = argparse.ArgumentParser(description=description)

    argParser.add_argument('path1', help="first logfile to plot", type=str)
    argParser.add_argument('path2', help="second logfile to plot", type=str)

    args = argParser.parse_args()

    refLog = parseLogFile(args.path1)
    log = parseLogFile(args.path2)

    x, y = preparePlotData(log)
    refX, refY = preparePlotData(refLog)

    plot(x, y, refX, refY)
