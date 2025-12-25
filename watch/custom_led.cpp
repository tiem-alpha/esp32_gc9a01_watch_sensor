#include "custom_led.h"
void MYLEDInit(MYLED *led)
{
    pinMode(led->PIN, OUTPUT);
    led->target = 0;
}

void MYLEDSet(MYLED *led, uint8_t percent)
{
    // pinMode(led->PIN,OUTPUT);
    if(percent>=100){
      percent = 100; 
    }else if (percent<=0)
    {
      percent =0;
    }
    Serial.printf("Set Led %d\n",percent);
    led->target = percent;
    ResetTime(&led->timer);
}

void MYLEDControl(MYLED *led)
{
  if (led->current == led->target) return;
    if (CheckPassUS(&led->timer, MYLED_AFFECT_TIME_US) == 1)
    {
        if (led->current < led->target)
        {
            led->current++;
        }
        else if (led->current > led->target)
        {
            led->current--;
        }
        else
        {
          
        }
        
    }
//    if(led->current == 0){
//        digitalWrite(led->PIN, 0);
//        Serial.println("bang 0");
//    }
//    else if(led->current ==100)
//    {
//        digitalWrite(led->PIN, 1);
//         Serial.println("bang 100");
//    }
//    else
//    {
        uint8_t  value = (led->current*255/100); 
        analogWrite(led->PIN, value);
        //  Serial.printf("bang analog %d\n",value);
//    }
}
