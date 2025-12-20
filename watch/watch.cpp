#include"watch.h"
#include"draw_util.h"
#include"draw_watch.h"
#include"server.h"
#include"temp_sensor.h"

// Thông tin NTP Server
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 25200; // UTC+7 (Việt Nam)
const int daylightOffset_sec = 0;

// Biến lưu thời gian
int hour, minute, second;


static unsigned long stamp;
static uint8_t isConnected;
static uint8_t isHaveTime;
time_t now;
static uint8_t getTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Không thể lấy thời gian");
        return 0;
    }

    hour = timeinfo.tm_hour % 12;
    minute = timeinfo.tm_min;
    second = timeinfo.tm_sec;
    stamp = millis();
     now  = mktime(&timeinfo);
    // Serial.println(now);
    return 1;
}

void initTime()
{
    watchInit();
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if (getConnected() == 1)
    {
        for(int i =0 ;i<20 ;i++)
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
              
                uint32_t temp = (millis() - stamp)/1000;//ms
                now += temp;
                stamp += temp*1000;
                struct tm* timeinfo = localtime(&now);

                char buffer[30];
                // strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
                // Serial.println(buffer);
                hour = timeinfo->tm_hour % 12;
                minute = timeinfo->tm_min;
                second = timeinfo->tm_sec;
            }
        }
    }
}


void drawTimeAnalog()
{
    drawClockHands(hour, minute, second);
}