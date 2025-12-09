#ifndef BUTTON_H

#define BUTTON_H
#include<Arduino.h>
#include"custom_timer.h"

// Timing
#define DEBOUND_TIME_MS  50       // thời gian debounce
#define DOUBLE_CLICK_TIME_MS  300   // tối đa 300ms để double click
#define LONG_CLICK_TIME_MS  800     // ấn giữ trên 800ms là long press

//void (*callBack)(void); 
#define IDLE_STATE LOW
#define PRESS_STATE HIGH
enum{
    IDLE,
    CLICK,
    RELEASE,
    CLICK2,
    RELEASE2,
};

 typedef void (*button_callback)();

struct MyButton {
    uint8_t pin;

    // State
    bool currentState = IDLE_STATE;
    bool lastStableState = IDLE_STATE;
    uint32_t lastDebounceTime = 0;

    bool isPressed = false;
    uint32_t pressedTime = 0;
    uint32_t releasedTime = 0;

    bool waitingSecondClick = false;

    // Callbacks
    button_callback onClick = nullptr;
    button_callback onDoubleClick = nullptr;
    button_callback onLongPress = nullptr;
};

void MyButtonInit(MyButton &button);
void MyButtonSetClickCallback(MyButton &btn, button_callback callback);
void MyButtonSetDoubleClickCallback(MyButton &btn, button_callback callback);
void MyButtonSetLongPressCallback(MyButton &btn, button_callback callback);
void MyButtonControl(MyButton &btn);

#endif
