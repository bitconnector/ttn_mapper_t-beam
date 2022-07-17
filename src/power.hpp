#pragma once
#include <Arduino.h>
#include <axp20x.h>

#include "config.hpp"

extern AXP20X_Class axp;
extern bool axpIrq;

void setup_axp();
void startup_axp();
void axp_gps(bool state);
void axp_lora(bool state);
void axp_interrupt(void);
uint8_t axp_loop();
void axp_sleep();
uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset);
