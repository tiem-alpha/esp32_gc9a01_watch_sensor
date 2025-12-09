#include <SPI.h>
#include <WiFi.h>
#include <time.h>
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include "draw_watch.h"
#include "custom_button.h"
#include "custom_timer.h"
#include "custom_led.h"
#include "config.h"

// Thông tin WiFi
const char *ssid = "Tiem";
const char *password = "11111111";

// Thông tin NTP Server
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200; // UTC+7 (Việt Nam)
const int daylightOffset_sec = 0;

// Biến lưu thời gian
int hour, minute, second;
int lastSecond = -1;
uint8_t state_led = 0;
float t; //= sht31.readTemperature();
float h; //= sht31.readHumidity();

Adafruit_SHT31 sht31 = Adafruit_SHT31();

MYLED led = {
    .PIN = LED
};

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
    { Serial.println("Long Press"); }};

void TempHumSensorInit()
{
  Serial.println("SHT31 test");
  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    //    while (1) delay(1);
  }
}

void updateTime()
{
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Không thể lấy thời gian");
    return;
  }

  hour = timeinfo.tm_hour % 12;
  minute = timeinfo.tm_min;
  second = timeinfo.tm_sec;
}

void TempHumRead()
{
   t = sht31.readTemperature();
    h = sht31.readHumidity();

    if (!isnan(t))
    { // check if 'is not a number'
      Serial.print("Temp *C = ");
      Serial.print(t);
      Serial.print("\t\t");
    }
    else
    {
      Serial.println("Failed to read temperature");
    }

    if (!isnan(h))
    { // check if 'is not a number'
      Serial.print("Hum. % = ");
      Serial.println(h);
    }
    else
    {
      Serial.println("Failed to read humidity");
    }
}


void Concurrent()
{
  // Cập nhật thời gian
  updateTime();
  MYLEDControl(&led);
  MyButtonControl(btn);
  // Chỉ vẽ lại khi giây thay đổi
  if (second != lastSecond)
  {

    TempHumRead();

    drawClockHands(hour, minute, second);
    // DisplayBuffers();

    lastSecond = second;
  }
}

void setup()
{
  Serial.begin(115200);
  MYLEDInit(&led);
  MyButtonInit(btn);
  TempHumSensorInit();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối WiFi");

  // Cấu hình thời gian
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  // Thêm số giờ
  watchInit();
}

void loop()
{

  Concurrent();
//  delay(100);
}
