#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <WiFi.h>

class TcpServer {
public:
    TcpServer(uint16_t port);
    void begin();
    void handleClient();

private:
    WiFiServer server;
    WiFiClient client;
};

#endif
