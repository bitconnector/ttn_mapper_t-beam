#include "power.hpp"

AXP20X_Class axp;
bool axpIrq = 0;

void startup_axp()
{
    Wire.begin(21, 22);
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        Serial.println("AXP192 Begin PASS");
    }
    else
    {
        Serial.println("AXP192 Begin FAIL");
    }

    pinMode(PMU_IRQ, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(PMU_IRQ), axp_interrupt, FALLING);
}

void setup_axp()
{
    startup_axp();

    axp.adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);

    axp_gps(1);
    axp_lora(1);

    Serial.print(" getVWarningLevel1 ");
    Serial.println(axp.getVWarningLevel1());
    Serial.print(" getVWarningLevel2 ");
    Serial.println(axp.getVWarningLevel2());
    Serial.print(" getTimerStatus ");
    Serial.println(axp.getTimerStatus());
    Serial.print(" getBattPercentage ");
    Serial.println(axp.getBattPercentage());
    Serial.print(" getPowerDownVoltage ");
    Serial.println(axp.getPowerDownVoltage());
    Serial.print(" getBattVoltage ");
    Serial.println(axp.getBattVoltage());
    Serial.print(" getDCDC1Voltage ");
    Serial.println(axp.getDCDC1Voltage());
    Serial.print(" getDCDC2Voltage ");
    Serial.println(axp.getDCDC2Voltage());
    Serial.print(" getDCDC3Voltage ");
    Serial.println(axp.getDCDC3Voltage());

    axp.enableIRQ(axp_irq_t::AXP202_ALL_IRQ, false);
    axp.enableIRQ(AXP202_PEK_LONGPRESS_IRQ | AXP202_PEK_SHORTPRESS_IRQ, true);
    // axp.enableIRQ(axp_irq_t::AXP202_NOE_ON_IRQ, false);
    axp.clearIRQ();
}

void axp_gps(bool state)
{
    axp.setPowerOutPut(AXP192_LDO3, state); // GPS
}

void axp_lora(bool state)
{
    axp.setPowerOutPut(AXP192_LDO2, state); // LORA
}

void axp_interrupt(void)
{
    axpIrq = true;
}

uint8_t axp_loop()
{
    uint8_t ret = 0;
    if (axpIrq)
    {
        axp.readIRQ();
        if (axp.isPEKShortPressIRQ())
        {
            Serial.printf("isPEKShortPressIRQ\n");
            ret = 1;
        }
        else if (axp.isPEKLongtPressIRQ())
        {
            Serial.printf("isPEKLongtPressIRQ\n");
            ret = 2;
        }

        Serial.println("axp20x irq enter!");
        if (axp.isAcinOverVoltageIRQ())
        {
            Serial.printf("isAcinOverVoltageIRQ\n");
        }
        if (axp.isAcinPlugInIRQ())
        {
            Serial.printf("isAcinPlugInIRQ\n");
        }
        if (axp.isAcinRemoveIRQ())
        {
            Serial.printf("isAcinRemoveIRQ\n");
        }
        if (axp.isVbusOverVoltageIRQ())
        {
            Serial.printf("isVbusOverVoltageIRQ\n");
        }
        if (axp.isVbusPlugInIRQ())
        {
            Serial.printf("isVbusPlugInIRQ\n");
        }
        if (axp.isVbusRemoveIRQ())
        {
            Serial.printf("isVbusRemoveIRQ\n");
        }
        if (axp.isVbusLowVHOLDIRQ())
        {
            Serial.printf("isVbusLowVHOLDIRQ\n");
        }
        if (axp.isBattPlugInIRQ())
        {
            Serial.printf("isBattPlugInIRQ\n");
        }
        if (axp.isBattRemoveIRQ())
        {
            Serial.printf("isBattRemoveIRQ\n");
        }
        if (axp.isBattEnterActivateIRQ())
        {
            Serial.printf("isBattEnterActivateIRQ\n");
        }
        if (axp.isBattExitActivateIRQ())
        {
            Serial.printf("isBattExitActivateIRQ\n");
        }
        if (axp.isChargingIRQ())
        {
            Serial.printf("isChargingIRQ\n");
        }
        if (axp.isChargingDoneIRQ())
        {
            Serial.printf("isChargingDoneIRQ\n");
        }
        if (axp.isBattTempLowIRQ())
        {
            Serial.printf("isBattTempLowIRQ\n");
        }
        if (axp.isBattTempHighIRQ())
        {
            Serial.printf("isBattTempHighIRQ\n");
        }
        if (axp.isChipOvertemperatureIRQ())
        {
            Serial.printf("isChipOvertemperatureIRQ\n");
        }
        if (axp.isChargingCurrentLessIRQ())
        {
            Serial.printf("isChargingCurrentLessIRQ\n");
        }
        if (axp.isDC2VoltageLessIRQ())
        {
            Serial.printf("isDC2VoltageLessIRQ\n");
        }
        if (axp.isDC3VoltageLessIRQ())
        {
            Serial.printf("isDC3VoltageLessIRQ\n");
        }
        if (axp.isLDO3VoltageLessIRQ())
        {
            Serial.printf("isLDO3VoltageLessIRQ\n");
        }
        if (axp.isPEKShortPressIRQ())
        {
            Serial.printf("isPEKShortPressIRQ\n");
        }
        if (axp.isPEKLongtPressIRQ())
        {
            Serial.printf("isPEKLongtPressIRQ\n");
        }
        if (axp.isNOEPowerOnIRQ())
        {
            Serial.printf("isNOEPowerOnIRQ\n");
        }
        if (axp.isNOEPowerDownIRQ())
        {
            Serial.printf("isNOEPowerDownIRQ\n");
        }
        if (axp.isVBUSEffectiveIRQ())
        {
            Serial.printf("isVBUSEffectiveIRQ\n");
        }
        if (axp.isVBUSInvalidIRQ())
        {
            Serial.printf("isVBUSInvalidIRQ\n");
        }
        if (axp.isVUBSSessionIRQ())
        {
            Serial.printf("isVUBSSessionIRQ\n");
        }
        if (axp.isVUBSSessionEndIRQ())
        {
            Serial.printf("isVUBSSessionEndIRQ\n");
        }
        if (axp.isLowVoltageLevel1IRQ())
        {
            Serial.printf("isLowVoltageLevel1IRQ\n");
        }
        if (axp.isLowVoltageLevel2IRQ())
        {
            Serial.printf("isLowVoltageLevel2IRQ\n");
        }
        if (axp.isTimerTimeoutIRQ())
        {
            Serial.printf("isTimerTimeoutIRQ\n");
            axp.offTimer();
            axp.setChgLEDMode(AXP20X_LED_BLINK_1HZ);
        }
        if (axp.isPEKRisingEdgeIRQ())
        {
            Serial.printf("isPEKRisingEdgeIRQ\n");
        }
        if (axp.isPEKFallingEdgeIRQ())
        {
            Serial.printf("isPEKFallingEdgeIRQ\n");
        }
        if (axp.isGPIO3InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO3InputEdgeTriggerIRQ\n");
        }
        if (axp.isGPIO2InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO2InputEdgeTriggerIRQ\n");
        }
        if (axp.isGPIO1InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO1InputEdgeTriggerIRQ\n");
        }
        if (axp.isGPIO0InputEdgeTriggerIRQ())
        {
            Serial.printf("isGPIO0InputEdgeTriggerIRQ\n");
        }

        axp.clearIRQ();
        axpIrq = 0;
    }
    return ret;
}

void axp_sleep()
{
    detachInterrupt(digitalPinToInterrupt(PMU_IRQ));
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
}

uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset)
{
    if (!axp.isVBUSPlug() && axp.isBatteryConnect())
        txBuffer[offset] = axp.getBattVoltage() / 20;
    else
        txBuffer[offset] = 0xff;
    return offset + 1;
}
