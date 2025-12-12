#ifndef MYLED_H
#define MYLED_H
#include"data_type.h"
#include"custom_timer.h"

#define MYLED_AFFECT_TIME_US 10000 

struct MYLED{
    uint8_t current; 
    uint8_t target;
    uint8_t status;
    int PIN; 
    MyTimer timer; 
};

void MYLEDInit(MYLED*led);
void MYLEDSet(MYLED *led, uint8_t percent);
void MYLEDControl(MYLED*led);
#endif
