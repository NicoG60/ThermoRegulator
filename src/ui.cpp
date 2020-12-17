#include "ui.h"

#include "bitmaps.h"
#include "thermometer.h"
#include "triac.h"
#include "controller.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_GFX.h>

Ui _ui;

/**
 * @brief Initialise the display
 */
Ui::Ui() :
    display(SSD1306_LCDWIDTH, SSD1306_LCDHEIGHT, &Wire, -1)
{}

/**
 * @brief Setup the display and the buttons
 */
void Ui::setup()
{
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.cp437();
    display.setTextColor(SSD1306_WHITE);

    pinMode(SW1_PIN, INPUT_PULLUP);
    pinMode(SW2_PIN, INPUT_PULLUP);
    pinMode(SW3_PIN, INPUT_PULLUP);

    setupInterrupt();
}

/**
 * @brief Clears the display and turn the backlight off
 */
void Ui::turnOff()
{
    if(isTurnedOn)
    {
        display.clearDisplay();
        display.display();
        display.ssd1306_command(SSD1306_DISPLAYOFF);
        isTurnedOn = false;
    }
}

/**
 * @brief Turn the backlight on again
 */
void Ui::turnOn()
{
    if(!isTurnedOn)
    {
        display.ssd1306_command(SSD1306_DISPLAYON);
        display.clearDisplay();
        display.display();
        isTurnedOn = true;
    }
}

/*******************************************************************************
 * Drawing functions
 ******************************************************************************/

void Ui::drawLoadingScreen()
{
    display.clearDisplay();

    display.drawRect(0, 0, 128, 32, SSD1306_WHITE);

    display.drawBitmap(5, 10, guinea_pig, guinea_pig_w, guinea_pig_h, 1);

    display.setTextSize(1);
    display.setCursor(32, 8);
    display.print(F("ThermoRegulator"));
    display.setCursor(32, 18);
    display.print(F("  - Loading -  "));

    display.display();
}

void Ui::drawIdleScreen()
{
    display.clearDisplay();

    display.drawRect(0, 0, 128, 32, SSD1306_WHITE);

    display.setTextSize(1);

    unsigned int m,s;
    _controller.thermoTimer.remainingTime(m, s);

    display.setCursor(4, 4);
    display.print(_controller.thermoTimer.hasExpired() ? 'x' : 'R');
    display.print(": ");
    if(m < 10) display.print('0');
    display.print(m);
    display.print(':');
    if(s < 10) display.print('0');
    display.println(s);

    display.setCursor(4, display.getCursorY());
    display.print("T: ");
    display.print(_controller.ideal);
    display.println(".C");

    display.setCursor(4, display.getCursorY());
    display.print("P: ");

    if(_controller.isTurnedOn)
    {
        display.print(100 - (_triac.triacDelay/100));
        display.print("% ");

        if(_controller.shouldWarmUp())
            display.write(24);
        else if(_controller.shouldCoolDown())
            display.write(25);
    }
    else
        display.print("0% x");
    
    

    display.setCursor(85, 9);
    display.setTextSize(1, 2);
    display.print(_thermo.temperature, 1);
    display.print(".C");

    display.display();
}

void Ui::drawMenuScreen()
{
    display.clearDisplay();

    display.drawRect(0, 0, 128, 32, SSD1306_WHITE);

    display.setTextSize(1);
    switch(_controller.state)
    {
    case Controller::MenuSetTemp:
        display.setCursor(24, 4);
        display.print("Set Temperature");
        break;

    case Controller::MenuSetTempBias:
        display.setCursor(23, 4);
        display.print("Calibrate Sensor");
        break;

    case Controller::MenuSetTime:
        display.setCursor(40, 4);
        display.print("Set Timer");
        break;

    case Controller::MenuResetTime:
        display.setCursor(35, 4);
        display.print("Reset Timer");
        break;

    case Controller::MenuShowLoading:
        display.setCursor(30, 4);
        display.print("Splash screen");
        break;

    case Controller::MenuDebug:
        display.setCursor(49, 4);
        display.print("Debug");
        break;

    default:
    case Controller::MenuReturn:
        display.setCursor(47, 4);
        display.print("Return");
        break;
    }

    drawButton(32, 22, '\x11');

    drawButton(64, 22, '\x19');

    drawButton(96, 22, '\x10');

    display.display();
}

void Ui::drawSetTempScreen()
{
    display.clearDisplay();

    display.drawRect(0, 0, 128, 32, SSD1306_WHITE);

    display.setTextSize(1, 2);
    display.setCursor(51, 9);
    display.print(_controller.idealTmp);
    display.print(".C");

    drawButton(16, 16, '-');
    drawButton(112, 16, '+');

    display.display();
}

void Ui::drawSetTempBiasScreen()
{
    display.clearDisplay();

    display.drawRect(0, 0, 128, 32, SSD1306_WHITE);

    display.setTextSize(1, 2);
    display.setCursor(50, 9);
    display.print(_controller.biasTmp, 1);
    display.print(".C");

    drawButton(16, 16, '-');
    drawButton(112, 16, '+');

    display.display();
}

void Ui::drawSetTimeScreen()
{
    display.clearDisplay();

    display.drawRect(0, 0, 128, 32, SSD1306_WHITE);

    display.setTextSize(1, 2);
    display.setCursor(48, 9);
    display.print(_controller.timerTmp);
    display.print("min");

    drawButton(16, 16, '-');
    drawButton(112, 16, '+');

    display.display();
}

void Ui::drawDebugScreen()
{
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);

    display.print("T: ");
    display.println(_thermo.temperature);

    display.print("I: ");
    display.print(_triac.syncDelay);
    display.print("|");
    display.print(_triac.triacDelay);
    display.print("|");
    display.println(_triac.tickCount);

    display.print("L: ");
    display.println(loopTimer.elapsedTime());
    loopTimer.restart();

    display.display();
}

void Ui::drawButton(int x, int y, char c)
{
    display.drawCircle(x, y, 7, SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(x-2, y-3);
    display.print(c);
}

/*******************************************************************************
 * Buttons management function
 ******************************************************************************/
bool Ui::hasButtonBeenPressed() const
{
    return btnLeftPressed || btnCenterPressed || btnRightPressed;
}

void Ui::resetButtons()
{
    btnLeftPressed = false;
    btnRightPressed = false;
    btnCenterPressed = false;
}

void Ui::setupInterrupt()
{
    attachInterrupt(digitalPinToInterrupt(SW1_PIN), Ui::interrupt1, FALLING);
    attachInterrupt(digitalPinToInterrupt(SW2_PIN), Ui::interrupt2, FALLING);
    attachInterrupt(digitalPinToInterrupt(SW3_PIN), Ui::interrupt3, FALLING);
}

void Ui::removeInterrupt()
{
    detachInterrupt(digitalPinToInterrupt(SW1_PIN));
    detachInterrupt(digitalPinToInterrupt(SW2_PIN));
    detachInterrupt(digitalPinToInterrupt(SW3_PIN));
}

void Ui::interrupt(volatile bool& btn)
{
    static unsigned long last_time = 0;
    unsigned long time = millis();

    // If interrupts come faster than 300ms, assume it's a bounce and ignore
    if (time - last_time > 300)
        btn = true;

    last_time = time;
}

void Ui::interrupt1()
{
    interrupt(_ui.btnLeftPressed);
}

void Ui::interrupt2()
{
    interrupt(_ui.btnCenterPressed);
}

void Ui::interrupt3()
{
    interrupt(_ui.btnRightPressed);
}