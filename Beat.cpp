#include "Beat.h"

BeatAnalyzer::BeatAnalyzer(AudioAnalyzePeak* _peakAnalyzer, beat_callback _beatCallback) {
	peakAnalyzer = _peakAnalyzer;
	onBeatCallback = _beatCallback;
	numBeats = 0;
}

BeatAnalyzer::~BeatAnalyzer() {
    // cleanup necessary? //
 }

void BeatAnalyzer::update() {    
	if(peakAnalyzer->available()) {
		float peak = peakAnalyzer->read();
          
		// calculate an average of the last few frames of
		// audio data -- anything greater than this AND the running beat threshold
		// will count as a beat
		if(millis() - lastSampleTime >= averagingIntervalMs) {
			avgPeak = peakRunningTotal / AVERAGE_FRAMES;
			peakRunningTotal = 0;
			averagingTimer = 0;
		}
      	
      	// add sample to running total //
		peakRunningTotal += peak;
		lastSampleTime = millis();
        
        // only check for beat and adjust beatCutoff if beatWaitIntervalMs has passed //
		boolean doDetectBeat = (millis() - lastPeakTime > beatWaitIntervalMs);
		// a beat is counted when the current peak value is greater than beatMinPeak
		// AND greater than the running average
		if(doDetectBeat && peak > beatCutoff && peak > beatMinPeak && peak > avgPeak) {
			numBeats = (numBeats < 65536) ? numBeats + 1 : 1; // reset numBeats to 1 if it is greater than 16 bits
          
			beatInfo.numBeats = numBeats;
			beatInfo.lastPeak = peak;
            
			// slightly increase beat threshold //
			beatCutoff = peak * 1.1;
          
			// callback //
			(*onBeatCallback)(beatInfo);
            
			// reset timer //
			lastPeakTime = millis();
			beatDecayDelay = 0;
		} else {
			// no beat //
			if(beatDecayDelay <= BEAT_HOLD_TIME) {
				beatDecayDelay++;
			} else {
				// slowly decrease beat cutoff with time //
				beatCutoff *= BEAT_DECAY_RATE;
				beatCutoff = max(beatCutoff, beatMinPeak);
			}
		}
	}
}

uint16_t BeatAnalyzer::getNumBeats(boolean _clear = false) {
	uint16_t n = numBeats;
	if(_clear) numBeats = 0;
	return n;
}

boolean BeatAnalyzer::gotBeat() {
	return (numBeats > 0);
}