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

#define HWSERIAL Serial1
#define PACKET_SIZE 4

typedef enum Opcode {
    SET_MODE = 0xFF,
    SET_MODE_PARAMETER = 0x01, // second byte will define parameter, following 1-2 bytes for value
    SET_COLOR = 0x0C,
    SET_BRIGHTNESS = 0x0B,
    SET_TIME = 0x02,
} Opcode;

// modes //
typedef enum Modes {
  SCAN = 0x01,
  PULSE = 0x02,
  STRESS_TEST = 0x03,
  ALL_ON = 0x04,
  SUNRISE = 0x05,
  BEAT = 0x06
} Modes;
Modes mode;

#define BTN_PIN 12
uint8_t btnState = 0;

// anim/lerp settings //
#define DEF_TIME_IN 250
#define DEF_TIME_OUT 2000
#define LERP_PING_PONG true
#define LERP_LOOP false
#define TEST_LENGTH 2
#define TEST_MOVE_SPEED 1

#define SUNRISE_DURATION 10000

// colors //
Color_t selectedColor = Color_t(255,255,255);
Color_t idleColor = Color_t(255,255,255);
Color_t peakColor = Color_t(255,32,0);
Color_t sunriseStartColor = Color_t(128,0,255);
Color_t sunriseEndColor = Color_t(255,200,0);
ColorLerper lerp;
ColorLerper sunriseLerp;

Color_t pulseStartColor = Color_t(0,0,0);
Color_t pulseEndColor = Color_t(0,64,255);
#define PULSE_DURATION 1000
ColorLerper pulseLerp;

// led setup //
const int ledsPerPin = 288;
const uint8_t pinsUsed = 5;
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

/*** MODE SET FUNCTIONS **/
void setupMode(Modes _m, Color_t _c = selectedColor) {
  switch(_m) {
    case ALL_ON:
      setGlobalColor(_c.r, _c.g, _c.b, &leds);
      mode = ALL_ON;
      break;
    case PULSE:
      mode = PULSE;
      setGlobalColor(0,0,0, &leds);
      pulseLerp.setup(Color_t(0,0,0), _c, PULSE_DURATION, PULSE_DURATION, true, true);
      pulseLerp.start();
      break;
    case SCAN:
      mode = SCAN;
      setGlobalColor(0,0,0, &leds);
      break;
    case STRESS_TEST:
      mode = STRESS_TEST;
      setGlobalColor(0,0,0, &leds);
      break;
    default:
      break;
  }
}     
/** END MODE SET FUNCTIONS **/

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
//  setGlobalColor(idleColor.r, idleColor.g, idleColor.b, &leds);
  setGlobalColor(0,0,0, &leds);
//  setGlobalColor(selectedColor.r, selectedColor.b, selectedColor.g, &leds);
  lerp.setup(idleColor, peakColor, DEF_TIME_IN, DEF_TIME_OUT, LERP_PING_PONG, LERP_LOOP);
  lerp.pause();
  
  sunriseLerp.setup(sunriseStartColor, sunriseEndColor, SUNRISE_DURATION, SUNRISE_DURATION, false, false);
  sunriseLerp.pause();
  
  pulseLerp.setup(pulseStartColor, pulseEndColor, PULSE_DURATION, PULSE_DURATION, true, true);
  pulseLerp.pause();

  // init serial //   
  Serial.begin(9600);
  
  // init hardware serial for BLE //
  HWSERIAL.begin(57600);
  
  // setup btn //
  pinMode(BTN_PIN, INPUT);

  mode = ALL_ON;
  setupMode(mode);
  
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

uint16_t testStripPosition = 0;
uint16_t lastTestStripPosition = 0;
long testLastMoveTime = 0;

void loop() {
  beatAnalyzer.update();
  lerp.update();
  
  // handle BLE serial input //
  if(HWSERIAL.available() >= PACKET_SIZE) {
    byte serialData[PACKET_SIZE];
    for(size_t i = 0; i < PACKET_SIZE; i++) {
      serialData[i] = HWSERIAL.read();
    }
    
    switch(serialData[0]) {
      case SET_MODE:
        switch(serialData[1]) {
          case ALL_ON:
            setupMode(ALL_ON, selectedColor);
            break;
          case SCAN:
            setupMode(SCAN);
            break;
          case PULSE:
            setupMode(PULSE);
            break;
          case STRESS_TEST:
            setupMode(STRESS_TEST);
            break;
          default:
            break;
        }
        break;
      case SET_COLOR:
        selectedColor.r = serialData[1];
        selectedColor.g = serialData[2];
        selectedColor.b = serialData[3];
        setupMode(mode);
        break;
      default:
        break;    
    }
  }
  
  // handle current mode //
  Color_t c;
  #define NUM_RAND_INDICES 144
  uint16_t randomIndices[NUM_RAND_INDICES];
  
  switch(mode) {
    case SCAN:
      if(millis() - testLastMoveTime > TEST_MOVE_SPEED) {
        // animate a test strip across the length of the strip, and back again
//        setGlobalColor(0,0,0,&leds);
        for(size_t i = lastTestStripPosition; i < TEST_LENGTH + lastTestStripPosition; i++) {
          leds.setPixel(i, 0, 0, 0);
        }
        
        for(size_t i = testStripPosition; i < TEST_LENGTH + testStripPosition; i++) {
          leds.setPixel(i, selectedColor.r, selectedColor.g, selectedColor.b);
        }
        leds.show();
        testLastMoveTime = millis();
        lastTestStripPosition = testStripPosition;
        testStripPosition = (testStripPosition + 1) < (ledsPerPin*pinsUsed) ? testStripPosition + 1 : 0;
      }
      break;
     case PULSE:
       pulseLerp.update();
       if(pulseLerp.isLerping()) {
         c = pulseLerp.getLerpedColor();
         setGlobalColor(c.r, c.g, c.b, &leds);
       }
       break;
     case STRESS_TEST:
       // light up random group of LEDS
       for(size_t i = 0; i < NUM_RAND_INDICES; i++) {
         // last random set off, first
         leds.setPixel(randomIndices[i], 0, 0, 0);
         // now generate a new set of random indices...
         randomIndices[i] = random(0, ledsPerPin * pinsUsed);
         // light new random index
         
       }
       
       for(size_t i = 0; i < NUM_RAND_INDICES; i++) {
         leds.setPixel(randomIndices[i], selectedColor.r, selectedColor.g, selectedColor.b);
       }
       
       leds.show();
       
       break;
     case ALL_ON:
       // handle when button is pressed (do not need to constantly refresh)
       break;
     case SUNRISE:
       sunriseLerp.update();
       if(sunriseLerp.isLerping()) {
         c = sunriseLerp.getLerpedColor();
         setGlobalColor(c.r, c.g, c.b, &leds);
       }
       break;
     case BEAT:
       if(lerp.isLerping()) {
          c = lerp.getLerpedColor();
          setGlobalColor(c.r, c.g, c.b, &leds);
        }
        break;
     default:
        break;
  }
  
  /** handle momentary button press **/
  uint8_t lastBtnState = btnState;
  btnState = digitalRead(BTN_PIN);
  
  if(btnState == LOW && btnState != lastBtnState) {
    switch(mode) {
      case SCAN:
        // move to pulse mode //
        setupMode(PULSE, selectedColor);
        break;
      case PULSE:
        // move to stress test mode //
        setupMode(STRESS_TEST, selectedColor);
        break;
       case STRESS_TEST:
         // move to all on mode //
         setupMode(ALL_ON, selectedColor);
         break;
       case ALL_ON:
         // move to scan mode //
         setupMode(SCAN, selectedColor);
         break;
       default:
         break;
    }
    
    Serial.print("current mode: ");
    Serial.println(mode);
  }
  
  
//  if(lerpValue != lastLerpValue) {
//    Color_t c = getLerpedColor(idleColor, peakColor, lerpValue);
//    setGlobalColor(c.r, c.g, c.b, &leds);
//  }
//  
//  if(lerpValue > 0.0) lerpValue -= lerpDecrease;
}


