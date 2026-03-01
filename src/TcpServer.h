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
    String inputBuffer;

    // File reception
    uint8_t* fileBuffer;
    size_t fileSize;
    size_t bytesReceived;
    bool receivingFile;

    void processCommand(const String& cmd);
};

#endif