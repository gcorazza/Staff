#include <Arduino.h>
#include "FastLED.h"
#include <PNGdec.h>
#include <LittleFS.h>
#include "src/GY521.h"
#include "src/globals.h"
#include "src/lib.h"
#include "src/wifi.h"

#define LED_DATA_PIN 23

int i=0;

CRGB leds[NUM_LEDS];

PNG png;

unsigned long lastPrint = 0;
const unsigned long interval = 10000; // 10 Sekunden

void setup() {

  FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(115200);
  randomSeed(0);
  delay(3000);
  setupGy();
  setupWifi();
  listLittleFS();
  printESP();
  setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));
  drawPNGtoLEDs("/effects/disch.png");
}

void loop() {
  ledAliveAnimation();
  loopGy();
  loopWifi();

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


// Globale Datei-Referenz
File gFile;


// Here are the callback functions that the decPNG library
// will use to open files, fetch data and close the file.

File pngfile;
void* myOpen(const char* filename, int32_t* filesize)
{
    gFile = LittleFS.open(filename, "rb");
    if (!gFile) return NULL;
    *filesize = gFile.size();
    return &gFile;
}

void myClose(void* handle){
    File* f = (File*)handle;
    f->close();
}

int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen){
    File *f = (File *)pFile->fHandle;
    return f->read(pBuf, iLen);
}

int32_t mySeek(PNGFILE *pFile, int32_t iPosition)
{
    File *f = (File *)pFile->fHandle;
    if (!f->seek(iPosition)) return -1;
    return f->position();
}

int myDraw(PNGDRAW* draw) {
    uint8_t *p = draw->pPixels;

    for (int x = 0; x < draw->iWidth; x++)
    {
        uint8_t r = p[x * 3 + 0];
        uint8_t g = p[x * 3 + 1];
        uint8_t b = p[x * 3 + 2];

        leds[x] = CRGB(r, g, b);
    }

    FastLED.show();
    return 1;
}

// ---------------- Pixel in NeoPixel schreiben ----------------
bool drawPNGtoLEDs(const char* filename)
{
    int res = png.open(filename, myOpen, myClose, myRead, mySeek, myDraw);
    if (res != PNG_SUCCESS) {
        Serial.println("PNG open failed!");
        return false;
    }

    uint16_t width = png.getWidth();
    uint16_t height = png.getHeight();
    if (width != NUM_LEDS) {
        Serial.print("PNG width = ");
        Serial.print(width);
        Serial.print(" != NUM_LEDS = ");
        Serial.println(NUM_LEDS);
        png.close();
        return false;
    }

    png.decode(NULL, 0);  // startet komplettes Rendering
    png.close();
    return true;
}

