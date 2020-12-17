#include "controller.h"
#include "thermometer.h"
#include "triac.h"

Controller _controller;

/**
 * @brief Setups everything, sensor, screen, timers, etc...
 */
void Controller::setup()
{
    delay(500);
    
    // Setup UI
    _ui.setup();
    _ui.drawLoadingScreen();

    // Setup Thermo
    EEPROM.get(BIAS_ADDR, _thermo.bias);
    _thermo.setup();

    EEPROM.get(IDEAL_ADDR, ideal);

    // Setup Triac
    _triac.setup();

    // Setup Relay
    pinMode(RELAY_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    // Setup Timers
    screenTimer.setDeadline(1, 0);
    screenTimer.restart();

    unsigned long z;
    EEPROM.get(TIMER_ADDR, z);
    setTimer(0, 0, z);
    resetTimer();

    turnOn();
}

/**
 * @brief called on every loop. Update Ui and temperature state
 */
void Controller::update()
{
    updateTemperature();
    processButtonPressed();
    updateUI();
}

/**
 * @brief Reads the temperature and adapt the heat power consequently.
 * 
 * If the timer expired, turns the heat off.
 */
void Controller::updateTemperature()
{
    _thermo.update();

    if(thermoTimer.hasExpired())
    {
        if(isTurnedOn)
            turnOff();
    }
    else
    {
        if(!isTurnedOn)
            turnOn();

             if (shouldWarmUp())   warmUp();
        else if (shouldCoolDown()) coolDown();
    }
}

/**
 * @brief Called when a button has been pressed. Set the current controller state
 * depending on the button pressed.
 */
void Controller::processButtonPressed()
{
    if(_ui.hasButtonBeenPressed())
    {
        bool expired = screenTimer.hasExpired();

        screenTimer.restart();
        if(expired)
        {
            _ui.resetButtons();
            return;
        }

        switch(state)
        {
        case Idle:
            state = MenuSetTemp;
            break;

        case MenuSetTemp:
            processMenu(SetTemp, [=](){idealTmp = ideal;});
            break;
        
        case MenuSetTempBias:
            processMenu(SetTempBias, [=](){biasTmp = _thermo.bias;});
            break;

        case MenuSetTime:
            processMenu(SetTime,
                        [=](){
                            unsigned int m, s;
                            Timer::toMinSec(thermoTimer.deadline, m, s);
                            timerTmp = m;
                        });
            break;

        case MenuResetTime:
            processMenu(Idle, [=](){resetTimer();});
            break;

        case MenuShowLoading:
            processMenu(LoadingScreen);
            break;

        case MenuDebug:
            processMenu(Debug);
            break;

        case MenuReturn:
            processMenu(Idle);
            break;

        case SetTemp:
            processActions([=](){idealTmp--; if(idealTmp < 0)  idealTmp = 99;},
                           [=](){idealTmp++; if(idealTmp > 99) idealTmp = 0;},
                           [=]()
                           {
                               setIdeal(idealTmp);
                               state = Idle;
                           });
            break;

        case SetTempBias:
            processActions([=](){biasTmp-=0.5; if(biasTmp < -5) biasTmp =  5;},
                           [=](){biasTmp+=0.5; if(biasTmp >  5) biasTmp = -5;},
                           [=]()
                           {
                               _thermo.bias = biasTmp;
                               EEPROM.update(BIAS_ADDR, _thermo.bias);
                               state = Idle;
                           });
            break;

        case SetTime:
            processActions([=](){timerTmp--; if(timerTmp < 0)  timerTmp = 99;},
                           [=](){timerTmp++; if(timerTmp > 99) timerTmp = 0;},
                           [=]()
                           {
                               setTimer(timerTmp);
                               resetTimer();
                               state = Idle;
                           });
            break;

        default:
            state = Idle;
            break;
        }

        _ui.resetButtons();
    }
}

/**
 * @brief Updates what is drawn on screen depending on the controller state.
 */
void Controller::updateUI()
{
    if (screenTimer.hasExpired())
        _ui.turnOff();
    else
    {
        _ui.turnOn();

        switch(state)
        {
        default:
        case Idle:
            _ui.drawIdleScreen();
            break;

        case MenuSetTemp:
        case MenuSetTempBias:
        case MenuSetTime:
        case MenuResetTime:
        case MenuShowLoading:
        case MenuDebug:
        case MenuReturn:
            _ui.drawMenuScreen();
            break;

        case SetTemp:
            _ui.drawSetTempScreen();
            break;

        case SetTempBias:
            _ui.drawSetTempBiasScreen();
            break;

        case SetTime:
            _ui.drawSetTimeScreen();
            break;

        case LoadingScreen:
            _ui.drawLoadingScreen();
            break;

        case Debug:
            _ui.drawDebugScreen();
            break;
        }
    }
}

/**
 * @brief This si an overloaded function.
 */
void Controller::processPages(int start, int end, int accept)
{
    processPages(start, end, accept, [](){});
}

/**
 * @brief This si an overloaded function.
 */
void Controller::processMenu(int accept)
{
    processMenu(accept, [](){});
}

/**
 * @brief Increase the heat power
 */
void Controller::warmUp()
{
    _triac.decDelay(10);
}

/**
 * @brief Decrease the heat power
 */
void Controller::coolDown()
{
    _triac.incDelay(10);
}

/**
 * @brief Cut the main line off
 */
void Controller::turnOff()
{
    _triac.turnOff();

    // Switch the relay off
    digitalWrite(RELAY_PIN, LOW);
    delay(500);
    digitalWrite(LED_PIN, LOW);

    isTurnedOn = false;
}

/**
 * @brief Turn on the main line
 */
void Controller::turnOn()
{
    _triac.detectSync();
    _triac.turnOn();

    delay(1000);
    // Switch the relay on
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    delay(500);

    isTurnedOn = true;
}

/**
 * @brief Set and save the ideal temperature
 */
void Controller::setIdeal(int i)
{
    if(ideal != i)
    {
        ideal = i;
        EEPROM.put(IDEAL_ADDR, ideal);
    }
}

/**
 * @brief Returns true if it should be warmer
 */
bool Controller::shouldWarmUp()   const
{
    return _thermo.temperature < ideal;
}

/**
 * @brief Returns true if it should be colder
 */
bool Controller::shouldCoolDown() const
{
    return _thermo.temperature > ideal;
}

/**
 * @brief Set the main timer deadline
 */
void Controller::setTimer(unsigned int m, unsigned int s, unsigned long z)
{
    unsigned long z2 = Timer::fromMinSec(m, s, z);
    if(thermoTimer.deadline != z2)
    {
        thermoTimer.setDeadline(z2);
        EEPROM.put(TIMER_ADDR, z2);
    }
}

/**
 * @brief restart the main timer
 */
void Controller::resetTimer()
{
    thermoTimer.restart();
}