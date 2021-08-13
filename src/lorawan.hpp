#pragma once
#include <Arduino.h>

// LMIC
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>

#include "credentials.h"

// LoRa Pins
#define LoRa_RST 23
#define LoRa_CS 18
#define LoRa_DIO0 26
#define LoRa_DIO1 33
#define LoRa_DIO2 32

extern RTC_DATA_ATTR lmic_t RTC_LMIC;
extern osjob_t sendjob;
extern const lmic_pinmap lmic_pins;

void os_getArtEui(u1_t *buf);
void os_getDevEui(u1_t *buf);
void os_getDevKey(u1_t *buf);

extern uint8_t *data;
extern uint8_t size;
extern uint8_t port;
extern bool confirm;
extern bool has_send;

void startup_lorawan();
void lorawan_loop();
void lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm);
bool lorawan_has_send();
void lorawan_sleep(unsigned long sleep_time);

void do_send(osjob_t *j);
void onEvent(ev_t ev);
