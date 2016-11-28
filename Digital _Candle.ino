// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bme; // I2C

float newSamp;
float sampBuf[8];
float bufSum;
float bufAvg;
int sampCnt = 0;
int state = 0;
int blowDet = 0;
int softDet = 0;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            6

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  Serial.begin(9600);

  if (!bme.begin()) {  
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1);
  }
}

void loop() {

  newSamp = bme.readPressure();

  if (sampCnt > 15) {
    bufSum = 0;
    for(int i=0;i<8;i++) {
      bufSum = bufSum + sampBuf[i];
    }
    bufAvg = (bufSum / 8.0);
    if (newSamp > (bufAvg+9)) {
      if (softDet == 1) {
        blowDet = 1;
        softDet = 0;
        sampCnt = 0;
      } else {
        softDet = 1;
      }
    } else {
      softDet = 0;
      for(int i=1; i<8; i++) {
        sampBuf[i] = sampBuf[i-1];
      }
      sampBuf[0] = newSamp;
    }
  } else {
    blowDet = 0;
    sampCnt = sampCnt + 1;
    for(int i=1; i<8; i++) {
      sampBuf[i] = sampBuf[i-1];
    }
    sampBuf[0] = newSamp;
  }
  
  if (blowDet == 1) {
    state = !state;
    // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
    for(int i=0;i<NUMPIXELS;i++){

      if (state == 0) {
        // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
        pixels.setPixelColor(i, pixels.Color(200,0,0)); // Moderately bright green color.
      } else {
        pixels.setPixelColor(i, pixels.Color(0,0,200)); // Moderately bright green color.
      }
  
      pixels.show(); // This sends the updated pixel color to the hardware.
  
    }
  }

  delay(100);
}
