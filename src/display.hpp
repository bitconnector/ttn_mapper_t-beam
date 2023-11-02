#pragma once

#include <Arduino.h>
#include "config.hpp"
#include <Wire.h>

#include <U8g2lib.h>

extern U8G2_SSD1306_128X64_NONAME_F_HW_I2C *u8g2;

#ifndef OLED_WIRE_PORT
#define OLED_WIRE_PORT Wire
#endif

#define I2C_SDA 21
#define I2C_SCL 22

void setupDisplay();
