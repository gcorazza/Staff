#include "BluetoothSerial.h"
#include "LittleFS.h"

static BluetoothSerial SerialBT;

static File file;
static size_t bytesRemaining = 0;
static bool receivingFile = false;

void setupBt() {
  LittleFS.begin(true);
  SerialBT.begin("Staff");
  Serial.println("Bluetooth Ready");
}

void loopBt() {

  // If we are currently receiving file data
  if (receivingFile) {
    uint8_t buffer[512];

    int availableBytes = SerialBT.available();
    if (availableBytes > 0) {

      int toRead = min((int)bytesRemaining, availableBytes);
      int len = SerialBT.readBytes(buffer, toRead);

      file.write(buffer, len);
      bytesRemaining -= len;

      if (bytesRemaining == 0) {
        file.close();
        receivingFile = false;
        Serial.println("File received successfully.");
      }
    }
  }

  // If not receiving file, check for START line
  else if (SerialBT.available()) {

    String header = SerialBT.readStringUntil('\n');
    header.trim();

    if (header.startsWith("START ")) {

      int firstSpace = header.indexOf(' ');
      int secondSpace = header.indexOf(' ', firstSpace + 1);

      String filename = "/" + header.substring(firstSpace + 1, secondSpace);
      bytesRemaining = header.substring(secondSpace + 1).toInt();

      file = LittleFS.open(filename, FILE_WRITE);

      if (!file) {
        Serial.println("Failed to open file");
        return;
      }

      receivingFile = true;
      Serial.printf("Receiving %s (%d bytes)\n", filename.c_str(), bytesRemaining);
    }
  }
}
