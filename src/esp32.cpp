/*
good sources:
https://github.com/JackGruber/ESP32-LMIC-DeepSleep-example
https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/
*/
#include <Arduino.h>

#include "gps.hpp"
#include "lorawan.hpp"
#include "power.hpp"
#include "display.hpp"
#include "config.hpp"

unsigned int TX_INTERVAL = GPS_INTERVAL;
SLEEP_VAR int wakeup_count = 0;

void sendLocation();
void sendStatus(int state, int gps);

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

  // Serial.println(axp.getBattVoltage());
  Serial.println(PMU->getBattVoltage());

  esp_sleep_wakeup_cause_t wakeup_reason =
      esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER) // <-------------- timer
  {
    Serial.println(F("Wakeup caused by timer"));
    setup_gps();
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
      setup_gps();
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

    while(0){
      setupDisplay();
      delay(10000);
    }

    unsigned long t = millis();
    setup_gps();
    while (0)
    {
      axp_gps(1);
      Serial.println("activate");
      axp_print();
      t = millis() + 300000;
      t = millis() + 120000;
      //t = millis() + 10000;
      while (t > millis())
        gps_loop();
      axp_gps(0);
      Serial.println("deactivate");
      axp_print();
      t = millis() + 20000;
      while (t > millis())
        gps_loop();
    }

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
