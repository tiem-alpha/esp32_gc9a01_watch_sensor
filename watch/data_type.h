#ifndef DATA_TYPE_H
#define DATA_TYPE_H
#include<Arduino.h>
#define NULL 0
typedef void (*callBack)();
typedef struct {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
} Image4Bit;

typedef struct {
    const uint8_t* data;
    uint8_t width;
    uint8_t height;
} Image8Bit;

typedef struct {
    const uint16_t* data;
    uint8_t width;
    uint8_t height;
} Image565;
#endif