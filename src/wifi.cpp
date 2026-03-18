#include <WiFi.h>
#include "TcpServer.h"
#include <ESPmDNS.h>
#include "globals.h"

const char* ssid = "Gadderbaum";
const char* password = "isfibeTNG";

TcpServer tcpServer(888);   // Listen on port 888

int status;

//typedef enum {
//WL_NO_SHIELD = 255,
//      WL_IDLE_STATUS = 0,
//      WL_NO_SSID_AVAIL,
//      WL_SCAN_COMPLETED,
//      WL_CONNECTED,
//      WL_CONNECT_FAILED,
//      WL_CONNECTION_LOST,
//      WL_DISCONNECTED
//} wl_status_t;

void setupWifi() {
  int status = WiFi.status();
  Serial.print("Wifi Startstate: ");
  Serial.println(status);
  WiFi.begin(ssid, password);

  if (MDNS.begin(STAFFNAME)) {
    Serial.println("mDNS gestartet");
	MDNS.addService("arcane", "tcp", 888);
  }
}

void loopWifi() {
  int statusNow = WiFi.status();
  //status change
  if (statusNow != status) {
    Serial.print("Wifi status changed: ");
    Serial.println(statusNow);
    if (statusNow == WL_CONNECTED) {
      Serial.print("Wifi Connected! IP: ");
      Serial.println(WiFi.localIP());
      tcpServer.begin();
    }
    if (statusNow == WL_DISCONNECTED) {
      Serial.println("Wifi Disconnected!\n");
    }

    status = statusNow;
  }

  if (status == WL_CONNECTED) {
    tcpServer.handleClient();
  }
}

void turnOffWiFi() {
  Serial.println("Turning off WiFi...");
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void turnOnWiFi() {
  WiFi.mode(WIFI_STA);
}