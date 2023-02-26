#include "gps.hpp"

TinyGPSPlus gps;
#ifndef CUBECELL
HardwareSerial serialGPS(1);
#else
Air530Class serialGPS;
#endif

void setup_gps()
{
#ifndef CUBECELL
    serialGPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
#else
    serialGPS.begin();
#endif
}

void end_gps()
{
    serialGPS.end();

#ifdef ESP32
    digitalWrite(GPS_TX, HIGH);
    gpio_hold_en((gpio_num_t)GPS_TX);
#endif
}

void gps_loop()
{
    unsigned long lock = millis() + 2;
    while (serialGPS.available() > 0 && millis() < lock)
    {
        char c = serialGPS.read();
        gps.encode(c);
        Serial.print(c);
    }
}

int getGPS()
{
    Serial.println("GPS-loop");
    unsigned long time = millis() + 1200;
    while (!gps_valid() && time > millis())
        gps_loop();

    Serial.print("GPS-end: ");

    if (!gps_valid()) // no GPS
    {
        Serial.println("no fix");
        return 0;
    }
    if (gps_geo()) // Geofence
    {
        Serial.println("geofence");
        return 3;
    }
    if (!gps_moved(GPS_MOVE_DIST)) // no movement
    {
        Serial.println("no movement");
        return 2;
    }

    Serial.println("ok");
    return 1; // GPS ok
}

bool gps_valid()
{
    if (gps.location.isValid() && gps.hdop.isValid() && gps.altitude.isValid())
        return true;
    else
        return false;
}

SLEEP_VAR double last_lat;
SLEEP_VAR double last_lng;
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
