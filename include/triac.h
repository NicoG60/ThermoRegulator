#ifndef TRIAC_H
#define TRIAC_H

#include <stdint.h>

#include "pins.h"
#include "timer.h"

/**
 * @brief The Triac class is responsible of driving the triac.
 * 
 * For those who wonder, when you use a triac to control how much power you send
 * to a fixture, you need to time it right. The delay you wait between the current
 * crossing 0 and the moment you drive the triac is directly proportional to
 * the power the fixture will receive.
 * 
 * It uses a couple of tricks to avoid the use of delays that would block the
 * controller to do other things such as refreshing the screen and make the
 * Screen and buttons feel laggy.
 * 
 * Each time the main line crosses 0, an interrupt is triggered (zeroDetect()).
 * In this interrupt, the program starts a hardware timer that has been setup to
 * trigger a second interrupt exactly when we want the triac to be driven.
 * In this second interrupt, it drives the triac and stops the timer.
 */
class Triac 
{
public:
    constexpr Triac() = default;

    void setup();
    
    void setDelay(unsigned int us);
    void incDelay(unsigned int a);
    void decDelay(unsigned int a);

    void detectSync();
    void turnOn();
    void turnOff();

    void updateTickCount();

    void setupCounter();
    void startCounter();
    void stopCounter();
    bool isRunning();

    static void zeroDetected();
    static void timeout();

    unsigned long syncDelay  = 0;
    unsigned long triacDelay = 0;
    unsigned long triacMax   = 0;

    volatile uint16_t tickCount = 0;
};

extern Triac _triac;

#endif // TRIAC_H