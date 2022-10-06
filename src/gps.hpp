#pragma once
#include <Arduino.h>
#include <TinyGPS++.h>

#include "soc/rtc_cntl_reg.h"
#include "soc/rtc.h"
#include "driver/rtc_io.h"

#include "config.hpp"

extern TinyGPSPlus gps;
extern HardwareSerial serialGPS;

extern RTC_DATA_ATTR double last_lat;
extern RTC_DATA_ATTR double last_lng;

void setup_gps();
void end_gps();
void gps_loop();
int getGPS();
bool gps_valid();
bool gps_moved(int meter);
uint8_t gps_geo();
uint8_t location_bin(uint8_t *txBuffer, uint8_t offset);
