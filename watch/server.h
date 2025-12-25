#ifndef SERVER_H
#define SERVER_H
#include"data_type.h"

uint8_t getConnected();
void setConnected(uint8_t connected);
void initServer();
void notifyClients();
void ConnectedUploadEvent();
void ping();
void keepConnect(); 
#endif