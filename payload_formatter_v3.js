function decodeUplink(input) {
    var ptr = 0;
    var data = {};

    data.bat = input.bytes[ptr]
    data.bat += 250
    data.bat /= 100
    ptr = ptr + 1;

    if (input.fPort === 1) {
        data.msg = "no-gps";
    }
    else if (input.fPort === 2) {
        data.msg = "no-movement";
    }
    else if (input.fPort === 3) {
        data.msg = "geofence";
    }
    else {
        data.msg = "ok";
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
    }

    // data.msg += "\nbat:" + data.bat;

    return { data };
}
