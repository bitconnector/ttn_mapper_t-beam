#!/usr/bin/python3

import sys
import json
import matplotlib.pyplot as plt


STARTINGVOLTAGE = 4.1
COLOURS = ['blue', 'red', 'green', 'yellow']

logs = []
if(len(sys.argv) == 1):
    logs.append(input("path to log: "))
else:
    for path in sys.argv[1:]:
        logs.append(path)


def plotData(data, color):
    a_time = data['time']
    a_bat = data['bat']
    a_perc = data['percentage']
    a_status = data['status']
    a_gps = data['gps']

    idx_start = 0
    while idx_start < len(a_bat):
        if a_bat[idx_start] <= STARTINGVOLTAGE:
            break
        idx_start = idx_start+1

    print("start index:", idx_start)

    a_time_h = [(time-a_time[idx_start]) / 3600 for time in a_time]

    plt.plot(a_time_h[idx_start:], a_bat[idx_start:],
             label='Path', linewidth=2, color=color)


def plotData2(data, color):
    idx_start = 0
    while idx_start < len(data["values"]):
        if float(data["values"][idx_start][1]) <= STARTINGVOLTAGE:
            break
        idx_start = idx_start+1

    print("start index:", idx_start)
    tou = [None, *data["values"][idx_start:], None]
    for prev, curr, nxt in zip(tou, tou[1:], tou[2:]):
        #print(prev, curr, nxt)
        if(prev):
            linex = [int(prev[0]) / 3600, int(curr[0]) / 3600]
            liney = [float(prev[1]), float(curr[1])]
            plt.plot(linex, liney,
                     label='Path', linewidth=2, color=color)


def openLogs(logs):
    for i in range(len(logs)):
        print(logs[i])
        with open(logs[i]) as fp:
            data = json.loads(fp.read())
            if not "ver" in data:
                plotData(data, COLOURS[i])
            elif data["ver"] == 1:
                plotData2(data, COLOURS[i])


openLogs(logs)

plt.title(str(logs)+" bat")
plt.xlabel('time in h')
plt.ylabel('bat in v')
plt.ylim(3.2, 4.2)
# plt.axis('equal')

plt.show()
