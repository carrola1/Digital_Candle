// NeoPixel Ring simple sketch (c) 2013 Shae Erisson
// released under the GPLv3 license to match the rest of the AdaFruit NeoPixel library

#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

//#define BLYNK_USE_DIRECT_CONNECT
#include <BlynkSimpleRedBearLab_BLE_Nano.h>
#include <BLE_API.h>
#include <SimpleTimer.h>

char auth[] = "f7924766b7d64e4f8ec4a403b466b51a";
uint8_t device_name[] = "Blynk";

Adafruit_BMP280 bme; // I2C
SimpleTimer timer;

float newSamp;
float sampBuf[8];
float bufSum;
float bufAvg;
int sampCnt = 0;
int state = 1;
int blowDet = 0;
int softDet = 0;
int red = 0;
int green = 0;
int blue = 0;
int redLast = 0;
int greenLast = 0;
int blueLast = 0;
int redBlynk = 200;
int greenBlynk = 0;
int blueBlynk = 0;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            D0

// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      1

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_RGB + NEO_KHZ800);

void setup() {
  pixels.begin(); // This initializes the NeoPixel library.
  bme.begin();

  ble.init();

  ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::BREDR_NOT_SUPPORTED);
  ble.gap().accumulateAdvertisingPayload(GapAdvertisingData::SHORTENED_LOCAL_NAME,
                                         device_name, sizeof(device_name) - 1);

  ble.gap().setDeviceName(device_name);
  ble.gap().setTxPower(4);

  // Add Blynk service...
  Blynk.begin(auth);

  ble.gap().setAdvertisingType(GapAdvertisingParams::ADV_CONNECTABLE_UNDIRECTED);
  ble.gap().setAdvertisingInterval(Gap::MSEC_TO_GAP_DURATION_UNITS(1000));
  ble.gap().setAdvertisingTimeout(0);
  ble.startAdvertising();

  timer.setInterval(100L, getSample);
}

void getSample() {
  if (state == 1) {
    red = redBlynk;
    green = greenBlynk;
    blue = blueBlynk;
  }
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

    if (state == 0) {
      red = 0;
      green = 0;
      blue = 0;
    } else {
      red = redBlynk;
      green = greenBlynk;
      blue = blueBlynk;
    }
  }
  if ((red != redLast) || (green != greenLast) || (blue != blueLast)) {
    pixels.setPixelColor(0, pixels.Color(red,green,blue));
    pixels.show(); // This sends the updated pixel color to the hardware.
    redLast = red;
    greenLast = green;
    blueLast = blue;
  }
  
}

BLYNK_WRITE(V0)
{
  redBlynk = param.asInt();
}

BLYNK_WRITE(V1)
{
  greenBlynk = param.asInt();
}

BLYNK_WRITE(V2)
{
  blueBlynk = param.asInt();
}

void loop() {
  Blynk.run();
  timer.run(); // Initiates SimpleTimer
}
