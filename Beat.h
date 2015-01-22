#ifndef BEAT_H
#define BEAT_H

#include <Arduino.h>
#include <Audio.h>

#define AVERAGE_FRAMES 200      // ticks to average audio energy for comparison against instance energy //
#define BEAT_WAIT_INTERVAL 400  // time to wait for detection after a beat
#define BEAT_HOLD_TIME 80       // number of frames to hold a beat
#define BEAT_DECAY_RATE  0.99   // how much to decay the beat threshold on a non-beat
#define BEAT_MIN 0.25           // anything with a peak level lower than this will not count as a beat

//Track a threshold volume level.
//If the current volume exceeds the threshold then you have a beat. Set the new threshold to the current volume.
//Reduce the threshold over time, using the Decay Rate.
//Wait for the Hold Time before detecting for the next beat. This can help reduce false positives.

class BeatAnalyzer {
  public:
    typedef struct BeatInfo {
      uint16_t numPeaks;
      float lastPeak;
    } BeatInfo;

    typedef void(*beat_callback)(BeatInfo);

    BeatAnalyzer(AudioAnalyzePeak* _peakAnalyzer, beat_callback _beatCallback);
    ~BeatAnalyzer();

    void update();
    
    uint16_t getNumBeats(boolean _clear = false);
    boolean gotBeat();

  private:
    BeatInfo beatInfo;

    // peak analysis //
    AudioAnalyzePeak* peakAnalyzer;
    float avgPeak;
    float peakRunningTotal = 0.0;
    long lastPeakTime = 0;
    
    // beat detection vars //
    uint16_t beatWaitIntervalMs = 350;
    
    uint16_t averagingIntervalMs = 200;
    long lastSampleTime = 0;

    float beatMinPeak = 0.2;
    float beatCutoff = 0.5;
    uint16_t beatDecayDelay = 0;   // beatHoldTime should never be more than a few seconds, so 16bit uint is ok
    beat_callback onBeatCallback;
};

#endif

// #include <Arduino.h>
// #include <Audio.h>

// // TO-DO: parameterize and reduce //
// #define AVERAGE_FRAMES 200      // ticks to average audio energy for comparison against instance energy //
// #define BEAT_WAIT_INTERVAL 350  // time to wait for detection after a beat
// #define BEAT_HOLD_TIME 80       // number of frames to hold a beat
// #define BEAT_DECAY_RATE  0.98   // how much to decay the beat threshold on a non-beat
// #define BEAT_MIN 0.2           // anything with a peak level lower than this will not count as a beat

// typedef struct BeatInfo {
//   uint16_t numBeats;
//   float lastPeak;
// } BeatInfo;

// typedef void(*beat_callback)(BeatInfo);
// typedef void(*bpm_changed_callback)(int, int);

// class BeatAnalyzer {
//   public:
//     BeatInfo beatInfo;
    
//     BeatAnalyzer(AudioAnalyzePeak* _peakAnalyzer, beat_callback _beatCallback) {
//       peakAnalyzer = _peakAnalyzer;
//       onBeatCallback = _beatCallback;
//       fftEnabled = false;
//       init();
//     }
    
//     ~BeatAnalyzer() {
//       delete peakAnalyzer;
//       delete fftAnalyzer;
//     }
    
//     void update() {
//         if(fftEnabled && fftAnalyzer->available()) {
//           for(size_t i = 0; i < 127; i++) {
//             float n = fftAnalyzer->read(i);
//             binAverages[i].add(n);
//           }
//         }
      
//         if(peakAnalyzer->available()) {
//           float peak = peakAnalyzer->read();
          
//           // calculate an average of the last few frames of
//           // audio data -- anything greater than this AND the running beat threshold
//           // will count as a beat
//           if(numFramesCounted >= AVERAGE_FRAMES) {
//             avgPeak = peakRunningTotal / AVERAGE_FRAMES;
//             peakRunningTotal = 0;
//             numFramesCounted = 0;
//           }
      
//           peakRunningTotal += peak;
//           numFramesCounted++;
        
//         boolean doDetectBeat = (millis() - peakTimer > BEAT_WAIT_INTERVAL);
//         if(doDetectBeat && peak > beatCutoff && peak > BEAT_MIN && peak > avgPeak) {
//           numBeats = (numBeats < 65536) ? numBeats + 1 : 1; // reset numBeats to 1 if it is greater than 16 bits
          
//           beatInfo.numBeats = numBeats;
//           beatInfo.lastPeak = peak;
            
//           // slightly increase beat threshold //
//           beatCutoff = peak * 1.1;
          
//           // call callback //
//           (*onBeatCallback)(beatInfo);
            
//           // reset timer //
//           peakTimer = millis();
//           beatTime = 0;
//         } else {
//           if(beatTime <= BEAT_HOLD_TIME) {
//             beatTime++;
//           } else {
//             // slowly decrease beat cutoff with time //
//             beatCutoff *= BEAT_DECAY_RATE;
//             beatCutoff = max(beatCutoff, BEAT_MIN);
//           }
//         }
//       }
//     }
    
//     uint16_t getNumBeats(boolean _clear = false) {
//       uint16_t n = numBeats;
//       if(_clear) numBeats = 0;
//       return n;
//     }
    
//     void calculateBPM(bpm_changed_callback bpmCallback) {
//       if(((millis() - lastBpmTime) / 1000) > BPMCalculateDelay) {
//         uint16_t lastBpm = bpm;
//         bpm = numBeats / (millis() / 1000);
//         if(bpm != lastBpm) {
//           fadeTime = ((bpm / 60) * 1000) / 6;
//           Serial.print("BPM: "); Serial.println(bpm);
//           Serial.print("Fade time: "); Serial.println(fadeTime);
//           bpmCallback(bpm, fadeTime);
// //          lerp.setup(idleColor, peakColor, fadeTime, true);
//         }
//           lastBpmTime = millis();
//       }
//     }
    
//     boolean gotBeat() {
//       return (numBeats > 0);
//     }
    
//    private:
   
//      void init() {
//        numBeats = 0;
//      }
    
//     // peak analysis //
//     AudioAnalyzePeak* peakAnalyzer;
//     float avgPeak;
//     float peakRunningTotal = 0.0;
//     uint16_t numFramesCounted = 0;
//     long peakTimer = 0;
    
//     // fft analysis //
//     boolean fftEnabled;
//     AudioAnalyzeFFT256* fftAnalyzer;
//     static const uint8_t binsToCountForBeat = 4; // pick the top n-number of bins with the highest freq energy to count as beats
//     uint8_t beatBins[binsToCountForBeat]; // store indices of FFT bins with highest freq energy;
    
//     typedef struct BinInfo {
//       float average;
//       uint16_t numSamples;
      
//       void add(float sample) {
//         this->average += average;
//         this->numSamples++;
//         this->average /= this->numSamples;
//       }
//     } BinInfo;
//     BinInfo binAverages[127];
    
//     uint16_t fftAverageTimer = 0;
//     static const uint8_t fftAveragePeriod = 2; // in seconds
    
//     // other beat bizzness //
//     float beatCutoff = 0.5;
//     long beatTime = 0;
//     beat_callback onBeatCallback;
      
//     // bpm calculation //
//     const uint8_t BPMCalculateDelay = 12; // number of seconds to wait before counting bpm
//     uint16_t fadeTime;                    // some fraction of the time between beats -- allow enough time to fade in and out before next beat
//     uint16_t bpm = 120;
//     uint16_t numBeats = 0;
//     long lastBpmTime = 0;
// };


