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
    BeatAnalyzer(AudioAnalyzePeak* _peakAnalyzer) {
      analyzer = _peakAnalyzer;
      numBeats = 0;
    }
    
    ~BeatAnalyzer() {
      delete analyzer;
    }
    
    void update() {
        if(analyzer->available()) {
          float peak = analyzer->read();
          
          // calculate an average of the last few frames of
          // audio data -- anything greater than this AND the running beat threshold
          // will count as a beat
          if(numFramesCounted >= AVERAGE_FRAMES) {
            avgPeak = peakRunningTotal / AVERAGE_FRAMES;
            peakRunningTotal = 0;
            numFramesCounted = 0;
            
//            if(avgPeak > 0.1) {
//              Serial.print("Average: ");
//              Serial.println(avgPeak);
//            }
          }
      
          peakRunningTotal += peak;
          numFramesCounted++;
        
        boolean doDetectBeat = (millis() - timer > BEAT_WAIT_INTERVAL);
        if(doDetectBeat && peak > beatCutoff && peak > BEAT_MIN && peak > avgPeak) {
          // got beat //
          Serial.print("BEAT: ");
          Serial.println(peak);
          numBeats++;
            
          // slightly increase beat threshold //
          beatCutoff = peak * 1.1;
            
          // reset timer //
          timer = millis();
          beatTime = 0;
        } else {
          if(beatTime <= BEAT_HOLD_TIME) {
            beatTime++;
          } else {
            // slowly decrease beat cutoff with time //
            beatCutoff *= BEAT_DECAY_RATE;
            beatCutoff = max(beatCutoff, BEAT_MIN);
          }
        }
      }
    }
    
    uint16_t getNumBeats(boolean _clear = false) {
      uint16_t n = numBeats;
      if(_clear) numBeats = 0;
      return n;
    }
    
   private:
      AudioAnalyzePeak* analyzer;
      float avgPeak;
      float peakRunningTotal = 0.0;
      int numFramesCounted = 0;
      long timer = 0;
      int numBeats;
      
      float beatCutoff = 0.5;
      long beatTime = 0;
};


