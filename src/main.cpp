/*
good sources:
https://github.com/JackGruber/ESP32-LMIC-DeepSleep-example
https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
*/
#include <Arduino.h>

#include "gps.hpp"
#include "lorawan.hpp"
#include "power.hpp"
#include "config.hpp"

unsigned int TX_INTERVAL = GPS_INTERVAL;

int getGPS();
void sendLocation();
void sendStatus(int state, int gps);
void setSleepTimer(int seconds);
void enterSleep();

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(ButtonPin, INPUT);
  Serial.begin(115200);
  digitalWrite(LED, LOW); // LED on

  esp_sleep_wakeup_cause_t wakeup_reason =
      esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) // <-------------- timer
  {
    Serial.println(F("Wakeup caused by timer"));
    if (getGPS() == 1)
      sendLocation();
    setSleepTimer(TX_INTERVAL);
  }
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    Serial.println(F("Wakeup caused by axp"));
    startup_axp();
    axp_interrupt();
    axp_gps(1); // turn GPS on
    uint8_t cause = axp_loop();
    if (cause == 1) // <----------------------------- short press power
    {
      Serial.println(F("send status and location"));
      int gpsStatus = getGPS();
      sendStatus(1, gpsStatus);
      if (gpsStatus == 1 || gpsStatus == 2)
        sendLocation();
      setSleepTimer(TX_INTERVAL);
    }
    else if (cause == 2) // <------------------------- long press power
    {
      Serial.print(F("entering deep sleep for infinity\n"));
      axp_gps(0);              // turn GPS off
      sendStatus(3, 0);
      digitalWrite(LED, HIGH); // turn the LED off
      enterSleep();            // enter sleep without timer
    }
  }
  else // <------------------------------------------------------ reset
  {
    Serial.println(F("Wakeup caused by reset"));
    setup_axp();
    startup_lorawan();
    sendStatus(2, 0);
    setSleepTimer(TX_INTERVAL);
  }

  pinMode(LED, INPUT_PULLDOWN); // let the LED glim
  enterSleep();
}

void loop()
{
}

int getGPS()
{
  setup_gps();

  unsigned long time = millis() + 1200;
  while (!gps_valid() && time > millis())
    gps_loop();

  if (!gps_valid()) // no GPS
    return 0;

  if (gps_geo()) // Geofence
    return 3;

  if (!gps_moved(GPS_MOVE_DIST)) // no movement
    return 2;

  return 1; // GPS ok
}

void sendLocation()
{
  startup_lorawan();
  uint8_t txBuffer[14];
  uint8_t bufferSize = 0;
  uint8_t port = 21;
  bufferSize = vbatt_bin(txBuffer, bufferSize); // get battery level
  bufferSize = location_bin(txBuffer, bufferSize);
  lorawan_send(port, txBuffer, bufferSize, 0, LORAWAN_DEFAULT_SF);
}

void sendStatus(int state, int gps)
{
  startup_lorawan();
  uint8_t txBuffer[14];
  uint8_t bufferSize = 0;
  uint8_t port = 1;
  bufferSize = vbatt_bin(txBuffer, bufferSize); // get battery level
  txBuffer[bufferSize] = state;
  bufferSize++;
  txBuffer[bufferSize] = gps;
  bufferSize++;
  lorawan_send(port, txBuffer, bufferSize, 0, 10);
}

void setSleepTimer(int seconds)
{
  esp_sleep_enable_timer_wakeup(seconds * 1000000);
}

void enterSleep()
{
  lorawan_sleep();
  axp_sleep();
  Serial.flush();
  esp_deep_sleep_start();
}
