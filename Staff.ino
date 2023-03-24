#include "FastLED.h"
#include "globals.h"
#include "lib.h"

#define DATA_PIN 6

int DEL = 500;

CRGB leds[NUM_LEDS];

void setup() {

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(9600);
  randomSeed(28);
  delay(100);
}

void loop() {
  int arr[NUM_LEDS];
  for(int i = 0; i < NUM_LEDS; i++)
        arr[i] = (rand() % 10);

  bubbleSort(arr, NUM_LEDS);
}

void swap(int* xp, int* yp)
{
    int temp = *xp;
    *xp = *yp;
    *yp = temp;
}
 
// A function to implement bubble sort
void bubbleSort(int arr[], int n)
{
    int i, j;
    for (i = 0; i < n - 1; i++)
 
        // Last i elements are already in place
        for (j = 0; j < n - i - 1; j++)
            if (arr[j] > arr[j + 1])
                {
                  swap(&arr[j], &arr[j + 1]);
                  displayArray(arr);
                }
}

void displayArray(int arr[])
{
  for(int i = 0; i < NUM_LEDS; i++){
    int v = map(arr[i], 0,10,0,255);
    leds[i] = CRGB(v, v, v);
  }
  delay(DEL);
}

void policeLights() {
  for (byte i = 1; i <= 3; i++) {
    setPolice(CRGB::Blue, CRGB::Red);
    delay(DEL);
    clearLEDsInvisible(leds);
    delay(DEL);
  }
  delay(DEL);
  for (byte i = 1; i <= 3; i++) {
    setPolice(CRGB::Red, CRGB::Blue);
    delay(DEL);
    clearLEDsInvisible(leds);
    delay(DEL);
  }
  delay(DEL);
}

void setPolice(CRGB c1, CRGB c2) {
  for (byte i = 0; i < NUM_LEDS / 2; i++) {
    leds[i] = c1;
  }
  for (byte i = NUM_LEDS / 2; i < NUM_LEDS; i++) {
    leds[i] = c2;
  }
  FastLED.show();
}
