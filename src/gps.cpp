#include "gps.hpp"

TinyGPSPlus gps;
HardwareSerial serialGPS(1);

void setup_gps()
{
    serialGPS.begin(9600, SERIAL_8N1, 34, 12);
}

void gps_loop()
{
    unsigned long lock = millis() + 2;
    while (serialGPS.available() > 0 && millis() < lock)
    {
        gps.encode(serialGPS.read());
    }
}

bool gps_valid()
{
    if (gps.location.age() < 10000)
        return true;
    else
        return false;
}

RTC_DATA_ATTR double last_lat;
RTC_DATA_ATTR double last_lng;
bool gps_moved(int meter)
{
    if (gps.distanceBetween(gps.location.lat(), gps.location.lng(), last_lat, last_lng) < meter)
        return false;
    else
    {
        last_lat = gps.location.lat();
        last_lng = gps.location.lng();
        return true;
    }
}

uint8_t gps_geo()
{
    for (int i = 0; i < (sizeof(geofence) / sizeof(Geofence)); i++)
    {
        if (gps.distanceBetween(gps.location.lat(), gps.location.lng(), geofence[i].lat, geofence[i].lng) < geofence[i].radius)
        {
            return i + 1;
        }
    }
    return 0;
}

uint8_t location_bin(uint8_t *txBuffer, uint8_t offset)
{
    uint32_t LatitudeBinary, LongitudeBinary;
    uint16_t altitudeGps;
    uint8_t hdopGps;

    LatitudeBinary = ((gps.location.lat() + 90) / 180) * 16777215;
    LongitudeBinary = ((gps.location.lng() + 180) / 360) * 16777215;

    txBuffer[offset + 0] = (LatitudeBinary >> 16) & 0xFF;
    txBuffer[offset + 1] = (LatitudeBinary >> 8) & 0xFF;
    txBuffer[offset + 2] = LatitudeBinary & 0xFF;

    txBuffer[offset + 3] = (LongitudeBinary >> 16) & 0xFF;
    txBuffer[offset + 4] = (LongitudeBinary >> 8) & 0xFF;
    txBuffer[offset + 5] = LongitudeBinary & 0xFF;

    altitudeGps = gps.altitude.meters();
    txBuffer[offset + 6] = (altitudeGps >> 8) & 0xFF;
    txBuffer[offset + 7] = altitudeGps & 0xFF;

    if (gps.hdop.isValid())
        hdopGps = gps.hdop.hdop() * 10;
    else
        hdopGps = 144;
    txBuffer[offset + 8] = hdopGps & 0xFF;

    return offset + 9;
}
