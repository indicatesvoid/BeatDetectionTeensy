####Teensy Beat Detection

## Important Note
This was a quick experiment and isn't necessarily pristine, guaranteed, or fully vetted. I hope to get around to polishing it up a bit in the future, but it is currently on hold.

There are a number of improvements (API improvements, FFT support, etc) in the works but not present on this repo.

Please note that the 'lib' branch contains the most up-to-date code here.

## Description
This example uses the Teensy's built in audio libraries to do a simple form of beat detection.

The 'BeatAnalyzer' class (found in Beat.h) takes a reference to an AudioAnalyzePeak object in its constructor. It then calculates a running average of the song's peak intensity, comparing that against the current peak intensity. If it finds a beat, it simply logs out to the Serial console. A beat is only considered valid if it meets the following criteria:

1. Its peak intensity is greater than the running average
2. Its peak intensity is greater than a minimum threshold (BEAT_MIN)
3. Its peak intensity is greater than or equal to that of the last detected beat
3. It occurs more than n-number of milliseconds after the last valid beat (where n is defined by BEAT_WAIT_INTERVAL)

Criteria #3 self-adjusts over time -- with each beat, the cutoff value for the next one will be set at slightly over the peak intensity of the current beat. Over time, this cutoff value will decay until the next beat, by a factor defined by BEAT_DECAY_RATE.

These criteria, when combined, do a fairly good job of detecting beats with a minimum of false positives. Adjusting the defines at the top of Beat.h may net you different results.

Future iterations may include an FFT pre-pass to isolate valid beats to a specific frequency range, or only test for beats within the frequency range that has the highest peak intensity average.


####Notes
http://www.airtightinteractive.com/2013/10/making-audio-reactive-visuals/
http://www.gamedev.net/page/resources/_/technical/math-and-physics/beat-detection-algorithms-r1952
https://github.com/ddf/Minim/blob/master/src/ddf/minim/analysis/BeatDetect.java
