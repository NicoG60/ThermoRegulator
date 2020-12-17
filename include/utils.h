#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

namespace utils
{

inline constexpr double mapd(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

inline void invert(int pin)
{
    int s = digitalRead(pin);
    digitalWrite(pin, !s);
}

}

#endif // UTILS_H