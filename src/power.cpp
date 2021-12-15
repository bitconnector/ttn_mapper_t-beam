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
    attachInterrupt(digitalPinToInterrupt(PMU_IRQ), axp_interrupt, FALLING);
}

void setup_axp()
{
    startup_axp();

    axp_gps(1);
    axp_lora(1);

    axp.enableIRQ(axp_irq_t::AXP202_PEK_LONGPRESS_IRQ, true);
    axp.enableIRQ(axp_irq_t::AXP202_PEK_SHORTPRESS_IRQ, true);
    axp.enableIRQ(axp_irq_t::AXP202_VBUS_CONNECT_IRQ, true);
    axp.enableIRQ(axp_irq_t::AXP202_VBUS_REMOVED_IRQ, true);

    axp.clearIRQ();
}

void axp_gps(bool state)
{
    axp.setPowerOutPut(AXP192_LDO3, state); //GPS
}

void axp_lora(bool state)
{
    axp.setPowerOutPut(AXP192_LDO2, state); //LORA
}

void axp_interrupt(void)
{
    axpIrq = true;
}

void axp_loop()
{
    if (axpIrq)
    {
        axp.readIRQ();
        if (axp.isPEKShortPressIRQ())
        {
            Serial.printf("isPEKShortPressIRQ\n");
        }
        else if (axp.isPEKLongtPressIRQ())
        {
            Serial.printf("isPEKLongtPressIRQ\n");
        }
        else if (axp.isVbusPlugInIRQ())
        {
            Serial.printf("isVbusPlugInIRQ\n");
            //axp_gps(0); //switch off gps
            //axp.setChgLEDMode(axp_chgled_mode_t::AXP20X_LED_OFF);
        }
        else if (axp.isVbusRemoveIRQ())
        {
            Serial.printf("isVbusRemoveIRQ\n");
            //axp_gps(1); //switch on gps
            //axp.setChgLEDMode(axp_chgled_mode_t::AXP20X_LED_BLINK_1HZ);
        }
        else if (axp.isVBUSPlug())
        {
            Serial.printf("isVBUSPlug\n");
        }
        else
            Serial.printf("unknown IRQ reason\n");

        axp.clearIRQ();
        axpIrq = 0;
    }
}

void axp_sleep()
{
    detachInterrupt(digitalPinToInterrupt(PMU_IRQ));
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, 0);
}

uint8_t vbatt_bin(uint8_t *txBuffer, uint8_t offset)
{
    axp.debugCharging();
    txBuffer[offset + 0] = axp.getBattVoltage() / 20;
    return offset + 1;
}
