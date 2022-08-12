#!/usr/bin/python3

import paho.mqtt.client as mqtt
import json
from datetime import datetime
import random
import signal
import sys
import os
import configparser

CONFIGFILE = "config.ini"

MQTT = None

counter = 0
initial_timestamp = 0
a_time = []
a_bat = []
a_percentage = []
a_status = []
a_gps = []


def on_message(client, userdata, message):
    msg = str(message.payload.decode("utf-8"))
    obj = json.loads(msg)
    try:
        log_status(obj)
    except:
        pass


def log_status(obj):
    global counter
    counter = counter+1

    message = obj["uplink_message"]
    payload = message["decoded_payload"]

    f_port = message["f_port"]
    if(f_port != 1):
        return

    device_id = obj["end_device_ids"]["device_id"]
    spreading_factor = message["settings"]["data_rate"]["lora"]["spreading_factor"]
    frequency = message["settings"]["frequency"]
    gw_cnt = len(message["rx_metadata"])
    time = message["received_at"]
    timestamp = float(datetime.strptime(
        time[:26], '%Y-%m-%dT%H:%M:%S.%f').timestamp())
    timenorm = datetime.fromtimestamp(timestamp).strftime("%Y.%m.%d %H:%M:%S")

    try:
        f_cnt = message["f_cnt"]
    except:
        f_cnt = 0

    bat = payload["bat"]
    percentage = payload["percentage"]
    status = payload["status"]
    gps = payload["gps"]

    global initial_timestamp
    if(initial_timestamp == 0):
        initial_timestamp = int(timestamp)

    timestamp_relative = int(timestamp)-initial_timestamp
    print(f'{timenorm},\t{timestamp_relative},\t{bat},\t{percentage},\t{status},\t{gps}')

    global a_time, a_bat, a_percentage, a_status, a_gps
    a_time.append(timestamp_relative)
    a_bat.append(bat)
    a_percentage.append(percentage)
    a_status.append(status)
    a_gps.append(gps)


def signal_handler(sig, frame):
    print('You pressed Ctrl+C!')
    client.disconnect()
    print("disconnected MQTT")

    global initial_timestamp
    if(initial_timestamp == 0):
        sys.exit(0)

    date = datetime.fromtimestamp(
        initial_timestamp).strftime("%Y.%m.%d_%H:%M:%S.log")

    global a_time, a_bat, a_percentage
    data = {"start": initial_timestamp, "time": a_time, "bat": a_bat,
            "percentage": a_percentage, "status": a_status, "gps": a_gps}
    print(data)

    logfile = open(date, "w")
    json.dump(data, logfile)
    print("saved logfile: ", date)

    sys.exit(0)


def on_connect(client, userdata, flags, rc):
    client.subscribe(MQTT["Topic"])


def config_gen(path):
    config = configparser.ConfigParser()
    config['MQTT'] = {'Server': 'eu1.cloud.thethings.network',
                      'Port': '1883',
                      'User': 'application@ttn',
                      'Pass': 'NNSXS.II75FDDP6SGYPEKQ6PRM4Y.JZKGBPDGGJZJXLBV2GVEVFSWAIUVL',
                      'Topic': '#'
                      }
    with open(path, 'w') as configfile:
        config.write(configfile)


def config_read(path):
    config = configparser.ConfigParser()
    config.read(path)
    global MQTT
    MQTT = config['MQTT']


if __name__ == "__main__":
    if not os.path.exists(CONFIGFILE):
        config_gen(CONFIGFILE)
        print("Config file generated. Please modify", CONFIGFILE)
        sys.exit(0)

    config_read(CONFIGFILE)

    client = mqtt.Client(f'python-mqtt-{random.randint(0, 1000)}')
    client.username_pw_set(MQTT["User"], MQTT["Pass"])
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT["Server"], int(MQTT["Port"]))

    signal.signal(signal.SIGINT, signal_handler)

    print('timestamp,\t\ttime,\tbat,\tperc,\tstatus,\tgps')
    client.loop_forever()
