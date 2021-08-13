#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>

#include "locations.h"

extern TinyGPSPlus gps;
extern HardwareSerial serialGPS;

extern RTC_DATA_ATTR double last_lat;
extern RTC_DATA_ATTR double last_lng;

void setup_gps();
void gps_loop();
bool gps_valid();
bool gps_moved(int meter);
uint8_t gps_geo();
uint8_t location_bin(uint8_t *txBuffer, uint8_t offset);
