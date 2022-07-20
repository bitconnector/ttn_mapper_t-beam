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
    plt.scatter(a_time_h[idx_start:], a_bat[idx_start:],
                label='Points', linewidth=4, color=color)


def openLogs(logs):
    for i in range(len(logs)):
        print(logs[i])
        with open(logs[i]) as fp:
            data = json.loads(fp.read())
        plotData(data, COLOURS[i])


openLogs(logs)

plt.title(str(logs)+" bat")
plt.xlabel('time in h')
plt.ylabel('bat in v')
plt.ylim(3.5, 4.2)
# plt.axis('equal')

plt.show()
