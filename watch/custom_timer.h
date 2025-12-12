#ifndef TIMER_H
#define TIMER_H

#include"data_type.h"

 struct MyTimer{
    uint32_t value;
}; 

void ResetTime(MyTimer*timer);
uint8_t CheckPassUS(MyTimer*timer, uint32_t time);
uint8_t CheckPassMS(MyTimer*timer, uint32_t time);
uint8_t CheckPassS(MyTimer*timer, uint32_t time);
#endif
