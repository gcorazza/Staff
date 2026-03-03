#include <Arduino.h>
#include "FastLED.h"
#include <PNGdec.h>
#include <LittleFS.h>
#include "src/GY521.h"
#include "src/globals.h"
#include "src/lib.h"
#include "src/wifi.h"
#include "src/LEDs.h"
#include "src/StickGesture.h"

StickGesture stickGesture;

int i=0;


unsigned long lastPrint = 0;
const unsigned long interval = 10000; // 10 Sekunden

void setup() {
  Serial.begin(115200);
  randomSeed(0);
  delay(3000);
  setupLEDs();
  setupGy();
  setupWifi();
  listLittleFS();
  printESP();
  setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));
  drawPNGtoLEDs("/effects/disch.png");
}

void loop() {
  ledAliveAnimation();
  loopWifi();
  loopGy();
  StickGesture::Gesture ges = stickGesture.loopGesture();

  if(ges == StickGesture::Gesture::HitGround){
	  drawPNGtoLEDs("/effects/disch.png");
  }

  unsigned long now = millis();

  if (now - lastPrint >= interval) {
    lastPrint = now;
    Serial.print("Zeit seit Start (ms): ");
    Serial.println(now);
  }
}


void printFileHex(const char* filename) {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed!");
        return;
    }

    File file = LittleFS.open(filename, "r");
    if (!file) {
        Serial.println("File open failed!");
        return;
    }

    Serial.print("Dumping file: ");
    Serial.println(filename);

    size_t count = 0;

    while (file.available()) {
        uint8_t byte = file.read();

        // 2-stellige HEX-Ausgabe
        if (byte < 0x10) Serial.print("0");
        Serial.print(byte, HEX);
        Serial.print(" ");

        count++;

        // Zeilenumbruch alle 16 Bytes
        if (count % 16 == 0) {
            Serial.println();
        }
    }

    Serial.println();
    Serial.print("Total bytes: ");
    Serial.println(count);

    file.close();
}


void printESP(){
  Serial.println(ESP.getChipModel());
  Serial.println(ESP.getChipRevision());
  Serial.println(ESP.getSdkVersion());
}

void ledAliveAnimation(){
    if(i == 119){
      i =0;
    }
    i++;
    setBulk(leds, 0, 120, CRGB(0,0,0));
    setBulk(leds, i, i+1, CRGB(255,255,255));
    FastLED.show();
}


void listLittleFS() {
  if (!LittleFS.begin(true)) return;

  Serial.println("Listing LittleFS files:");

  File root = LittleFS.open("/effects");
  File file = root.openNextFile();

  while (file) {
    Serial.print("File: ");
    Serial.print(file.name());
    Serial.print("  Size: ");
    Serial.println(file.size());
    file = root.openNextFile();
  }
}
