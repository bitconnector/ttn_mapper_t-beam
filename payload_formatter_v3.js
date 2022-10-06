function decodeUplink(input) {
    bat_min = 3.4
    bat_max = 4.16

    var ptr = 0;
    var data = {};

    if (input.bytes[ptr] === 255) {
        data.bat = bat_max;
        data.percentage = 100
        batState = "charging"
    }
    else {
        data.bat = input.bytes[ptr]
        data.bat += 250
        data.bat /= 100
        data.percentage = ((data.bat - bat_min) / (bat_max - bat_min)) * 100 | 0
        if (data.percentage > 100) data.percentage = 100;
        batState = "bat: " + data.percentage + "% " + data.bat + "V";
    }
    ptr = ptr + 1;

    if (input.fPort === 1) {
        data.msg = "status: ";
        data.status = input.bytes[ptr]
        if (data.status === 1) data.msg += "button pressed"
        if (data.status === 2) data.msg += "startup"
        if (data.status === 3) data.msg += "enter sleep"
        if (data.status === 4) data.msg += "running"
        ptr = ptr + 1;

        data.msg += "\nGPS: "
        data.gps = input.bytes[ptr]
        if (data.gps === 0) data.msg += "no fix"
        if (data.gps === 1) data.msg += "ok"
        if (data.gps === 2) data.msg += "no movement"
        if (data.gps === 3) data.msg += "geofence"
        ptr = ptr + 1;
    }
    else if (input.fPort === 21) {
        data.msg = "location: ";
        data.lat = ((input.bytes[ptr] << 16) >>> 0) + ((input.bytes[ptr + 1] << 8) >>> 0) + input.bytes[ptr + 2];
        data.lat = (data.lat / 16777215.0 * 180) - 90;
        data.lon = ((input.bytes[ptr + 3] << 16) >>> 0) + ((input.bytes[ptr + 4] << 8) >>> 0) + input.bytes[ptr + 5];
        data.lon = (data.lon / 16777215.0 * 360) - 180;

        var altValue = ((input.bytes[ptr + 6] << 8) >>> 0) + input.bytes[ptr + 7];
        var sign = input.bytes[ptr + 6] & (1 << 7);
        if (sign) {
            data.alt = 0xFFFF0000 | altValue;
        }
        else {
            data.alt = altValue;
        }
        data.hdop = input.bytes[ptr + 8] / 10.0;
        ptr = ptr + 9;

        data.msg += data.lat + " " + data.lon;
    }

    data.msg += "\n" + batState;

    return { data };
}
