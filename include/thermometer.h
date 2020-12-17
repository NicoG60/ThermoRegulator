#ifndef THERMOMETER_H
#define THERMOMETER_H

#include <OneWire.h>
#include <DallasTemperature.h>

#include "pins.h"
#include "utils.h"

#define N_AVG    100
#define MAX_READ 1024
#define A_REF    1.1

/**
 * @brief The Thermometer class is responsible for managing the temperature sensor.
 */
class Thermometer
{
public:
    Thermometer();

    void setup();
    void update();

    float temperature = 0;
    float bias = 0;

private:
    OneWire           _one;
    DallasTemperature _dallas;
    DeviceAddress     _addr = {0, 0, 0, 0, 0, 0, 0, 0};
};

extern Thermometer _thermo;

#endif // THERMOMETER_H