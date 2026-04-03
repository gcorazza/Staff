#include "TcpServer.h"
#include "LEDs.h"
#include "Animator.h"
#include "PNGAnimation.h"
#include <WiFi.h>
#include <Arduino.h>
#include "lwip/sockets.h"
#include "lwip/inet.h"

TcpServer::TcpServer(uint16_t port)
    : server(port),
      fileBuffer(nullptr),
      fileSize(0),
      bytesReceived(0),
      receivingFile(false),
      currentFilename(""),
      currentStoredFilename(""),
      currentSaveMode(SaveMode::None),
      discardIncomingFile(false)
{
}

void TcpServer::begin() {
    server.begin();
    Serial.println("TCP Server started");
    storage.begin();
}

static unsigned long lastHeartbeatTime = 0;

// Call this in loop()
void TcpServer::handleClient() {
    // Accept new client if none connected
    if (!client || !client.connected()) {
        if (client) {
            client.stop();
            Serial.println("Client disconnected");
        }
        receivingFile = false;
        currentFilename = "";
        currentStoredFilename = "";
        currentSaveMode = SaveMode::None;
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
        size_t remaining = fileSize - bytesReceived;
        int availableBytes = client.available();
        if (availableBytes > (int)remaining) {
            availableBytes = static_cast<int>(remaining);
        }

        if (discardIncomingFile) {
            uint8_t sink[256];
            int toProcess = availableBytes;
            while (toProcess > 0) {
                int chunk = toProcess > static_cast<int>(sizeof(sink)) ? static_cast<int>(sizeof(sink)) : toProcess;
                int n = client.read(sink, chunk);
                if (n <= 0) {
                    break;
                }
                bytesReceived += n;
                toProcess -= n;
            }

            if (bytesReceived == fileSize) {
                receivingFile = false;
                discardIncomingFile = false;
                client.println("IGNORED: Used cached file");
                Serial.println("Skipped incoming PNG; cached version already rendered");
            }
        } else if (fileBuffer != nullptr && availableBytes > 0) {
            int n = client.read(fileBuffer + bytesReceived, availableBytes);
            if (n > 0) {
                bytesReceived += n;
                Serial.printf("Received chunk: %d bytes, total %d/%d\n", n, bytesReceived, fileSize);
            }

            if (bytesReceived == fileSize) {
                receivingFile = false;
                client.println("File received successfully");
                Serial.println("File fully received");

                animator.play(std::make_shared<PNGAnimation>(fileBuffer, fileSize, leds, NUM_LEDS), true);

                String savedPath;
                if (!storage.saveFile(fileBuffer, fileSize, currentFilename, currentStoredFilename, currentSaveMode, &savedPath)) {
                    if (currentSaveMode != SaveMode::None) {
                        client.println("WARNING: Failed to store file");
                    }
                } else if (currentSaveMode != SaveMode::None) {
                    client.println("SAVED: " + savedPath);
                }

                delete[] fileBuffer;  // free RAM
                fileBuffer = nullptr;
                currentFilename = "";
                currentStoredFilename = "";
                currentSaveMode = SaveMode::None;
            }
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
        Serial.println("Send Heartbeat");
        client.println("HEARTBEAT");
        lastHeartbeatTime = millis();
    }
}

// Parse commands
void TcpServer::processCommand(const String& cmd) {
    Serial.print("Command received: "); Serial.println(cmd);

    if (cmd.equalsIgnoreCase("ls")) {
        if (client && client.connected()) {
            storage.listEffects(client);
        }
        // Always print on Serial Bus
        storage.listEffects();
        return;
    }

    if (cmd == "STATUS") {
        client.println("OK");
    }
    else if (cmd.startsWith("PLAYPNG ")) {
        int firstSpace = cmd.indexOf(' ');
        int secondSpace = cmd.indexOf(' ', firstSpace + 1);
        if (firstSpace < 0 || secondSpace < 0) {
            client.println("ERROR: Bad PLAYPNG format");
            return;
        }

        int thirdSpace = cmd.indexOf(' ', secondSpace + 1);
        String filename;
        String sizeStr;
        String modeStr;

        if (thirdSpace < 0) {
            sizeStr = cmd.substring(secondSpace + 1);
            modeStr = "NOSAVE";
        } else {
            sizeStr = cmd.substring(secondSpace + 1, thirdSpace);
            modeStr = cmd.substring(thirdSpace + 1);
        }

        filename = cmd.substring(firstSpace + 1, secondSpace);
        sizeStr.trim();
        modeStr.trim();
        filename.trim();

        fileSize = sizeStr.toInt();
        if (fileSize <= 0) {
            client.println("ERROR: Invalid size");
            return;
        }

        String sanitizedName = storage.sanitizeFilename(filename);
        if (sanitizedName.isEmpty()) {
            client.println("ERROR: Invalid filename");
            return;
        }

        String modeLower = modeStr;
        modeLower.toLowerCase();
        if (modeLower == "nosave" || modeLower == "none") {
            currentSaveMode = SaveMode::None;
        } else if (modeLower == "temp" || modeLower == "temporary") {
            currentSaveMode = SaveMode::Temp;
        } else if (modeLower == "persistent" || modeLower == "effects" || modeLower == "permanent") {
            currentSaveMode = SaveMode::Persistent;
        } else {
            client.println("ERROR: Unknown save mode");
            return;
        }

        String cachedPath;
        bool usingCached = storage.findExistingFile(sanitizedName, fileSize, &cachedPath);

        if (!usingCached) {
            currentFilename = sanitizedName;
            currentStoredFilename = storage.prepareFilename(currentFilename, currentSaveMode);

            if (fileBuffer != nullptr) {
                delete[] fileBuffer;
                fileBuffer = nullptr;
            }

            fileBuffer = new uint8_t[fileSize];
            discardIncomingFile = false;
        } else {
            if (fileBuffer != nullptr) {
                delete[] fileBuffer;
                fileBuffer = nullptr;
            }
            currentFilename = "";
            currentStoredFilename = "";
            currentSaveMode = SaveMode::None;
            discardIncomingFile = true;
            // drawPNGtoLEDs(cachedPath.c_str()); // entfernt, da veraltet
            animator.play(cachedPath.c_str(), true);
            client.println("USING-CACHED: " + cachedPath);
            Serial.println("Using cached PNG: " + cachedPath);
        }

        bytesReceived = 0;
        receivingFile = true;

        client.println("READY: Receiving " + filename + " (" + String(fileSize) + " bytes) with mode " + modeStr);
        Serial.println("Receiving file: " + filename + " (" + String(fileSize) + " bytes) mode=" + modeStr);
    }
    else {
        client.println("ERROR: Unknown command");
    }
}

String TcpServer::getFilename(const String& baseName) {
    return storage.getFilename(baseName);
}

