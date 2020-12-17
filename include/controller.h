#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "timer.h"
#include "ui.h"
#include "pins.h"
#include <Arduino.h>
#include <EEPROM.h>

#define IDEAL_ADDR 0
#define TIMER_ADDR IDEAL_ADDR + sizeof(int)
#define BIAS_ADDR  TIMER_ADDR + sizeof(int)

/**
 * @brief Th Controller class is the main controller of the program.
 * 
 * It manages the Screen, the different timers to turn on or off the heat.
 * It is also responsible for regulating the heat power depending on the
 * temperature
 */
class Controller
{
public:
    /**
     * @brief The different state to display on screen
     */ 
    enum State
    {
        Idle,

        MenuSetTemp,
        MenuSetTempBias,
        MenuSetTime,
        MenuResetTime,
        MenuShowLoading,
        MenuDebug,
        MenuReturn,

        SetTemp,
        SetTempBias,
        SetTime,
        LoadingScreen,
        Debug
    };

public:
    Controller() = default;

    void setup();
    void update();

    void updateTemperature();
    void processButtonPressed();
    void updateUI();

    /**
     * @brief A helper function that calls the callback corresponding to the
     * button being pressed
     */
    template<class LCB, class RCB, class CCB>
    void processActions(LCB l, RCB r, CCB c)
    {
        if(_ui.btnLeftPressed)
            l();
        else if(_ui.btnRightPressed)
            r();
        else if(_ui.btnCenterPressed)
            c();
    }

    /** 
     * @brief A helper function that provide a page based behaviour.
     * 
     * While the user presses the left or right buttons, the state member
     * variable is cycling through all the possible values between @a start and
     * @a end (included).
     * 
     * When the user press the central button. state is set to @a accepted and
     * @a cb is called.
     */
    template<class CB>
    void processPages(int start, int end, int accept, CB cb)
    {
        processActions(
        [=]()
        {
            if(state == start)
                state = end;
            else
                state--;
        },
        [=]()
        {
            if(state == end)
                state = start;
            else
                state++;
        },
        [=](){
            cb();
            state = accept;
        });
    }
    void processPages(int start, int end, int accept);

    /** 
     * @brief A helper function that cycle through the possible menus of the
     * interface.
     */
    template<class CB>
    void processMenu(int accept, CB cb)
    {
        processPages(MenuSetTemp, MenuReturn, accept, cb);
    }
    void processMenu(int accept);

    void warmUp();
    void coolDown();

    void turnOff();
    void turnOn();

    void setIdeal(int i);

    bool shouldWarmUp()   const;
    bool shouldCoolDown() const;

    void setTimer(unsigned int m, unsigned int s = 0, unsigned long int z = 0);
    void resetTimer();

    DeadlineTimer screenTimer;
    DeadlineTimer thermoTimer;

    int lastState = Idle;
    int state = Idle;

    bool isTurnedOn = false;

    int ideal       = 0;
    int idealTmp    = 0;

    int timerTmp    = 0;

    float biasTmp = 0;
};

extern Controller _controller;

#endif // CONTROLLER_H