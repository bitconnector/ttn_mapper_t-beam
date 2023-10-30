#ifdef AXP
#include "power.hpp"

AXP20X_Class axp;
XPowersLibInterface *PMU = NULL;
bool axpIrq = 0;

void startup_axp()
{
    Wire.begin(AXP_SDA, AXP_SCL);
    // if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS))
    // {
    //     Serial.println("AXP192 Begin PASS");
    // }
    // else
    // {
    //     Serial.println("AXP192 Begin FAIL");
    // }
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

    /*
    axp.adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 |
                       AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1,
                   true);

    axp.setlongPressTime(2); // Long press time to 2s
    axp.setShutdownTime(3);  // Shutdown time to 10s
    axp.setPowerDownVoltage(3200);

    axp.setLDO2Voltage(2000);  // LoRa VDD
    axp.setLDO3Voltage(3000);  // GPS
    axp.setDCDC3Voltage(2600); // lower ESP32 voltage

    axp_gps(1);
    axp_lora(1);

    axp.enableIRQ(axp_irq_t::AXP202_ALL_IRQ, false);
    axp.enableIRQ(AXP202_PEK_LONGPRESS_IRQ | AXP202_PEK_SHORTPRESS_IRQ | AXP202_NOE_OFF_IRQ, true);
    axp.clearIRQ();
    */
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

        // disable all axp chip interrupt
        PMU->disableIRQ(XPOWERS_AXP192_ALL_IRQ);

        // Set constant current charging current
        PMU->setChargerConstantCurr(XPOWERS_AXP192_CHG_CUR_450MA);

        // Set up the charging voltage
        PMU->setChargeTargetVoltage(XPOWERS_AXP192_CHG_VOL_4V2);

        PMU->setPowerKeyPressOffTime(XPOWERS_AXP192_POWEROFF_10S);
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

        // LoRa VDD 3300mV
        PMU->setPowerChannelVoltage(XPOWERS_ALDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO2);

        // GNSS VDD 3300mV
        PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO3);
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
        //pmuIrqMask |= XPOWERS_AXP2101_BAT_INSERT_IRQ | XPOWERS_AXP2101_BAT_REMOVE_IRQ;      // BATTERY
        //pmuIrqMask |= XPOWERS_AXP2101_VBUS_INSERT_IRQ | XPOWERS_AXP2101_VBUS_REMOVE_IRQ;    // VBUS
        pmuIrqMask |= XPOWERS_AXP2101_PKEY_SHORT_IRQ | XPOWERS_AXP2101_PKEY_LONG_IRQ;       // POWER KEY
        //pmuIrqMask |= XPOWERS_AXP2101_BAT_CHG_DONE_IRQ | XPOWERS_AXP2101_BAT_CHG_START_IRQ; // CHARGE
    }
    PMU->disableIRQ(pmuIrqDis);
    PMU->clearIrqStatus();
    PMU->enableIRQ(pmuIrqMask);

    axp_lora(1);
    axp_gps(1);
}

void axp_gps(uint8_t state)
{

    if (state == 0) // disable GPS
    {
        // if (axp.isLDO3Enable())
        //     axp.setPowerOutPut(AXP192_LDO3, AXP202_OFF); // GPS
        if (PMU->getChipModel() == XPOWERS_AXP192)
        {
            // gnss module power channel -  now turned on in setGpsPower
            // PMU->setPowerChannelVoltage(XPOWERS_LDO3, 3300);
            // PMU->enablePowerOutput(XPOWERS_LDO3);
            PMU->disablePowerOutput(XPOWERS_LDO3);
        }
        else if (PMU->getChipModel() == XPOWERS_AXP2101)
        {

            /**
             * gnss module power channel
             * The default ALDO4 is off, you need to turn on the GNSS power first, otherwise it will be invalid during initialization
             */
            // PMU->setPowerChannelVoltage(XPOWERS_ALDO4, 3300);
            // PMU->enablePowerOutput(XPOWERS_ALDO4);
            PMU->disablePowerOutput(XPOWERS_ALDO4);
        }
        return;
    }

    uint16_t voltage = 3000;
    if (state == 2)
        voltage = 2500;

    // if (!axp.isLDO3Enable())
    //     axp.setPowerOutPut(AXP192_LDO3, AXP202_ON); // GPS
    // if (axp.getLDO3Voltage() != voltage)
    //     axp.setLDO3Voltage(voltage);

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        // gnss module power channel -  now turned on in setGpsPower
        PMU->setPowerChannelVoltage(XPOWERS_LDO3, 3300);
        PMU->enablePowerOutput(XPOWERS_LDO3);
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        /**
         * gnss module power channel
         * The default ALDO4 is off, you need to turn on the GNSS power first, otherwise it will be invalid during initialization
         */
        PMU->setPowerChannelVoltage(XPOWERS_ALDO4, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO4);
    }
}

void axp_lora(bool state)
{
    // axp.setPowerOutPut(AXP192_LDO2, state); // LORA

    if (PMU->getChipModel() == XPOWERS_AXP192)
    {
        // lora radio power channel
        PMU->setPowerChannelVoltage(XPOWERS_LDO2, 3300);
        PMU->enablePowerOutput(XPOWERS_LDO2);
    }
    else if (PMU->getChipModel() == XPOWERS_AXP2101)
    {
        // lora radio power channel
        PMU->setPowerChannelVoltage(XPOWERS_ALDO3, 3300);
        PMU->enablePowerOutput(XPOWERS_ALDO3);
    }
}

uint8_t axp_cause()
{
    uint8_t ret = 0;
    /*axp.readIRQ();
    if (axp.isPEKShortPressIRQ())
    {
        Serial.printf("isPEKShortPressIRQ\n");
        ret = 1;
    }
    if (axp.isPEKLongtPressIRQ())
    {
        Serial.printf("isPEKLongtPressIRQ\n");
        ret = 2;
    }
    if (axp.isNOEPowerDownIRQ())
    {
        Serial.printf("isNOEPowerDownIRQ\n");
        ret = 3;
    }

    axp.clearIRQ();*/

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
    /*if (axp.isBatteryConnect())
        txBuffer[offset] = (axp.getBattVoltage() / 10) - 250;
    else
        txBuffer[offset] = 0xff;
    return offset + 1;*/
    if (PMU->isBatteryConnect())
        txBuffer[offset] = (PMU->getBattVoltage() / 10) - 250;
    else
        txBuffer[offset] = 0xff;
    return offset + 1;
}
#endif
