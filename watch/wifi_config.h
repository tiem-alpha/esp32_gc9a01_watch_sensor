
You sent
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H
#include"data_type.h"

bool WifiConfigInit(callBack wifiConfig);
void WifiConfigRun();
void clearWifiConfig();
void startConfigPortal();
uint8_t waitWifiDone();
#endif