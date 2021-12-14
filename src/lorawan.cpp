#include "lorawan.hpp"

unsigned char Buffer[235];
RTC_DATA_ATTR LoraWANmessage message;
RTC_DATA_ATTR bool joined = 0;

bool has_send = false;

void startup_lorawan()
{
    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
    delay(100);
    LoRa.begin(868100000);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);

    has_send = 0;

#ifdef USE_OTAA
    if (joined == 0)
    {
        message = LoraWANmessage(Buffer);
        LoraWANactivation otaa = LoraWANactivation(&message);
        otaa.setDevNonce((uint16_t)random(70225));
        otaa.setDevEUI(DEVEUI);
        otaa.setJoinEUI(APPEUI);
        otaa.setAppKey(APPKEY);

        Serial.print("Joining");
        while (!joined)
        {
            LoRa.begin(868100000);
            LoRa.enableCrc();
            LoRa.disableInvertIQ();

            esp_random();
            otaa.setDevNonce((uint16_t)random(256 * 256));
            otaa.joinmsg();
            LoRa.beginPacket(); // start packet
            LoRa.write(message.data, message.dataLen);
            LoRa.endPacket(); // finish packet and send it
            unsigned long txTime = millis();

            //RX1
            LoRa.disableCrc();
            LoRa.enableInvertIQ();
            LoRa.receive();
            while (txTime + 10000 > millis() && !joined)
            {
                if (LoRa.parsePacket())
                {
                    message.dataLen = 0;
                    while (LoRa.available())
                        message.data[message.dataLen++] = LoRa.read();
                    joined = otaa.checkJoin((char *)message.data, message.dataLen);
                    printPackage((char *)message.data, message.dataLen, 0);
                }
            }
            Serial.print(".");
        }
        Serial.println("success!");
    }
#else
    if (joined == 0)
    {
        message = LoraWANmessage(Buffer);
        message.setDevAddr(DEVADDR);
        message.setNwkSKey(NWKSKEY);
        message.setAppSKey(APPSKEY);
        joined = 1;
    }
#endif
}

void lorawan_loop()
{
    delay(0);
}

void lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm)
{
    LoRa.begin(getFrequency());
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.enableCrc();
    LoRa.disableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(7);
    LoRa.setSignalBandwidth(125E3);

    // char payload[100];
    // sprintf(payload, "r=%02x", (int)random(256));
    // Serial.printf("Sending: %s\n", payload);

    message.uplink((char *)_data, _size, _port, _confirm);
    printPackage((char *)message.data, message.dataLen, 1);

    LoRa.beginPacket(); // start packet
    LoRa.write(message.data, message.dataLen);
    LoRa.endPacket(); // finish packet and send it
    has_send = 1;
}

void printHex2(unsigned v)
{
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

bool lorawan_has_send()
{
    return has_send;
}

void lorawan_sleep(unsigned long sleep_time)
{
    Serial.println(F("No DutyCycle recalculation function!"));
    LoRa.sleep();
}

long getFrequency()
{
    uint16_t frequencylist[] = {8681, 8683, 8685, 8671, 8673, 8675, 8677, 8679};
    uint8_t idx = message.frameCounterUp % (sizeof(frequencylist) / sizeof(uint16_t));
    long frequency = frequencylist[idx] * 100000;
    return frequency;
}

void printPackage(char *data, uint16_t size, bool structure)
{
    Serial.print("+ Package:");
    for (int i = 0; i < size; i++)
        Serial.printf(" %02x", data[i]);
    if (structure)
    {
        Serial.print("\n          |hd|   addr    |Ct| FCnt|");
        if (size < 13)
        {
            Serial.println("    MIC    |");
        }
        else if (size == 13)
        {
            Serial.println("Pt|    MIC    |");
        }
        else
        {
            Serial.print("Pt|");
            for (int i = 14; i < size; i++)
                Serial.print("   ");
            Serial.println("  |    MIC    |");
        }
    }
    else
        Serial.println();
}
