#include <Arduino.h>
#include "FastLED.h"
#include "globals.h"
#include "lib.h"
#include "GY521.h"
#include "bt.h"
#include <PNGdec.h>

#define DATA_PIN 23

int i=0;

CRGB leds[NUM_LEDS];

PNG png;

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(115200);
  randomSeed(0);
  delay(3000);
  setupGy();
  setupBt();
  listLittleFS();
  printESP();

  //drawPNGtoLEDs("/disch.png", 0);
  //printFileHex("/disch.png");

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

void loop() {
  loopGy();
  loopBt();
  ledAliveAnimation();
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

  File root = LittleFS.open("/");
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

void* myOpen(const char* filename, int32_t* filesize) {
    Serial.print("myOpen filename: ");
    Serial.println(filename);
    gFile = LittleFS.open(filename, "r");
    if (!gFile){
      Serial.print("Error open file");
      return nullptr;
    }
    *filesize = gFile.size();
    Serial.print("File size: ");
    Serial.println(*filesize);
    return &gFile;
}

void myClose(void* handle) {
    // handle = &gFile
    File* f = (File*)handle;
    f->close();
}

int32_t myRead(PNGFILE* handle, uint8_t* buf, int32_t len)
{
    File* f = (File*)handle;
    return f->read(buf, len);
}


int32_t mySeek(PNGFILE* handle, int32_t pos)
{
    File* f = (File*)handle;
    if (!f->seek(pos)) return -1;
    return f->position();   // WICHTIG!
}

int myDraw(PNGDRAW* draw) {
    // Leerlauf: Wir speichern die Pixel in Zeilen manuell
    return 1;
}

// ---------------- Hilfsfunktion RGB565 → CRGB ----------------
CRGB RGB565toCRGB(uint16_t color) {
    uint8_t r = ((color >> 11) & 0x1F) << 3;
    uint8_t g = ((color >> 5) & 0x3F) << 2;
    uint8_t b = (color & 0x1F) << 3;
    return CRGB(r, g, b);
}

// ---------------- Pixel in NeoPixel schreiben ----------------
bool drawPNGtoLEDs(const char* filename, int row) {
    if (!LittleFS.begin()) {
        Serial.println("LittleFS mount failed!");
        return false;
    }

    // Erstelle neuen Pfad mit führendem "/"
    //char filepath[64]; // groß genug für den Pfad
    //snprintf(filepath, sizeof(filepath), "/%s", filename);

    // Prüfe, ob Datei existiert und Größe stimmt
    File f = LittleFS.open(filename, "r");
    if (!f) {
        Serial.println("File open failed!");
        return false;
    }
    Serial.print("File size: ");
    Serial.println(f.size());
    f.close();


    int res = png.open(filename, myOpen, myClose, myRead, mySeek, myDraw);
    if (res != PNG_SUCCESS) {
        Serial.print("PNG open failed! Error code: ");
        Serial.println(res);
        return false;
    }

    png.decode(NULL, 0); // nur Header lesen
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
    if (row >= height) {
        Serial.println("Requested row > PNG height");
        png.close();
        return false;
    }

    // Zeilenweise decodieren
    uint16_t* rowBuffer = new uint16_t[width];
    for (int y = 0; y <= row; y++) {
        png.decode(rowBuffer, width * sizeof(uint16_t));
    }

    // Pixel in NeoPixel Buffer schreiben
    for (int x = 0; x < width; x++) {
        leds[x] = RGB565toCRGB(rowBuffer[x]);
    }

    delete[] rowBuffer;
    png.close();

    FastLED.show();  // Daten an LEDs senden
    return true;
}

