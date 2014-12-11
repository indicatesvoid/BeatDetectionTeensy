#ifndef UTILS_H
#define UTILS_H

//#include <OctoWS2811.h>

static void setGlobalColor(unsigned int color, OctoWS2811* leds) {
  for(size_t i = 0; i < leds->numPixels(); i++) {
    leds->setPixel(i, color);
  }
  leds->show();
}

static void setGlobalColor(uint8_t r, uint8_t g, uint8_t b, OctoWS2811* leds) {
  for(size_t i = 0; i < leds->numPixels(); i++) {
    leds->setPixel(i, r, g, b);
  }
  leds->show();
}

#endif
