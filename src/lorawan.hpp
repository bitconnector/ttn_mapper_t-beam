#pragma once
#include <Arduino.h>

#include <LoraWANactivation.hpp>
#include <LoRa.h>

#include "config.hpp"

extern unsigned char Buffer[235];
extern RTC_DATA_ATTR LoraWANmessage message;
extern RTC_DATA_ATTR bool joined;

extern bool has_send;

void startup_lorawan();
void lorawan_loop();
void lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm);
bool lorawan_has_send();
void lorawan_sleep(unsigned long sleep_time);
void printPackage(char *data, uint16_t size, bool structure = 0);
long getFrequency();
