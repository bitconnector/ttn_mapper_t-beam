#pragma once
#include <Arduino.h>
#include <axp20x.h>

#define I2C_SDA 21
#define I2C_SCL 22
#define PMU_IRQ 35

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
