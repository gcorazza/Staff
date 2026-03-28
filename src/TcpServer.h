#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <WiFi.h>
#include "EffectStorage.h"

class TcpServer {
public:
    TcpServer(uint16_t port);
    void begin();
    void handleClient();
    String getFilename(const String& baseName);

private:
    using SaveMode = EffectStorage::SaveMode;

    WiFiServer server;
    WiFiClient client;
    String inputBuffer;

    // File reception
    uint8_t* fileBuffer;
    size_t fileSize;
    size_t bytesReceived;
    bool receivingFile;
    String currentFilename;
    String currentStoredFilename;
    SaveMode currentSaveMode;
    EffectStorage storage;
    bool discardIncomingFile;

    void processCommand(const String& cmd);
};

#endif

