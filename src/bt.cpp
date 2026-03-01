#include "BluetoothSerial.h"
#include "LittleFS.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "nvs_flash.h"


static BluetoothSerial SerialBT;

static File file;
static size_t bytesRemaining = 0;
static bool receivingFile = false;

void setupBt() {
  LittleFS.begin(true);

  if(!SerialBT.begin("Staff")){
    Serial.println("An error occurred initializing Bluetooth");
  } else {
    Serial.println("Bluetooth Ready");
    esp_bt_gap_set_scan_mode(
    ESP_BT_CONNECTABLE,
    ESP_BT_GENERAL_DISCOVERABLE
);
  }
}

  void printBufferHex(const uint8_t* buffer, size_t len) {
    for (size_t i = 0; i < len; i++) {

        // führende 0 für einstellige Werte
        if (buffer[i] < 0x10) Serial.print("0");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");

        // Zeilenumbruch alle 16 Bytes
        if ((i + 1) % 16 == 0) {
            Serial.println();
        }
    }

    Serial.println();
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
      printBufferHex(buffer, len);
      bytesRemaining -= len;
      Serial.println(bytesRemaining);

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
