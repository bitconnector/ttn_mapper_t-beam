//https://github.com/HelTecAutomation/CubeCell-Arduino/blob/master/libraries/Basics/examples/LowPower/LowPower_WakeUpByGPIO/LowPower_WakeUpByGPIO.ino
#include "LoRaWan_APP.h"
#include "Arduino.h"

#define CUBECELL
//#define ButtonPin

//#include "gps.hpp"
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
  // digitalWrite(Vext, LOW);

  lorawan_sleep();
  attachInterrupt(ButtonPin, onWakeUp, FALLING);
  TimerInit(&sleep, onWakeUp);

  Serial.printf("Startup\n");
  delay(3000);
  //startup_lorawan();
  //sendStatus(2, 0);
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
    }
    else if (buttonHold < 2500) //long press
    {
      Serial.printf("long\n");
    }
  }
  else //Timer wakeup
  {
    Serial.printf("timer\n");
  }
  TimerSetValue(&sleep, 5000);
  TimerStart(&sleep);
  Serial.flush();
  delay(300);
  lowpower = 1;
  while (lowpower)
    lowPowerHandler();
}

void loop2()
{
  while (true)
  {
    Serial.printf("%i, %u\n", !digitalRead(ButtonPin), millis());
    delay(100);
  }
  if (digitalRead(ButtonPin)) // <-------------- timer
  {
    Serial.println(F("Wakeup caused by timer"));
    int gpsStatus = 0; //getGPS();

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
      int gpsStatus = 0; //getGPS();
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

  Serial.println("enter sleep");

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
  //bufferSize = location_bin(txBuffer, bufferSize);
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
