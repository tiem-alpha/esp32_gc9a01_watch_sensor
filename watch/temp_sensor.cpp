#include "temp_sensor.h"
#include "draw_util.h"
#include <Wire.h>
#include "Adafruit_SHT31.h"
#define SENSOR_INTERVAL_READ_MS 1000
float t; //= sht31.readTemperature();
float h; //= sht31.readHumidity();

Adafruit_SHT31 sht31 = Adafruit_SHT31();
char tempStr[10];
char humStr[10];
uint16_t hum_x = 100;
uint16_t hum_y = 170;
uint16_t temp_x = 100;
uint16_t temp_y = 140;
void TempHumSensorInit()
{
    Serial.println("SHT31 test");
    if (!sht31.begin(0x44))
    { // Set to 0x45 for alternate i2c addr
        Serial.println("Couldn't find SHT31");
        //    while (1) delay(1);
    }
}

unsigned long tempStamp = millis();
void TempHumRead()
{
    if (millis() - tempStamp >= SENSOR_INTERVAL_READ_MS)
    {
        tempStamp = millis();
        t = sht31.readTemperature();
        h = sht31.readHumidity();

        if (!isnan(t))
        { // check if 'is not a number'
            //        Serial.print("Temp *C = ");
            //        Serial.print(t);
            //        Serial.print("\t\t");
            int temptem = (int)t;
            memset(tempStr, 0, 10);
            sprintf(tempStr, "%d°C", temptem);
            // Serial.println(tempStr);
        }
        else
        {

            Serial.println("Failed to read temperature");
        }

        if (!isnan(h))
        { // check if 'is not a number'
            //        Serial.print("Hum. % = ");

            int temphum = (int)h;
            memset(humStr, 0, 10);
            sprintf(humStr, "%d%%", temphum); // %% để in ra dấu %

            // Serial.println(humStr);
        }
        else
        {
            Serial.println("Failed to read humidity");
        }
    }
}

void setTempTextPos(uint16_t x, uint16_t y)
{
    // hum_x = x;
    // hum_y = y;
    temp_x = x; 
    temp_y = y;
}

void setHumTextPos(uint16_t x, uint16_t y)
{
    hum_x = x;
    hum_y = y;
}

void drawSensorInfor()
{
    DrawString(temp_x, temp_y, tempStr, WHITE);
    DrawString(hum_x, hum_y, humStr, WHITE);
}