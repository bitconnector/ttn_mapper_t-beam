#pragma once
#ifdef AXP
#include <Arduino.h>
#include <XPowersLib.h>

#include "config.hpp"

extern XPowersLibInterface *PMU;
extern bool axpIrq;

void setup_axp();
void startup_axp();
void axp_gps(uint8_t state);
void axp_lora(bool state);
uint8_t axp_cause();
void axp_sleep();
uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset);
#endif
