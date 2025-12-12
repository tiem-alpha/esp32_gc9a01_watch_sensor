#include "server.h"
uint8_t isConnected =0; 

uint8_t getConnected(){
    return isConnected;
}

void setConnected(uint8_t connected){
    isConnected = connected; 
}