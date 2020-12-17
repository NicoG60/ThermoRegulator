#ifndef UI_H
#define UI_H

#include <Adafruit_SSD1306.h>

#include "timer.h"

/**
 * @brief The Ui class is responsible of drawing stuff on the screen and detect
 * user interraction.
 */
class Ui
{
public:
    Ui();
    
    void setup();

    void turnOff();
    void turnOn();

    void drawLoadingScreen();
    void drawIdleScreen();
    void drawMenuScreen();
    void drawSetTempScreen();
    void drawSetTempBiasScreen();
    void drawSetTimeScreen();
    void drawDebugScreen();

    void drawButton(int x, int y, char c);

    bool hasButtonBeenPressed() const;
    void resetButtons();

    void setupInterrupt();
    void removeInterrupt();

    static void interrupt(volatile bool& btn);
    static void interrupt1();
    static void interrupt2();
    static void interrupt3();

public:
    volatile bool  btnLeftPressed   = false;
    volatile bool  btnCenterPressed = false;
    volatile bool  btnRightPressed  = false;

    bool isTurnedOn = true;
    Adafruit_SSD1306 display;

    MicroTimer loopTimer;
};

extern Ui _ui;

#endif // UI_H