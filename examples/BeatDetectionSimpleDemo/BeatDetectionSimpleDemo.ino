// based off of PassThroughStereo example

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "Beat.h"

// GUItool: begin automatically generated code
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
// GUItool: end automatically generated code
BeatAnalyzer beatAnalyzer(&analyzer);
 

const int myInput = AUDIO_INPUT_LINEIN;
//const int myInput = AUDIO_INPUT_MIC;


void setup() {
  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(12);

  // Enable the audio shield, select input, and enable output
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
   
  Serial.begin(9600);
  // turn on onboard led to signal successful completion of setup //
  digitalWrite(13, HIGH);
}

elapsedMillis volmsec=0;

void loop() {
  beatAnalyzer.update();
}


