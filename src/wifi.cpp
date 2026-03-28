#include <WiFi.h>
#include "TcpServer.h"
#include <ESPmDNS.h>
#include "globals.h"
#include "src/LEDs.h"

const char* ssid = "Gadderbaum";
const char* password = "isfibeTNG";

TcpServer tcpServer(888);   // Listen on port 888

int status = WL_IDLE_STATUS;

namespace {
unsigned long lastRssiLog = 0;
constexpr unsigned long RSSI_LOG_INTERVAL_MS = 60000;

const char* statusToString(wl_status_t st) {
    switch (st) {
        case WL_IDLE_STATUS: return "IDLE";
        case WL_NO_SSID_AVAIL: return "NO_SSID";
        case WL_SCAN_COMPLETED: return "SCAN_DONE";
        case WL_CONNECTED: return "CONNECTED";
        case WL_CONNECT_FAILED: return "CONNECT_FAILED";
        case WL_CONNECTION_LOST: return "CONNECTION_LOST";
        case WL_DISCONNECTED: return "DISCONNECTED";
        default: return "UNKNOWN";
    }
}

void startWifiConnection(const char* reason) {
    Serial.print("WiFi reconnect requested: ");
    Serial.println(reason);
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
}
}

void setupWifi() {
  WiFi.mode(WIFI_STA);
  status = WiFi.status();
  Serial.print("Wifi Startstate: ");
  Serial.print(status);
  Serial.print(" (");
  Serial.print(statusToString(static_cast<wl_status_t>(status)));
  Serial.println(")");
  if (status != WL_CONNECTED) {
    startWifiConnection("startup");
  }

  if (MDNS.begin(STAFFNAME)) {
    Serial.println("mDNS gestartet");
    MDNS.addService("arcane", "tcp", 888);
  }
}

void loopWifi() {
  int statusNow = WiFi.status();
  if (statusNow != status) {
    Serial.print("Wifi status changed: ");
    Serial.print(statusNow);
    Serial.print(" (");
    Serial.print(statusToString(static_cast<wl_status_t>(statusNow)));
    Serial.println(")");

    if (statusNow == WL_CONNECTED) {
      Serial.print("Wifi Connected! IP: ");
      Serial.print(WiFi.localIP());
      Serial.print(" RSSI: ");
      Serial.println(WiFi.RSSI());
      tcpServer.begin();
    } else {
      switch (statusNow) {
        case WL_CONNECTION_LOST:
        case WL_DISCONNECTED:
        case WL_NO_SSID_AVAIL:
        case WL_CONNECT_FAILED:
          startWifiConnection("status change");
          break;
        default:
          break;
      }
    }

    status = statusNow;
  }

  if (status == WL_CONNECTED) {
    tcpServer.handleClient();
    if (millis() - lastRssiLog > RSSI_LOG_INTERVAL_MS) {
      lastRssiLog = millis();
      Serial.print("Wifi RSSI: ");
      Serial.println(WiFi.RSSI());
    }
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