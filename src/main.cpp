#include "controller.h"
#include <Arduino.h>

void setup()
{
    _controller.setup();
}

void loop()
{
    _controller.update();
}

// 0800 028 1456