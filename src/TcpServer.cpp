#include "TcpServer.h"
#include "LEDs.h"
#include <WiFi.h>
#include <Arduino.h>
#include "lwip/sockets.h"
#include "lwip/inet.h"

TcpServer::TcpServer(uint16_t port)
    : server(port),
      fileBuffer(nullptr),
      fileSize(0),
      bytesReceived(0),
      receivingFile(false)
{
}

void TcpServer::begin() {
    server.begin();
    Serial.println("TCP Server started");
}

int lastHeartbeatTime = 0;

// Call this in loop()
void TcpServer::handleClient() {
    // Accept new client if none connected
    if (!client || !client.connected()) {
        if (client) {
            client.stop();
            Serial.println("Client disconnected");
        }
        receivingFile = false;
        if (fileBuffer) {
            delete[] fileBuffer;
            fileBuffer = nullptr;
        }
        client = server.available();
        if (client) {
            Serial.println("Client connected");
            int optval = 1;
            setsockopt(client.fd(), SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
        }
        return;
    }

    // Receiving file
    if (receivingFile && client.available()) {
        int toRead = client.available();
        if (toRead > fileSize - bytesReceived) toRead = fileSize - bytesReceived;

        int n = client.read(fileBuffer + bytesReceived, toRead); // write directly into buffer
        if (n > 0) {
            bytesReceived += n;
            Serial.printf("Received chunk: %d bytes, total %d/%d\n", n, bytesReceived, fileSize);
        }

        if (bytesReceived == fileSize) {
            receivingFile = false;
            client.println("File received successfully");
            Serial.println("File fully received");

            drawPNGtoLEDs(fileBuffer, fileSize);

            delete[] fileBuffer;  // free RAM
            fileBuffer = nullptr;
        }
    }

    // Otherwise read commands line by line
    while (!receivingFile && client.available()) {
        char c = client.read();
        inputBuffer += c;

        if (c == '\n') {  // end of command
            inputBuffer.trim();
            processCommand(inputBuffer);
            inputBuffer = "";
        }
    }

    //send heartbeat
    if (millis() - lastHeartbeatTime > 5000) {
        client.println("HEARTBEAT");
        lastHeartbeatTime = millis();
    }
}

// Parse commands
void TcpServer::processCommand(const String& cmd) {
    Serial.print("Command received: "); Serial.println(cmd);

    if (cmd == "STATUS") {
        client.println("OK");
    }
    else if (cmd.startsWith("PLAYPNG ")) {
        // Expected format: SENDFILE filename size
        int firstSpace = cmd.indexOf(' ');
        int secondSpace = cmd.indexOf(' ', firstSpace + 1);
        if (secondSpace < 0) {
            client.println("ERROR: Bad SENDFILE format");
            return;
        }

        String filename = cmd.substring(firstSpace + 1, secondSpace);
        String sizeStr = cmd.substring(secondSpace + 1);
        fileSize = sizeStr.toInt();

        if (fileSize <= 0) {
            client.println("ERROR: Invalid size");
            return;
        }

        if (fileBuffer != nullptr) {
            delete[] fileBuffer;
            fileBuffer = nullptr;
        }

        // Allocate RAM for entire file
        fileBuffer = new uint8_t[fileSize];
        bytesReceived = 0;
        receivingFile = true;

        client.println("READY: Receiving " + filename + " (" + String(fileSize) + " bytes)");
        Serial.println("Receiving file: " + filename + " (" + String(fileSize) + " bytes)");
    }
    else {
        client.println("ERROR: Unknown command");
    }
}