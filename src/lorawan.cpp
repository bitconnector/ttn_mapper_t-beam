#include "lorawan.hpp"

unsigned char Buffer[235];
RTC_DATA_ATTR LoraWANmessage message;
RTC_DATA_ATTR bool joined = 0;

void startup_lorawan()
{
    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);

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
            long frequency = getFrequency();
            lora_tx(frequency, 7);

            esp_random();
            otaa.setDevNonce((uint16_t)random(256 * 256));
            otaa.joinmsg();
            LoRa.beginPacket(); // start packet
            LoRa.write(message.data, message.dataLen);
            LoRa.endPacket(); // finish packet and send it
            unsigned long txTime = millis();

            // RX1
            lora_rx(frequency, 7);
            while (txTime + 20000 > millis() && !joined)
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

bool lorawan_send(uint8_t _port, uint8_t *_data, uint8_t _size, bool _confirm, int _sf)
{
    long frequency = getFrequency();
    lora_tx(frequency, _sf);

    message.uplink((char *)_data, _size, _port, _confirm);
    printPackage((char *)message.data, message.dataLen, 1);

    LoRa.beginPacket(); // start packet
    LoRa.write(message.data, message.dataLen);
    LoRa.endPacket(); // finish packet and send it

    if (_confirm)
    {
        return 1;
    }
    return 0;
}

void printHex2(unsigned v)
{
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

void lorawan_sleep()
{
    LoRa.sleep();
}

long getFrequency()
{
    uint16_t frequencylist[] = {8681, 8683, 8685, 8671, 8673, 8675, 8677, 8679};
    uint8_t idx = message.frameCounterUp % (sizeof(frequencylist) / sizeof(uint16_t));
    long frequency = frequencylist[idx] * 100000;
    return frequency;
}

void lora_tx(long frequency, int sf)
{
    LoRa.begin(frequency);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.enableCrc();
    LoRa.disableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(125E3);
}

void lora_rx(long frequency, int sf)
{
    LoRa.begin(frequency);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.disableCrc();
    LoRa.enableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(125E3);
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
