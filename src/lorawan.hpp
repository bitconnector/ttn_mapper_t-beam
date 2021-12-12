#pragma once
#include <Arduino.h>

#include <LoraWANactivation.hpp>
#include <LoRa.h>

#include "credentials.h"

// LoRa Pins
#define LoRa_RST 23
#define LoRa_CS 18
#define LoRa_DIO0 26
#define LoRa_DIO1 33
#define LoRa_DIO2 32

extern unsigned char Buffer[235];
extern RTC_DATA_ATTR LoraWANmessage message;

extern bool has_send;

void startup_lorawan();
void lorawan_loop();
void lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm);
bool lorawan_has_send();
void lorawan_sleep(unsigned long sleep_time);
void printPackage(char *data, uint16_t size, bool structure = 0);
