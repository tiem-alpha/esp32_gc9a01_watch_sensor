#include "watch.h"
#include "draw_util.h"
#include "draw_watch.h"
#include "server.h"
#include "temp_sensor.h"
#include "image_data.h"
#include "image_eight_data.h"

// Thông tin NTP Server
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200; // UTC+7 (Việt Nam)
const int daylightOffset_sec = 0;

// Biến lưu thời gian
int hour, minute, second;
char date[21];

static unsigned long stamp;
static uint8_t isConnected;
static uint8_t isHaveTime;
static uint8_t getDate = 0;
time_t now;

const char *thu_vn[] = {
    "CN", "T2", "T3",
    "T4", "T5", "T6", "T7"};

static uint8_t getTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Không thể lấy thời gian");
        return 0;
    }

    hour = timeinfo.tm_hour; //% 12;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    stamp = millis();
    now = mktime(&timeinfo);
    // Serial.println(now);
    return 1;
}

void initTime()
{
    watchInit();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if (getConnected() == 1)
    {
        for (int i = 0; i < 20; i++)
        {
            if (getTime() == 1)
            {
                isHaveTime = 1;

                break;
            }
            delay(100);
        }
    }
}

void updateTime()
{
    if (getConnected() == 1)
    {
        if (isHaveTime == 0)
        {
            if (getTime() == 1)
            {
                isHaveTime = 1;
            }
        }
        else
        {
            if (millis() - stamp >= 1000)
            {

                uint32_t temp = (millis() - stamp) / 1000; // ms
                now += temp;
                stamp += temp * 1000;
                struct tm *timeinfo = localtime(&now);

                // char buffer[30];

                hour = timeinfo->tm_hour; // % 12;
                if (hour == 0)
                {
                    getDate = 0;
                }
                if (getDate == 0)
                {
                    // strftime(date, sizeof(date), "%A, %d/%m/%Y", timeinfo);
                    snprintf(date, sizeof(date),
                             "%s, %02d/%02d/%04d",
                             thu_vn[timeinfo->tm_wday],
                             timeinfo->tm_mday,
                             timeinfo->tm_mon + 1,
                             timeinfo->tm_year + 1900);
                    getDate = 1;
                }
                // Serial.println(date);
                minute = timeinfo->tm_min;
                second = timeinfo->tm_sec;
            }
        }
    }
}

void drawTimeAnalog()
{
    hour = hour % 12;
    drawClockHands(hour, minute, second);
}

void drawDigital()
{
    uint8_t padding = padding;
    uint8_t hh = hour / 10;
    uint8_t hl = hour % 10;
    uint8_t minh = minute / 10;
    uint8_t minl = minute % 10;
    uint8_t sech = second / 10;
    uint8_t secl = second % 10;
    int x = 20;
    int y = 90;
    if (hh != 0)
        Draw8bitImageProgmemNoBG(x, y, images8[hh]);
    x += images8[hh].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[hl]);
    x += images8[hh].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[10]);
    x += images8[10].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[minh]);
    x += images8[minh].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[minl]);
    x += images8[minl].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[10]);
    x += images8[10].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[sech]);
    x += images8[sech].width + padding;
    Draw8bitImageProgmemNoBG(x, y, images8[secl]);
    DrawSmallString(60, 20, date, WHITE);
}