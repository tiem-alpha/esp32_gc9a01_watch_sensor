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
#include"draw_util.h"
#include "watch.h"
#include"wifi_config.h"
#include"temp_sensor.h"



uint8_t state_led = 0;

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
    { Serial.println("Long Press");
    clearWifiConfig();
    ESP.restart();
}};
enum
{
    CONFIG_WIFI,
    CONNECT_WIFI,
    WATCH,
    FOTA,
};
static uint8_t stateMachine = 0;

void Concurrent()
{
    // Cập nhật thời gian
    WifiConfigRun();
    updateTime();
    MYLEDControl(&led);
    MyButtonControl(btn);
    // Chỉ vẽ lại khi giây thay đổi
    // TempHumRead();
}

void appInit()
{
    Serial.begin(115200);
    MYLEDInit(&led);
    MyButtonInit(btn);
    TempHumSensorInit();
    WifiConfigInit();
    initTime();
}

void appRun()
{
    // switch()
    Concurrent();
    DisplayBuffers();
}