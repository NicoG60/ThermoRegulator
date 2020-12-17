#include "timer.h"
#include <Arduino.h>

/**
 * @brief Restart the counter
 */
void Timer::restart()
{
    start = millis();
}

/**
 * @brief Returns the number of milliseconds elapsed since restart() has been
 * called
 */
unsigned long Timer::elapsedTime() const
{
    return millis() - start;
}

/**
 * @brief Returns the time elapsed since restart() has been called.
 * Returns it in minutes @a m, seconds @a s and milliseconds @a z
 */
void Timer::elapsedTime(unsigned int& m,
                        unsigned int& s,
                        unsigned int& z) const
{
    z = toMinSec(elapsedTime(), m, s);
}

/**
 * @brief Returns the time elapsed since restart() has been called.
 * Returns it in minutes @a m, seconds @a s
 */
void Timer::elapsedTime(unsigned int& m,
                        unsigned int& s) const
{
    toMinSec(elapsedTime(), m, s);
}

/**
 * @brief accumulate @a m minutes, @a s seconds and @a z milliseconds into a number
 * of milliseconds
 */
unsigned long Timer::fromMinSec(unsigned int m,
                                unsigned int s,
                                unsigned long z)
{
    return m*60000 + s*1000 + z;
}

/**
 * @brief Breakdown @a z milliseconds into @a m minutes and @a s seconds. Retruns
 * the remaining milliseconds
 */
unsigned int Timer::toMinSec(unsigned long z,
                             unsigned int& m,
                             unsigned int& s)
{
    auto rm = ldiv(z, 60000);
    auto rs = ldiv(rm.rem, 1000);

    m = rm.quot;
    s = rs.quot;
    return rs.rem;
}



// =============================================================================



/**
 * @brief Sets the number of milliseconds before the timer expire
 */
void DeadlineTimer::setDeadline(unsigned long z)
{
    deadline = z;
}

/**
 * @brief Sets the time before the timer expire
 */
void DeadlineTimer::setDeadline(unsigned int m,
                                unsigned int s,
                                unsigned long z)
{
    deadline = fromMinSec(m, s, z);
}

/**
 * @brief Returns the remaining milliseconds before the timer expires
 */
unsigned long DeadlineTimer::remainingTime() const
{
    unsigned long t = elapsedTime();
    if(t >= deadline)
        return 0;
    else
        return deadline - t;
}

/**
 * @brief Breakdown the remaining milliseconds into @a m minutes, @a s seconds,
 * @a z milliseconds
 */
void DeadlineTimer::remainingTime(unsigned int& m,
                                  unsigned int& s,
                                  unsigned int& z) const
{
    z = toMinSec(remainingTime(), m, s);
}

/**
 * @brief Breakdown the remaining milliseconds into @a m minutes and @a s seconds
 */
void DeadlineTimer::remainingTime(unsigned int& m,
                                  unsigned int& s) const
{
    toMinSec(remainingTime(), m, s);
}

/**
 * @brief Returns whether the timer expires
 */
bool DeadlineTimer::hasExpired() const
{
    return elapsedTime() >= deadline;
}



// =============================================================================



/**
 * @brief Restart the counter
 */
void MicroTimer::restart()
{
    start = micros();
}

/**
 * @brief Returns the number of microseconds elapsed since restart() has been
 * called
 */
unsigned long MicroTimer::elapsedTime() const
{
    return micros() - start;
}



//==============================================================================



/**
 * @brief Sets the number of microseconds before the timer expire
 */
void MicroDeadlineTimer::setDeadline(unsigned long us)
{
    deadline = us;
}

/**
 * @brief Returns the remaining microseconds before the timer expires
 */
unsigned long MicroDeadlineTimer::remainingTime() const
{
    unsigned long t = elapsedTime();
    if(t >= deadline)
        return 0;
    else
        return deadline - t;
}

/**
 * @brief Returns whether the timer expires
 */
bool MicroDeadlineTimer::hasExpired() const
{
    return elapsedTime() >= deadline;
}
