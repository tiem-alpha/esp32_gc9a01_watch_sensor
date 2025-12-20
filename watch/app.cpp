#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <Arduino.h>
#include <Wire.h>

#include "draw_watch.h"
#include "custom_button.h"
#include "custom_timer.h"
#include "custom_led.h"
#include "config.h"
#include "app.h"
#include "draw_util.h"
#include "watch.h"
#include "wifi_config.h"
#include "temp_sensor.h"
#include "output.h"

uint8_t state_led = 0;
static uint8_t stateMachine = 0;
MYLED led = {
    .PIN = LED};

MyButton btn = {
    .pin = BUTTON,
    .onClick = []()
    { 
      state_led = state_led ==0 ?100:0;
      MYLEDSet(&led,state_led);
      Serial.println("Single Click"); },
    .onDoubleClick = []()
    { Serial.println("Double Click"); },
    .onLongPress = []()
    { Serial.println("Long Press"); },
    .onFSPress = []()
    {
        Serial.println("Factory Reset"); 
        clearWifiConfig();
        ESP.restart(); 
    }
};

enum
{
    CONFIG_WIFI,
    CONNECT_WIFI,
    WATCH,
    FOTA,
};

void Concurrent()
{
    // Cập nhật thời gian
    WifiConfigRun();
    MYLEDControl(&led);
    MyButtonControl(btn);
    TempHumRead();
}

void changeToWaitWifi()
{
    stateMachine = CONNECT_WIFI;
}

void appInit()
{
    Serial.begin(115200);
    MYLEDInit(&led);
    MyButtonInit(btn);
    TempHumSensorInit();
    initTime();
    if (WifiConfigInit(changeToWaitWifi))
    {
        stateMachine = WATCH;
    }
    else
    {
        stateMachine = CONFIG_WIFI;
    }
}

void appRun()
{
    // switch()
    Concurrent();
    // static uint8_t count =0;
    static unsigned long timeStamp = millis();
    switch (stateMachine)
    {
    case CONFIG_WIFI:
        startConfigPortal();
        break;

    case CONNECT_WIFI:

        switch (waitWifiDone())
        {
        case 1:
            stateMachine = WATCH;
            //            Serial.println(
            break;

        case 2:
            stateMachine = CONFIG_WIFI;
            break;

        default:
            break;
        }
        break;

    case WATCH:
        if (millis() - timeStamp >= 1000)
        {
            //   Serial.println("watch state");
            drawBackGround(FACE1);
            updateTime();
            drawSensorInfor();
            // drawTimeAnalog();
            drawDigital();

            timeStamp = millis();
        }

        break;

    case FOTA:
        break;

    default:
        break;
    }
    DisplayBuffers();
}
