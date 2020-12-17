#include "triac.h"
#include "utils.h"
#include <Arduino.h>

Triac _triac;

/**
 * @brief Initialise the triac control
 */
void Triac::setup()
{
    pinMode(TRIAC_PIN, OUTPUT);
    pinMode(INTER_PIN, INPUT);

    setupCounter();
}

/**
 * @brief Sets the delay to wait between the current crossing 0 and the triac
 * being driven.
 * 
 * More delay == less power
 */
void Triac::setDelay(unsigned int us)
{
    if(triacDelay != us && us <= triacMax)
    {
        triacDelay = us;
        updateTickCount();
    }
}

/**
 * @brief Increase the delay by @a a
 */
void Triac::incDelay(unsigned int a)
{
    triacDelay += a;
    if(triacDelay > triacMax)
        triacDelay = triacMax;

    updateTickCount();
}

/**
 * @brief Decrease the delay by @a a
 */
void Triac::decDelay(unsigned int a)
{
    if(triacDelay <= a)
        triacDelay = 0;
    else
        triacDelay -= a;

    updateTickCount();
}

/**
 * @brief Detect the offset of the zero crossing detector.
 * 
 * The electronic generate a short impusle when the current crosses 0. This
 * impulse starts a little bit before the actual zero and end a little bit after.
 * 
 *              ___
 * ____________|   |____________  <--- This is the zero detection signal
 * 
 * --------------|--------------  <--- This is a representation of when the 
 *                                     Current corsses 0
 * 
 * In theory, all impulses are exactly equal and you can take half of the
 * measured time to get the exact zero cross point.
 * 
 * In practice this is not true and your controller is not accurate enough for
 * that. So instead of halving the duration, we'll keep it. We'll even add a
 * little extra delay to be absolutely sure we will avoid flickers.
 * 
 * What you get is that
 * 
 *              ___                                ___
 * ____________|   |______________________________|   |_______________
 * 
 * --------------|----------------------------------|-----------------
 * 
 * =========|        |==========================|       |=============
 *                     \_ This is the actual available time to count
 *                        And it is way enough to drive a simple IR lamp.
 */
void Triac::detectSync()
{
    // Takes 100 pulse length and get the max one
    for(int i = 0; i < 100; i++)
    {
        unsigned long pulse = pulseIn(INTER_PIN, HIGH, 15000);

        if(pulse == 0)
            continue;

        if(pulse > syncDelay)
            syncDelay = pulse;
    }

    syncDelay += 300; // little extra to avoid timing issue

    // Update max delay allowed to avoid flicker
    triacMax = 10000-syncDelay;
    
    if(triacDelay > triacMax)
        triacDelay = triacMax;

    updateTickCount();
}

/**
 * @brief Setup the interrupts so the triac can be driven
 */
void Triac::turnOn()
{
    attachInterrupt(digitalPinToInterrupt(INTER_PIN),
                    Triac::zeroDetected,
                    RISING);
}

/**
 * @brief Remove the interrupt to avoid the triac being driven when not wanted
 */
void Triac::turnOff()
{
    detachInterrupt(digitalPinToInterrupt(INTER_PIN));
    digitalWrite(TRIAC_PIN, LOW);
}

/**
 * @brief Compute the number of ticks the hardware timer needs to count to
 * satisfy the delay we want to wait.
 */
void Triac::updateTickCount()
{
    tickCount = ((syncDelay + triacDelay)/4)-1;
}

/**
 * @brief Configures the hardware timer
 */
void Triac::setupCounter()
{
    // Setup the timer B2 with /64 prescaler.
    // Which is a ~4us ticks
    TCB2.CTRLA = TCB_CLKSEL_CLKTCA_gc;
    TCB2.CTRLB = TCB_CNTMODE_INT_gc & ~TCB_CCMPEN_bm;
    TCB2.INTFLAGS = TCB_CAPT_bm;
    TCB2.INTCTRL = TCB_CAPT_bm;
}

/**
 * @brief Restart the counter so it counts from 0 to tickCount and call
 * timeout() once it has done that.
 */
void Triac::startCounter()
{
    if(isRunning())
        return;

    TCB2.CCMP = tickCount;
    TCB2.CNT = 0;
    TCB2.CTRLA |= TCB_ENABLE_bm;
}

/**
 * @brief Prevents the counter from overflowing and start counting again.
 */
void Triac::stopCounter()
{
    TCB2.CTRLA &= ~TCB_ENABLE_bm;
}

/**
 * @brief Whether the counter is currently counting
 */
bool Triac::isRunning()
{
    return TCB2.CTRLA & TCB_ENABLE_bm ? true : false;
}

/**
 * @brief Interrupt called when the current crosses 0
 */
void Triac::zeroDetected()
{
    _triac.startCounter();
}

/**
 * @brief Interrupt called when the counter reached tickCount.
 */
void Triac::timeout()
{
    _triac.stopCounter();

    digitalWrite(TRIAC_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIAC_PIN, LOW);
}

/**
 * @brief ISR called when the counter finished counting.
 * It resets interrupt flags and call the proper interrupt method.
 */
ISR(TCB2_INT_vect)
{
  TCB2.INTFLAGS = TCB_CAPT_bm;
  _triac.timeout();
}