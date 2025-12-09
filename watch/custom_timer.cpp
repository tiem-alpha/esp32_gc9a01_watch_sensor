#include "custom_timer.h"

void ResetTime(MyTimer *timer)
{
    timer->value = micros();
}
uint8_t CheckPassUS(MyTimer *timer, uint32_t time)
{
    if (micros() - timer->value >= time)
    {
        timer->value = micros();
        return 1;
    }
    return 0;
}

uint8_t CheckPassMS(MyTimer *timer, uint32_t time)
{
    if (micros() - timer->value >= (uint32_t)(time * 1000))
    {
        timer->value = micros();
        return 1;
    }
    return 0;
}

uint8_t CheckPassS(MyTimer *timer, uint32_t time)
{
    if (micros() - timer->value >= (uint32_t)(time * 1000000))
    {
        timer->value = micros();
        return 1;
    }
    return 0;
}
