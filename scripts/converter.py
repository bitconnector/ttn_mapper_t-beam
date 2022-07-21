#!/usr/bin/python3

import sys
import json
from datetime import datetime


data = {}
data["ver"] = 1


def xls_to_touple(file):
    with open(file) as f:
        data["time"] = int(datetime.strptime(
            f.readline().split(",")[0], "%Y.%m.%d %H:%M:%S").timestamp())
        tou = []
        for line in f:
            d = line.replace(" ", "").replace("\n", "").split(",")[1:]
            print(tuple(d))
            tou.append(tuple(d))
        data["values"] = tou


def list_to_touple(file):
    with open(file) as f:
        obj = json.loads(f.read())
        data['time'] = obj['start']
        tou = []
        i = 0
        while i < len(obj["time"]):
            t = obj['time'][i]
            v = obj['bat'][i]
            p = obj['percentage'][i]
            s = obj['status'][i]
            g = obj['gps'][i]
            tou.append((t, v, p, s, g))
            i = i+1
        data["values"] = tou


xls_to_touple(sys.argv[1])
#list_to_touple(sys.argv[1])


print(data)


try:
    new_file = input("save to: ")
    logfile = open(new_file, "w")
    json.dump(data, logfile)
except:
    pass
