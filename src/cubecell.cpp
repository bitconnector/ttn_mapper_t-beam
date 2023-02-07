//https://github.com/HelTecAutomation/CubeCell-Arduino/blob/master/libraries/Basics/examples/LowPower/LowPower_WakeUpByGPIO/LowPower_WakeUpByGPIO.ino
#include "LoRaWan_APP.h"
#include "Arduino.h"

#define CUBECELL
//#define ButtonPin

#include "gps.hpp"
#include "lorawan.hpp"
#include "config.hpp"

unsigned int TX_INTERVAL = GPS_INTERVAL;
int wakeup_count = 0;

void sendLocation();
void sendStatus(int state, int gps);

static TimerEvent_t sleep;
uint8_t lowpower = 0;
void onWakeUp()
{
  lowpower = 0;
}

void setup()
{
  pinMode(Vext, OUTPUT);
  pinMode(ButtonPin, INPUT);
  Serial.begin(115200);
  // digitalWrite(Vext, LOW); // OLED

  lorawan_sleep();
  attachInterrupt(ButtonPin, onWakeUp, FALLING);
  TimerInit(&sleep, onWakeUp);

  Serial.printf("Startup\n");
  delay(3000);

  setup_gps();
  while (0)
    gps_loop();

  startup_lorawan();
  sendStatus(2, 0);
}

void loop()
{
  TimerStop(&sleep);
  if (digitalRead(ButtonPin) == 0) //Interrupt wakeup
  {
    Serial.printf("button ");
    unsigned long buttonHold = millis();
    while (digitalRead(ButtonPin) == 0)
      ;
    buttonHold = millis() - buttonHold;
    if (buttonHold < 1000) //short press
    {
      Serial.printf("short\n");
      Serial.println(F("send status and location"));
      int gpsStatus = getGPS();
      sendStatus(1, gpsStatus);
      if (gpsStatus == 1 || gpsStatus == 2)
        sendLocation();
      TimerSetValue(&sleep, TX_INTERVAL * 1000);
      TimerStart(&sleep);
    }
    else if (buttonHold < 2500) //long press
    {
      Serial.printf("long\n");
      Serial.print(F("entering deep sleep for infinity\n"));
      end_gps();

      Serial.flush();
      delay(300);
      lowpower = 1;
      while (lowpower)
        lowPowerHandler();

      setup_gps();
    }
  }
  else //Timer wakeup
  {
    Serial.printf("timer\n");
    Serial.println(F("Wakeup caused by timer"));
    int gpsStatus = getGPS();
    if (wakeup_count % STATUS_INTERVAL == 0)
    {
      Serial.println(F("periodically send state"));
      sendStatus(4, gpsStatus);
    }
    if (gpsStatus == 1)
      sendLocation();
    TimerSetValue(&sleep, TX_INTERVAL * 1000);
    TimerStart(&sleep);
  }
  Serial.flush();
  delay(300);
  lowpower = 1;
  while (lowpower)
    lowPowerHandler();
  wakeup_count++;
}

uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset)
{
  txBuffer[offset] = (getBatteryVoltage() / 10) - 250;
  return offset + 1;
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