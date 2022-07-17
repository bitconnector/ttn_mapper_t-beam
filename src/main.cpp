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

unsigned int TX_INTERVAL = 20;

void sendLocation(bool force = 0)
{
  startup_axp();
  setup_gps();
  startup_lorawan();

  // wait for gps
  Serial.println(F("wait for GPS"));
  unsigned long time = millis() + 1200;
  while (!gps_valid() && time > millis())
  {
    gps_loop();
    lorawan_loop();
    axp_loop();
  }
  if (gps_valid())
    Serial.printf("got GPS left %lims\n", (time - millis()));
  else
    Serial.println("no GPS");

  // send data
  Serial.println(F("send Data"));
  uint8_t txBuffer[14];
  uint8_t bufferSize = 0;
  uint8_t port = 1;
  bufferSize = vbatt_bin(txBuffer, bufferSize); // get battery level
  if (gps_valid())
  {
    if (gps_geo())
    {
      port = 3;
      TX_INTERVAL = 120;
    }
    else if (!gps_moved(25) && !force)
    {
      port = 2;
    }
    else
    {
      bufferSize = location_bin(txBuffer, bufferSize);
      port = 21;
    }
  }
  else
  {
    TX_INTERVAL = 90;
  }
  lorawan_send(port, txBuffer, bufferSize, 0);

  // wait for end of transmission
  Serial.println(F("wait for the end of transmission"));
  while (!lorawan_has_send())
  {
    gps_loop();
    lorawan_loop();
    axp_loop();
  }

  // enter deep sleep
  Serial.print(F("entering deep sleep for "));
  Serial.print(TX_INTERVAL);
  Serial.println(F("sec"));
  lorawan_sleep(TX_INTERVAL * 1000);
  axp_sleep();
  Serial.flush();
  esp_sleep_enable_timer_wakeup(TX_INTERVAL * 1000000);
}

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(ButtonPin, INPUT);
  Serial.begin(115200);
  digitalWrite(LED, LOW);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  if (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER)
  {
    Serial.println(F("Wakeup caused by timer"));
    sendLocation();
  }
  else if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
  {
    Serial.println(F("Wakeup caused by axp"));
    startup_axp();
    axp_interrupt();
    axp_gps(1);
    uint8_t cause = axp_loop();
    if (cause == 1)
      sendLocation(1);
    Serial.println(F("going to sleep again"));
    if (cause != 2)
    {
      Serial.print(F("entering deep sleep for "));
      Serial.print(TX_INTERVAL);
      Serial.println(F("sec"));
      esp_sleep_enable_timer_wakeup(TX_INTERVAL * 1000000);
    }
    else
    {
      Serial.print(F("entering deep sleep for infinity\n"));
      esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP_IRQ, 0); // axp
      axp_gps(0);
      digitalWrite(LED, HIGH); // turn the LED off
      axp_sleep();
      Serial.flush();
      esp_deep_sleep_start();
    }
  }
  else
  {
    Serial.println(F("Wakeup caused by reset"));
    setup_axp();
    sendLocation();
  }

  axp_sleep();
  Serial.flush();
  esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP_IRQ, 0); // axp
  // digitalWrite(LED, HIGH);    //turn the LED off
  pinMode(LED, INPUT_PULLDOWN); // let the LED glim
  esp_deep_sleep_start();
}

void loop()
{
}
