#pragma once
#include <Arduino.h>

#include <LoraWANactivation.hpp>
#include "config.hpp"

#ifdef CUBECELL
#include "LoRaWan_APP.h"
extern int16_t Rssi;
extern bool radioBusy;
extern RadioEvents_t RadioEvents;
#else
#include <LoRa.h>
#endif

extern unsigned char Buffer[235];
extern SLEEP_VAR LoraWANmessage message;
extern SLEEP_VAR bool joined;

void startup_lorawan();
bool lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm, int _sf);
void lorawan_sleep();
void printPackage(char *data, uint16_t size, bool structure = 0);
long getFrequency();
void lora_tx(long frequency, int sf, uint8_t *data, uint8_t size);
void lora_rx(long frequency, int sf);
bool lora_busy();

#ifdef CUBECELL
void OnTxDone();
void OnTxTimeout();
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
#endif
