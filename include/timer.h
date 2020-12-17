#ifndef TIMER_H
#define TIMER_H

/**
 * @brief The Timer class is a simple time counter.
 * 
 * It counts milliseconds elapsed since the restart() function has been called
 */
class Timer
{
public:
    constexpr Timer() = default;

    void restart();

    unsigned long elapsedTime() const;

    void elapsedTime(unsigned int& m,
                     unsigned int& s,
                     unsigned int& z) const;
    
    void elapsedTime(unsigned int& m,
                     unsigned int& s) const;

    static unsigned long fromMinSec(unsigned int m,
                                    unsigned int s,
                                    unsigned long z = 0);

    static unsigned int toMinSec(unsigned long z,
                                 unsigned int& m,
                                 unsigned int& s);

private:
    unsigned long start = 0;;
};



// =============================================================================



/**
 * @brief The DeadlineTimer class is an extension of the basic timer.
 * 
 * It provides countdown functionality. It has a deadline of X milliseconds,
 * when these X milliseconds elapsed after calling restart(), it is considered
 * expired.
 */
class DeadlineTimer : public Timer
{
public:
    constexpr DeadlineTimer() = default;

    void setDeadline(unsigned long z);

    void setDeadline(unsigned int m,
                     unsigned int s,
                     unsigned long z = 0);

    unsigned long remainingTime() const;
    void remainingTime(unsigned int& m,
                       unsigned int& s,
                       unsigned int& z) const;

    void remainingTime(unsigned int& m,
                       unsigned int& s) const;

    bool hasExpired() const;

    unsigned long deadline = 0;
};



// =============================================================================



/**
 * @brief The MicroTimer class is a simple time counter of microseconds.
 * @see Timer
 */
class MicroTimer
{
public:
    constexpr MicroTimer() = default;

    void restart();

    unsigned long elapsedTime() const;

private:
    unsigned long start = 0;
};



// =============================================================================



/**
 * @brief The MicroDeadlineTimer is the same as its sister DeadlineTimer but with
 * microseconds
 */
class MicroDeadlineTimer : public MicroTimer
{
public:
    constexpr MicroDeadlineTimer() = default;

    void setDeadline(unsigned long us);

    unsigned long remainingTime() const;

    bool hasExpired() const;

    unsigned long deadline = 0;
};

#endif // IMER_H