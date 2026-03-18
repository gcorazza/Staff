#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H
#include <WiFi.h>

// WiFi credentials
extern const char* ssid;
extern const char* password;

// WiFi status tracker
extern int wifiStatus;

// Functions
void setupWifi();
void loopWifi();
void turnOffWiFi();
void turnOnWiFi();

#endif

