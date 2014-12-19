#ifndef UTILS_H
#define UTILS_H

static void setGlobalColor(unsigned int color) {
  CRGB* leds = FastLED.leds();
  uint16_t numLeds = FastLED.size();
  for(size_t i = 0; i < numLeds; i++) {
    leds[i] = color;
  }
  FastLED.show();
}

static void setGlobalColor(uint8_t r, uint8_t g, uint8_t b) {
  CRGB* leds = FastLED.leds();
  uint16_t numLeds = FastLED.size();
  for(size_t i = 0; i < numLeds; i++) {
    leds[i].setRGB(r, g, b);
  }
  FastLED.show();
}

#endif
