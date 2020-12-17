#include "thermometer.h"
#include <Arduino.h>
#include <math.h>

Thermometer _thermo;

/**
 * @brief Initialise the temperature sensor
 */
Thermometer::Thermometer() :
    _one(TEMP_PIN),
    _dallas(&_one)
{
    
}

/**
 * @brief Starts the temperature sensor communication
 */
void Thermometer::setup()
{
    pinMode(TEMP_PIN, INPUT_PULLUP);

    if(isnan(bias) || isinf(bias) || !(bias >= -5.0f && bias <= 5.0f))
        bias = 0;

    _dallas.begin();

    // These 2 lines allow us to do async readings.
    // It sounds complex but it's actually pretty simple
    // You just need to check the convertion is completed before you read a
    // value. Instead of letting the library waiting and doing nothing, The
    // controller can do other meaningful things in the meantime.
    _dallas.setCheckForConversion(true);
    _dallas.setWaitForConversion(false);
    _dallas.getAddress(_addr, 0);

    delay(500);
}

/**
 * @brief Checks if the conversion is finished. If it is the case, read the
 * value and ask for a new conversion. If not, just pass and continue do useful
 * stuff.
 */
void Thermometer::update()
{
    if(_dallas.isConversionComplete())
    {
        temperature = _dallas.getTempC(_addr) + bias;
        _dallas.requestTemperaturesByAddress(_addr);
    }
}
