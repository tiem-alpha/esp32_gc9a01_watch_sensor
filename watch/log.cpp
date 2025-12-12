#include"log.h"

#define LOG_MAX_SIZE 100

char logStr[LOG_MAX_SIZE]; 


void WriteLog(char*buff, uint8_t len){
    memset(logStr,0,LOG_MAX_SIZE);
    memcpy(logStr,buff,len);
}

char*getLog(){
    return logStr;
}