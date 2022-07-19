#pragma once
#include <Arduino.h>

#include <LoraWANactivation.hpp>
#include <LoRa.h>

#include "config.hpp"

extern unsigned char Buffer[235];
extern RTC_DATA_ATTR LoraWANmessage message;
extern RTC_DATA_ATTR bool joined;

void startup_lorawan();
bool lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm, int _sf);
void lorawan_sleep();
void printPackage(char *data, uint16_t size, bool structure = 0);
long getFrequency();
void lora_tx(long frequency, int sf);
void lora_rx(long frequency, int sf);
