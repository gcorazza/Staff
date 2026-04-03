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
#include "src/EffectStorage.h"
#include "src/effects/IdleAnimation.h"

StickGesture stickGesture;
IdleAnimation idle(leds, NUM_LEDS, 5);
int i=0;


unsigned long lastPrint = 0;
const unsigned long interval = 10000; // 10 Sekunden

void setup() {
  Serial.begin(115200);
  randomSeed(0);
  delay(500);
  setupLEDs();
  setupGy();
  setupWifi();
  EffectStorage fsInspector;
  fsInspector.begin();
  fsInspector.listEffects();
  printESP();
  setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));
  FastLED.show();
  drawPNGtoLEDs("/effects/disch.png");
}

void loop() {
  loopWifi();
  loopGy();

  const auto gesture = stickGesture.loopGesture();
  idle.updateMovementState(stickGesture.getMovementState());
  idle.idleAnimation();
  FastLED.show();
  setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));

  if (gesture == StickGesture::Gesture::HitGround) {
      drawPNGtoLEDs("/effects/disch.png");
  }

  unsigned long now = millis();

  if (now - lastPrint >= interval) {
    lastPrint = now;
    Serial.print("Zeit seit Start (ms): ");
    Serial.println(now);
  }
  delay(10);
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
    if(i == NUM_LEDS){
      i =0;
    }
    i++;
    setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));
    setBulk(leds, i, i+1, CRGB(255,255,255));
    FastLED.show();
}
