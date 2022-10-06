/*
good sources:
https://github.com/JackGruber/ESP32-LMIC-DeepSleep-example
https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
*/
#include <Arduino.h>

#include "gps.hpp"
#include "lorawan.hpp"
#ifdef ESP32
#include "power.hpp"
#endif
#include "config.hpp"

unsigned int TX_INTERVAL = GPS_INTERVAL;
SLEEP_VAR int wakeup_count = 0;

void sendLocation();
void sendStatus(int state, int gps);

#ifdef ESP32
void setSleepTimer(int seconds);
void enterSleep();

void setup()
{
  wakeup_count++;
  pinMode(LED, OUTPUT);
  pinMode(ButtonPin, INPUT);
  Serial.begin(115200);
  digitalWrite(LED, LOW); // LED on
  startup_axp();

  esp_sleep_wakeup_cause_t wakeup_reason =
      esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) // <-------------- timer
  {
    Serial.println(F("Wakeup caused by timer"));
    int gpsStatus = getGPS();

    if (gpsStatus == 0)
      axp_gps(1); // give GPS more volt to get a fix
    else
      axp_gps(2); // turn down voltage for GPS to save energy

    if (wakeup_count % STATUS_INTERVAL == 0)
    {
      Serial.println(F("periodically send state"));
      sendStatus(4, gpsStatus);
    }
    if (gpsStatus == 1)
      sendLocation();
    setSleepTimer(TX_INTERVAL);
  }
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    Serial.println(F("Wakeup caused by axp"));
    axp_gps(1); // turn GPS on
    uint8_t cause = axp_cause();
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
      axp_gps(0); // turn GPS off
      sendStatus(3, 0);
      digitalWrite(LED, HIGH); // turn the LED off
      enterSleep();            // enter sleep without timer
    }
    else if (cause == 3) // <------------------------- battery critical
    {
      Serial.println(F("Battery low shutting down"));
    }
  }
  else // <------------------------------------------------------ reset
  {
    Serial.println(F("Wakeup caused by reset"));
    delay(3000);
    setup_axp();
    end_gps();
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
#else
void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(ButtonPin, INPUT);
  Serial.begin(115200);
  digitalWrite(LED, LOW); // LED on

  Serial.println(F("Startup"));
  delay(3000);
  end_gps();
  startup_lorawan();
  sendStatus(2, 0);
  // setSleepTimer(TX_INTERVAL);
}

void loop()
{
  if (true) // <-------------- timer
  {
    Serial.println(F("Wakeup caused by timer"));
    int gpsStatus = getGPS();

    if (wakeup_count % STATUS_INTERVAL == 0)
    {
      Serial.println(F("periodically send state"));
      sendStatus(4, gpsStatus);
    }
    if (gpsStatus == 1)
      sendLocation();
    // setSleepTimer(TX_INTERVAL);
  }
  else if (false) // <---------- interrupt
  {
    Serial.println(F("Wakeup caused by axp"));
    uint8_t cause = 1;
    if (cause == 1) // <----------------------------- short press power
    {
      Serial.println(F("send status and location"));
      int gpsStatus = getGPS();
      sendStatus(1, gpsStatus);
      if (gpsStatus == 1 || gpsStatus == 2)
        sendLocation();
    }
    else if (cause == 2) // <------------------------- long press power
    {
      Serial.print(F("entering deep sleep for infinity\n"));
      // axp_gps(0); // turn GPS off
      sendStatus(3, 0);
      digitalWrite(LED, HIGH); // turn the LED off
      // enterSleep();            // enter sleep without timer
    }
  }

  pinMode(LED, INPUT_PULLDOWN); // let the LED glim
  wakeup_count++;
}

uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset)
{
  txBuffer[offset] = (getBatteryVoltage() / 10) - 250;
  return offset + 1;
}
#endif

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
  lorawan_send(port, txBuffer, bufferSize, 0, STATUS_SF);
}
