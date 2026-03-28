//
// Created by gian on 01.03.26.
//

#include "LEDs.h"
#include "lib.h"

File gFile;
CRGB leds[NUM_LEDS];
PNG png;

void setupLEDs() {
    FastLED.addLeds<NEOPIXEL, LED_DATA_PIN>(leds, NUM_LEDS);
}

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

void turnOffAllLEDs() {
    setBulk(leds, 0, NUM_LEDS, CRGB(0,0,0));
    FastLED.show();
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
    turnOffAllLEDs();
    return true;
}


// ---------------- Pixel in NeoPixel schreiben ----------------
bool drawPNGtoLEDs(uint8_t* buffer, size_t length)
{
    int res = png.openRAM(buffer, length, myDraw);
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
    turnOffAllLEDs();
    return true;
}


