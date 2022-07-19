#include "power.hpp"

AXP20X_Class axp;
bool axpIrq = 0;

void startup_axp()
{
    Wire.begin(AXP_SDA, AXP_SCL);
    if (!axp.begin(Wire, AXP192_SLAVE_ADDRESS))
    {
        Serial.println("AXP192 Begin PASS");
    }
    else
    {
        Serial.println("AXP192 Begin FAIL");
    }

    pinMode(AXP_IRQ, INPUT_PULLUP);
    // attachInterrupt(digitalPinToInterrupt(AXP_IRQ), axp_interrupt, FALLING);
}

void setup_axp()
{
    startup_axp();

    axp.adc1Enable(AXP202_VBUS_VOL_ADC1 | AXP202_VBUS_CUR_ADC1 | AXP202_BATT_CUR_ADC1 | AXP202_BATT_VOL_ADC1, true);
    axp.setlongPressTime(2); // Long press time to 2s
    axp.setShutdownTime(3);  // Shutdown time to 10s
    axp.setPowerDownVoltage(3400);

    axp_gps(1);
    axp_lora(1);

    axp.enableIRQ(axp_irq_t::AXP202_ALL_IRQ, false);
    axp.enableIRQ(AXP202_PEK_LONGPRESS_IRQ | AXP202_PEK_SHORTPRESS_IRQ | AXP202_NOE_OFF_IRQ, true);
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

uint8_t axp_cause()
{
    uint8_t ret = 0;
    axp.readIRQ();
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

    axp.clearIRQ();
    return ret;
}

void axp_sleep()
{
    detachInterrupt(digitalPinToInterrupt(AXP_IRQ));
    esp_sleep_enable_ext0_wakeup((gpio_num_t)AXP_IRQ, 0);
}

uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset)
{
    if (!axp.isVBUSPlug() && axp.isBatteryConnect())
        txBuffer[offset] = (axp.getBattVoltage() / 10) - 250;
    else
        txBuffer[offset] = 0xff;
    return offset + 1;
}
