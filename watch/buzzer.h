#ifndef BUZZER_H
#define BUZZER_H
#include"data_type.h"

enum{
    DUMMY,
    DO,
    RE,
    MI,
    FA,
    SON,
    LA, 
    SI,
    DOS,
}; 

typedef struct Note
{
    uint8_t note;
    uint16_t duration; 
}; 

typedef struct Buzzer
{
    uint8_t _pin;
    uint8_t status; 
    uint8_t volume;
    unsigned long buzzerEndTime;
    callBack onStart; 
    callBack onEnd;
};

void buzzer_init(Buzzer *buzz, uint8_t pin ); 
void buzzer_setCallbackOnStart(Buzzer *buzz, callBack onstart);
void buzzer_setCallbackOnEnd(Buzzer *buzz, callBack onend);
void buzzer_setVolume(Buzzer *buzz, uint8_t volume);
void buzzer_Start(Buzzer *buzz,  uint16_t freq, uint16_t duration);
void buzzer_Control(Buzzer *buzz);
void buzzer_Stop(Buzzer *buzz);
#endif