#include "lorawan.hpp"

unsigned char Buffer[235];
SLEEP_VAR LoraWANmessage message;
SLEEP_VAR bool joined = 0;

#ifdef CUBECELL
int16_t Rssi, rxSize;
char *rxpacket = (char *)&message.data;
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);
#endif

void startup_lorawan()
{
#ifdef CUBECELL
    static RadioEvents_t RadioEvents;
    RadioEvents.RxDone = OnRxDone;
    Radio.Init(&RadioEvents);
#else
    LoRa.setPins(LoRa_CS, LoRa_RST, LoRa_DIO0);
#endif

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
#ifdef ESP32
            esp_random();
#endif
            otaa.setDevNonce((uint16_t)random(256 * 256));
            otaa.joinmsg();

            long frequency = getFrequency();
            lora_tx(frequency, 7, message.data, message.dataLen);
            unsigned long txTime = millis();

            // RX1
            lora_rx(frequency, 7);
            while (txTime + 20000 > millis() && !joined)
            {
                message.dataLen = lora_get(message.data);
                if (message.dataLen > 0)
                {
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

    message.uplink((char *)_data, _size, _port, _confirm);
    printPackage((char *)message.data, message.dataLen, 1);

    lora_tx(frequency, _sf, message.data, message.dataLen);

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

long getFrequency()
{
    uint16_t frequencylist[] = {8681, 8683, 8685, 8671, 8673, 8675, 8677, 8679};
    uint8_t idx = message.frameCounterUp % (sizeof(frequencylist) / sizeof(uint16_t));
    long frequency = frequencylist[idx] * 100000;
    return frequency;
}

#ifdef CUBECELL
void lorawan_sleep()
{
    Radio.Sleep();
}

void lora_tx(long frequency, int sf, uint8_t *data, uint8_t size)
{
    Radio.SetChannel(frequency);
    Radio.SetTxConfig(MODEM_LORA, 20, 0, 0,
                      sf, 1,
                      8, false,
                      true, 0, 0, false, 0);
    Radio.SetSyncWord(0x34);

    Radio.Send(data, size);
}

void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr)
{
    Rssi = rssi;
    rxSize = size;
    memcpy(message.data, payload, size);
    rxpacket[size] = '\0';
}

void lora_rx(long frequency, int sf)
{
    Radio.SetChannel(frequency);
    Radio.SetRxConfig(MODEM_LORA, 0, sf,
                      1, 0, 8,
                      0, false,
                      0, false, 0, 0, true, true);
    Radio.Rx(0);
    rxSize = 0;
}

uint8_t lora_get(uint8_t *_data)
{
    uint8_t size = rxSize;
    rxSize = 0;
    return size;
}
#else
void lorawan_sleep()
{
    LoRa.sleep();
}

void lora_tx(long frequency, int sf, uint8_t *data, uint8_t size)
{
    LoRa.begin(frequency);
    LoRa.setPreambleLength(8);
    LoRa.setSyncWord(0x34);
    LoRa.enableCrc();
    LoRa.disableInvertIQ();
    LoRa.setCodingRate4(5);
    LoRa.setSpreadingFactor(sf);
    LoRa.setSignalBandwidth(125E3);

    LoRa.beginPacket(); // start packet
    LoRa.write(data, size);
    LoRa.endPacket(); // finish packet and send it
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

uint8_t lora_get(uint8_t *_data)
{
    uint8_t size = 0;
    if (LoRa.parsePacket())
    {
        while (LoRa.available())
            _data[size++] = LoRa.read();
    }
    return size;
}
#endif

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
