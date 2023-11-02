#ifdef AXP
#include "power.hpp"

XPowersLibInterface *PMU = NULL;
bool axpIrq = 0;

void startup_axp()
{
    Wire.begin(AXP_SDA, AXP_SCL);
    if (!PMU)
    {
        PMU = new XPowersAXP2101(Wire);
        if (!PMU->init())
        {
            Serial.println("Warning: Failed to find AXP2101 power management");
            delete PMU;
            PMU = NULL;
        }
        else
        {
            Serial.println("AXP2101 PMU init succeeded, using AXP2101 PMU");
        }
    }

    if (!PMU)
    {
        PMU = new XPowersAXP192(Wire);
        if (!PMU->init())
        {
            Serial.println("Warning: Failed to find AXP192 power management");
            delete PMU;
            PMU = NULL;
        }
        else
        {
            Serial.println("AXP192 PMU init succeeded, using AXP192 PMU");
        }
    }

    // pinMode(AXP_IRQ, INPUT_PULLUP);
    pinMode(AXP_IRQ, INPUT);
    // attachInterrupt(digitalPinToInterrupt(AXP_IRQ), axp_interrupt, FALLING);
}

void setup_axp()
{
    startup_axp();

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        // oled module power channel,
        // disable it will cause abnormal communication between boot and AXP power supply,
        // do not turn it off
        PMU->setPowerChannelVoltage(XPOWERS_DCDC1, 3300);

        // protected oled power source
        // PMU->setProtectedChannel(XPOWERS_DCDC1);
        // protected esp32 power source
        PMU->setProtectedChannel(XPOWERS_DCDC3);

        // disable not use channel
        PMU->disablePowerOutput(XPOWERS_DCDC2);

        // Set constant current charging current
        PMU->setChargerConstantCurr(XPOWERS_AXP192_CHG_CUR_450MA);

        // Set up the charging voltage
        PMU->setChargeTargetVoltage(XPOWERS_AXP192_CHG_VOL_4V2);
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        // Unuse power channel
        PMU->disablePowerOutput(XPOWERS_DCDC2);
        PMU->disablePowerOutput(XPOWERS_DCDC3);
        PMU->disablePowerOutput(XPOWERS_DCDC4);
        PMU->disablePowerOutput(XPOWERS_DCDC5);
        PMU->disablePowerOutput(XPOWERS_ALDO1);
        PMU->disablePowerOutput(XPOWERS_ALDO4);
        PMU->disablePowerOutput(XPOWERS_BLDO1);
        PMU->disablePowerOutput(XPOWERS_BLDO2);
        PMU->disablePowerOutput(XPOWERS_DLDO1);
        PMU->disablePowerOutput(XPOWERS_DLDO2);

        // GNSS RTC PowerVDD 3300mV
        PMU->setPowerChannelVoltage(XPOWERS_VBACKUP, 3300);
        PMU->enablePowerOutput(XPOWERS_VBACKUP);

        // ESP32 VDD 3300mV
        //  ! No need to set, automatically open , Don't close it
        //  PMU->setPowerChannelVoltage(XPOWERS_DCDC1, 3300);
        //  PMU->setProtectedChannel(XPOWERS_DCDC1);
        PMU->setProtectedChannel(XPOWERS_DCDC1);
    }
    PMU->enableSystemVoltageMeasure();
    PMU->enableVbusVoltageMeasure();
    PMU->enableBattVoltageMeasure();
    // It is necessary to disable the detection function of the TS pin on the board
    // without the battery temperature detection function, otherwise it will cause abnormal charging
    PMU->disableTSPinMeasure();

    // Set the time of pressing the button to turn off
    PMU->setPowerKeyPressOffTime(XPOWERS_POWEROFF_10S);

    uint64_t pmuIrqMask = 0;
    uint64_t pmuIrqDis = 0;

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        pmuIrqDis = XPOWERS_AXP192_ALL_IRQ;
        // pmuIrqMask = XPOWERS_AXP192_VBUS_INSERT_IRQ | XPOWERS_AXP192_BAT_INSERT_IRQ | XPOWERS_AXP192_PKEY_SHORT_IRQ;
        pmuIrqMask = XPOWERS_AXP192_PKEY_SHORT_IRQ | XPOWERS_AXP192_PKEY_LONG_IRQ;
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        pmuIrqDis = XPOWERS_AXP2101_ALL_IRQ;
        // pmuIrqMask |= XPOWERS_AXP2101_BAT_INSERT_IRQ | XPOWERS_AXP2101_BAT_REMOVE_IRQ;      // BATTERY
        // pmuIrqMask |= XPOWERS_AXP2101_VBUS_INSERT_IRQ | XPOWERS_AXP2101_VBUS_REMOVE_IRQ;    // VBUS
        pmuIrqMask |= XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ; // POWER KEY
        // pmuIrqMask |= XPOWERS_AXP2101_BAT_CHG_DONE_IRQ | XPOWERS_AXP2101_BAT_CHG_START_IRQ; // CHARGE
    }
    PMU->disableIRQ(pmuIrqDis);
    PMU->clearIrqStatus();
    PMU->enableIRQ(pmuIrqMask);

    axp_lora(1);
    axp_gps(1);
    axp_display(1);
}

void axp_gps(uint8_t state)
{
    if (state == 0) // disable GPS
    {
        if (PMU->getChipModel() == XPOWERS_AXP192)
        {
            PMU->disablePowerOutput(XPOWERS_LDO3);
        }
        else if (PMU->getChipModel() == XPOWERS_AXP2101)
        {
            PMU->disablePowerOutput(XPOWERS_ALDO3);
        }
        return;
    }

    uint16_t voltage = 3000;
    if (state == 2)
        voltage = 2500;

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        PMU->setPowerChannelVoltage(XPOWERS_LDO3, voltage);
        PMU->enablePowerOutput(XPOWERS_LDO3);
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        PMU->setPowerChannelVoltage(XPOWERS_ALDO3, voltage);
        PMU->enablePowerOutput(XPOWERS_ALDO3);
    }
}

void axp_lora(bool state)
{
    if (state == 0) // disable LoRa
    {
        if (PMU->getChipModel() == XPOWERS_AXP192)
        {
            PMU->disablePowerOutput(XPOWERS_LDO2);
        }
        else if (PMU->getChipModel() == XPOWERS_AXP2101)
        {
            PMU->disablePowerOutput(XPOWERS_ALDO2);
        }
        return;
    }

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        PMU->setPowerChannelVoltage(XPOWERS_LDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_LDO2);
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO2);
    }
}

void axp_display(bool state)
{
    if (state == 0) // disable LoRa
    {
        if (PMU->getChipModel() == XPOWERS_AXP192)
        {
            PMU->disablePowerOutput(XPOWERS_DCDC1);
        }
        else if (PMU->getChipModel() == XPOWERS_AXP2101)
        {
            // PMU->disablePowerOutput(XPOWERS_ALDO3);
        }
        return;
    }

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        PMU->setPowerChannelVoltage(XPOWERS_DCDC1, 3300);
        PMU->enablePowerOutput(XPOWERS_DCDC1);
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        // PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
        // PMU->enablePowerOutput(XPOWERS_ALDO3);
    }
}

void axp_print()
{
    Serial.print("isCharging:");
    Serial.println(PMU->isCharging() ? "YES" : "NO");
    Serial.print("isDischarge:");
    Serial.println(PMU->isDischarge() ? "YES" : "NO");
    Serial.print("isVbusIn:");
    Serial.println(PMU->isVbusIn() ? "YES" : "NO");
    Serial.print("getBattVoltage:");
    Serial.print(PMU->getBattVoltage());
    Serial.println("mV");
    Serial.print("getVbusVoltage:");
    Serial.print(PMU->getVbusVoltage());
    Serial.println("mV");
    Serial.print("getSystemVoltage:");
    Serial.print(PMU->getSystemVoltage());
    Serial.println("mV");
    // The battery percentage may be inaccurate at first use, the PMU will automatically
    // learn the battery curve and will automatically calibrate the battery percentage
    // after a charge and discharge cycle
    if (PMU->isBatteryConnect())
    {
        Serial.print("getBatteryPercent:");
        Serial.print(PMU->getBatteryPercent());
        Serial.println("%");
    }
    Serial.println();

    Serial.printf("=========================================\n");
    if (PMU->isChannelAvailable(XPOWERS_DCDC1))
    {
        Serial.printf("DC1  : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_DCDC1) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_DCDC1));
    }
    if (PMU->isChannelAvailable(XPOWERS_DCDC2))
    {
        Serial.printf("DC2  : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_DCDC2) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_DCDC2));
    }
    if (PMU->isChannelAvailable(XPOWERS_DCDC3))
    {
        Serial.printf("DC3  : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_DCDC3) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_DCDC3));
    }
    if (PMU->isChannelAvailable(XPOWERS_DCDC4))
    {
        Serial.printf("DC4  : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_DCDC4) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_DCDC4));
    }
    if (PMU->isChannelAvailable(XPOWERS_DCDC5))
    {
        Serial.printf("DC5  : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_DCDC5) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_DCDC5));
    }
    if (PMU->isChannelAvailable(XPOWERS_LDO2))
    {
        Serial.printf("LDO2 : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_LDO2) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_LDO2));
    }
    if (PMU->isChannelAvailable(XPOWERS_LDO3))
    {
        Serial.printf("LDO3 : %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_LDO3) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_LDO3));
    }
    if (PMU->isChannelAvailable(XPOWERS_ALDO1))
    {
        Serial.printf("ALDO1: %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_ALDO1) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_ALDO1));
    }
    if (PMU->isChannelAvailable(XPOWERS_ALDO2))
    {
        Serial.printf("ALDO2: %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_ALDO2) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_ALDO2));
    }
    if (PMU->isChannelAvailable(XPOWERS_ALDO3))
    {
        Serial.printf("ALDO3: %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_ALDO3) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_ALDO3));
    }
    if (PMU->isChannelAvailable(XPOWERS_ALDO4))
    {
        Serial.printf("ALDO4: %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_ALDO4) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_ALDO4));
    }
    if (PMU->isChannelAvailable(XPOWERS_BLDO1))
    {
        Serial.printf("BLDO1: %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_BLDO1) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_BLDO1));
    }
    if (PMU->isChannelAvailable(XPOWERS_BLDO2))
    {
        Serial.printf("BLDO2: %s   Voltage: %04u mV \n", PMU->isPowerChannelEnable(XPOWERS_BLDO2) ? "+" : "-", PMU->getPowerChannelVoltage(XPOWERS_BLDO2));
    }
    Serial.printf("=========================================\n");
}

uint8_t axp_cause()
{
    uint8_t ret = 0;
    uint32_t status = PMU->getIrqStatus();
    Serial.print("STATUS => HEX:");
    Serial.print(status, HEX);
    Serial.print(" BIN:");
    Serial.println(status, BIN);

    if (PMU->isPekeyShortPressIrq())
    {
        Serial.println("isPekeyShortPress");
        ret = 1;
    }
    if (PMU->isPekeyLongPressIrq())
    {
        Serial.println("isPekeyLongPress");
        ret = 2;
    }

    PMU->clearIrqStatus();

    return ret;
}

void axp_sleep()
{
    // detachInterrupt(digitalPinToInterrupt(AXP_IRQ));
    esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP_IRQ, 0);
}

uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset)
{
    if (PMU->isBatteryConnect())
        txBuffer[offset] = (PMU->getBattVoltage() / 10) - 250;
    else
        txBuffer[offset] = 0xff;
    return offset + 1;
}
#endif
