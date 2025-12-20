#include "custom_button.h"
#include "custom_timer.h"

void MyButtonInit(MyButton &button)
{
    pinMode(button.pin, INPUT);
    //  ResetTime(&button->time);
    //   button->status = digitalRead(button->PIN);
}

void MyButtonSetClickCallback(MyButton &btn, button_callback callback)
{
    btn.onClick = callback;
}

void MyButtonSetDoubleClickCallback(MyButton &btn, button_callback callback)
{
    btn.onDoubleClick = callback;
}

void MyButtonSetLongPressCallback(MyButton &btn, button_callback callback)
{
    btn.onLongPress = callback;
}

void MyButtonSetFSPressCallback(MyButton &btn, button_callback callback)
{
    btn.onFSPress = callback;
}

void MyButtonControl(MyButton &btn)
{
    uint32_t now = millis();
    bool reading = digitalRead(btn.pin); // HIGH = released, LOW = pressed

    // ------------------ DEBOUNCE ------------------
    if (reading != btn.currentState)
    {
        btn.lastDebounceTime = now;
        btn.currentState = reading;
    }

    if ((now - btn.lastDebounceTime) < DEBOUND_TIME_MS)
    {
        return; // chưa ổn định -> thoát, non-blocking
    }

    // ------------------ STATE MACHINE ------------------
    // Nhấn nút
    if (btn.currentState == PRESS_STATE && btn.lastStableState == IDLE_STATE)
    {
        btn.isPressed = true;
        btn.pressedTime = now;
    }

    // Thả nút
    if (btn.currentState == IDLE_STATE && btn.lastStableState == PRESS_STATE)
    {
        btn.isPressed = false;
        btn.releasedTime = now;

        uint32_t pressDuration = btn.releasedTime - btn.pressedTime;
        if (pressDuration >= FS_CLICK_TIME_MS)
        { // factory reset
             if (btn.onFSPress)
                    btn.onFSPress();
            btn.waitingSecondClick = false;
        }
        else
        {
            // Long press
            if (pressDuration >= LONG_CLICK_TIME_MS)
            {
                if (btn.onLongPress)
                    btn.onLongPress();
                btn.waitingSecondClick = false;
            }
            else
            {
                // Single / double click logic
                if (!btn.waitingSecondClick)
                {
                    btn.waitingSecondClick = true; // Chờ click thứ 2
                }
                else
                {
                    // double click
                    if (now - btn.releasedTime <= DOUBLE_CLICK_TIME_MS)
                    {
                        if (btn.onDoubleClick)
                            btn.onDoubleClick();
                        btn.waitingSecondClick = false;
                    }
                }
            }
        }
    }

    // Timeout double click → single click
    if (btn.waitingSecondClick &&
        (now - btn.releasedTime > DOUBLE_CLICK_TIME_MS))
    {

        if (btn.onClick)
            btn.onClick();
        btn.waitingSecondClick = false;
    }

    btn.lastStableState = btn.currentState;
}
