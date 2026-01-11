#include "buzzer.h"
#include <Arduino.h>
#include "esp32-hal-ledc.h"
#define PWM_CHANNEL 0
#define PWM_RESOLUTION 8 // 8-bit (0–255)
#define DEFAULT_VOL 128

void buzzer_init(Buzzer *buzz, uint8_t pin)
{
    buzz->_pin = pin;
    buzz->volume = DEFAULT_VOL;
    buzz->status = 0;
   ledcAttach(buzz->_pin, 0, PWM_RESOLUTION);
    
}

void buzzer_setVolume(Buzzer *buzz, uint8_t volume)
{
    uint16_t temp = volume * 255 / 100;
    temp = temp > 255 ? 255 : temp;
    buzz->volume = temp;
}

void buzzer_setCallbackOnStart(Buzzer *buzz, callBack onstart)
{
    buzz->onStart = onstart;
}

void buzzer_setCallbackOnEnd(Buzzer *buzz, callBack onend)
{
    buzz->onEnd = onend;
}

void buzzer_Stop(Buzzer *buzz)
{
    ledcWrite(PWM_CHANNEL, 0);
    buzz->status = 0;
    if (buzz->onEnd)
    {
        buzz->onEnd();
    }
}

void buzzer_Start(Buzzer *buzz, uint16_t freq, uint16_t duration)
{

    
    // ledcWrite(PWM_CHANNEL, buzz->volume);
    ledcAttach(buzz->_pin,freq, PWM_RESOLUTION);
    ledcWrite(PWM_CHANNEL, buzz->volume);
    buzz->buzzerEndTime = millis() + duration;
    buzz->status = 1; // on
    if (buzz->onStart)
    {
        buzz->onStart();
    }
}

void buzzer_Control(Buzzer *buzz)
{
    if (buzz->status == 1 && millis() >= buzz->buzzerEndTime)
    {
        buzzer_Stop(buzz);
    }
}