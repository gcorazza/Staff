#include "TcpServer.h"

TcpServer::TcpServer(uint16_t port)
  : server(port)   // Initialize WiFiServer with port
{
}

void TcpServer::begin() {
  server.begin();
}

void TcpServer::handleClient() {

  if (!client || !client.connected()) {
    client = server.available();
    return;
  }

  uint8_t buffer[256];

  int bytesRead = client.read(buffer, sizeof(buffer));

  if (bytesRead > 0) {
    Serial.write(buffer, bytesRead);

    // echo back
    client.write(buffer, bytesRead);
  }

  if (!client.connected()) {
    client.stop();
  }
}