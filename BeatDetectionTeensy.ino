// based off of PassThroughStereo example

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <OctoWS2811.h>
#include "Beat.h"
#include "Colors.h"
#include "Lerp.h"
#include "Utils.h"

// anim/lerp settings //
#define DEF_TIME_IN 250
#define DEF_TIME_OUT 2000
#define PING_PONG true
#define LOOP false

// colors //
Color_t idleColor = Color_t(255,255,255);
Color_t peakColor = Color_t(255,32,0);
ColorLerper lerp;

// led setup //
const int ledsPerPin = 8;
// allocate display memory in the lower-level stack
// (for quicker access via DMA), the *6 being some 
// sort of magic number I can't find any info on
// (2 bytes per color channel?)
DMAMEM int displayMemory[ledsPerPin*6];
int drawingMemory[ledsPerPin*6];
const int ledConfig = WS2811_GRB | WS2811_800kHz;
OctoWS2811 leds(ledsPerPin, displayMemory, drawingMemory, ledConfig);

// audio setup //
AudioInputI2S            i2s1;           //xy=200,69
AudioOutputI2S           i2s2;           //xy=365,94
AudioMixer4              mixer;
AudioConnection          inputToMixer1(i2s1, 0, mixer, 0);
AudioConnection          inputToMixer2(i2s1, 1, mixer, 1);
AudioConnection          patchCord1(mixer, 0, i2s2, 0);
AudioConnection          patchCord2(mixer, 0, i2s2, 1);
AudioAnalyzePeak         analyzer;
// route mixer output to analyzer
AudioConnection mixerToAnalyzer(mixer, analyzer);
AudioControlSGTL5000     sgtl5000_1;     //xy=302,184
BeatAnalyzer beatAnalyzer(&analyzer, &onBeat);

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;

float lerpValue = 0.0;
float lastLerpValue = lerpValue;
const float lerpDecrease = 0.01;

void onBeat(BeatInfo beatInfo) {
  Serial.print("BEAT: ");
  Serial.println(beatInfo.lastPeak);
  
//  Color_t lastPeakColor = peakColor;
//  float p = beatInfo.lastPeak;
//  if(p > 0.0 && p < 0.20) peakColor = Color_t(255, 255, 0);
//  if(p > 0.20 && p < 0.40) peakColor = Color_t(64, 32, 255);
//  else peakColor = Color_t(255, 32, 0);
  
//  if(peakColor != lastPeakColor) {
//    lerp.setEndColor(peakColor);
//  }  
  
//  lerpValue = min(beatInfo.lastPeak, 1.0);
//  setGlobalColor(peakColor.r, peakColor.g, peakColor.b, &leds);
  lerp.start();
}

void onBpmChanged(int bpm, int fadeTime) {
  lerp.setup(idleColor, peakColor, fadeTime/6, fadeTime, false, false);
  lerp.pause();
}

void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
  
  // init leds //
  leds.begin();
  setGlobalColor(idleColor.r, idleColor.g, idleColor.b, &leds);
  lerp.setup(idleColor, peakColor, DEF_TIME_IN, DEF_TIME_OUT, PING_PONG, LOOP);
  lerp.pause();

  // init serial //   
  Serial.begin(9600);
  
  // turn on onboard led to signal successful completion of setup //
  digitalWrite(13, HIGH);
}

uint8_t lerpUint8(uint8_t a, uint8_t b, float pct) {
  return a + ((b - a) * pct);
}

Color_t getLerpedColor(Color_t start, Color_t end, float pct) {
  Color_t c;
  c.r = lerpUint8(start.r, end.r, pct);
  c.g = lerpUint8(start.g, end.g, pct);
  c.b = lerpUint8(start.b, end.b, pct);
  
  return c;
}

void loop() {
  beatAnalyzer.update();
  lerp.update();
  
  if(lerp.isLerping()) {
    Color_t c = lerp.getLerpedColor();
    setGlobalColor(c.r, c.g, c.b, &leds);
  }
  
//  if(lerpValue != lastLerpValue) {
//    Color_t c = getLerpedColor(idleColor, peakColor, lerpValue);
//    setGlobalColor(c.r, c.g, c.b, &leds);
//  }
//  
//  if(lerpValue > 0.0) lerpValue -= lerpDecrease;
}


