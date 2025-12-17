#ifndef DEVICE_H

#define DEVICE_H
#include"data_type.h"
#include"color.h"

enum WATH_TYPE{
    ANALOG,
    DIGITAL,
};


typedef struct Watch{
    uint8_t watchType;
    uint8_t watchEnable;
    uint16_t hourColor;
    uint16_t minColor;
    uint16_t secColor;
    uint16_t dateColor;
    uint8_t hourType;
    uint8_t minType;
    uint8_t secType;
    uint8_t datetype;
    uint8_t backGroundIndex; 
    uint16_t *backGround; 
}; 

#endif