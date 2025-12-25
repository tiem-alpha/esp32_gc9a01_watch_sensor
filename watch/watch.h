#ifndef WATCH_H
#define WATCH_H
#include "data_type.h"
void initTime();
void updateTime();
void drawTimeAnalog();
void drawDigital();
uint8_t getAnalog();
void drawWatchFace();
void setAnalog(uint8_t ana);
void setTime(time_t epoch);
void getCloudTime();
void setHourColor(uint16_t color);
void setMinColor(uint16_t color);
void setSecColor(uint16_t color);
void setSpaceColor(uint16_t color);
void setShowWatch(uint8_t set); 
uint8_t getShowWatch();
#endif