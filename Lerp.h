#include "Colors.h"

//typedef unsigned int uint;

class Lerper {
  public:
    Lerper() {
      lastIncreaseTime = -1.0; 
      currentPct = 0.0;
      lerpComplete = false;
      dir = IN;
    };
    
    static uint16_t lerpUint(uint16_t a, uint16_t b, float pct) {
          return a + ((b - a) * pct);
    }

    uint16_t setup(uint16_t c1, uint16_t c2, long inTime, long outTime, boolean pingPong = true, boolean loop = false, float precision = 0.01) {
      v1 = c1;
      v2 = c2;
      pctIncreaseAmt = precision;
      lastIncreaseTime = millis();
      intervalIn = inTime / (1.0 / precision);
      intervalOut = outTime / (1.0 / precision);
      currentPct = 0.0;
      lastPct = -1.0;
      lerpComplete = false;
      bLoop = loop;
      bReverse = false;
      bPingPong = pingPong;
      
//      Serial.println("TEST: ");
//      Serial.print("255 lerped to 0 by 0.5: ");
//      Serial.println(lerpUint(255, 0, 0.5));
    }
    
    void start() {
      lerpComplete = false;
      dir = IN;
      bReverse = false;
      v0 = getCurrentLerped();
    }
    
    void pause() {
      lerpComplete = true;
    }
    
    boolean isLerping() {
      return !lerpComplete;
    }
    
    void setStart(uint16_t v) {
      v1 = v;
    }
    
    void setEnd(uint16_t v) {
      v2 = v;
    }
    
    void update(boolean force = false) {
      if(lerpComplete && !force) return;
      
      long t = (dir == IN) ? intervalIn : intervalOut;
      if(millis() - lastIncreaseTime >= t) {
        if(dir == IN) {
          if(currentPct < 1.0) {
            currentPct += pctIncreaseAmt;
          } else {
            if(bPingPong) dir = OUT;
            else lerpComplete = true;
          }
        }
        
        else if(dir == OUT) {
         if(currentPct > 0.0) {
          currentPct -= pctIncreaseAmt;
         } else {
          if(bLoop) dir = IN;
          else lerpComplete = true;
         }
        }
        
        // safety //
        if(currentPct > 1.0) currentPct = 1.0;
        else if(currentPct < 0.0) currentPct = 0.0;
        
        lastIncreaseTime = millis();
      }
      
//      if(millis() - lastIncreaseTime >= interval) {
//        if(currentPct < 1.0 && !bReverse) {
//          currentPct += pctIncreaseAmt;
//        } else if(bReverse && currentPct > 0.0) {
//          currentPct -= pctIncreaseAmt;
//        } else {
//          if(bLoop) bReverse = !bReverse;
//          if(bPingPong && !bLoop) {
//            if(!bReverse) bReverse = true;
//            else lerpComplete = true;
//          }
//          else if(!bLoop || !bReverse) lerpComplete = true;
//        }
//        
//        // safety //
//        if(currentPct > 1.0) currentPct = 1.0;
//        else if(currentPct < 0.0) currentPct = 0.0;
//        
//        lastIncreaseTime = millis();
//      }
    }
    
    uint16_t getCurrentLerped() {
      uint16_t start = (v0 == -1) ? v1 : v0;
      uint16_t lerped = lerpUint(start, v2, currentPct); 
      v0 = -1;
      return lerped;
    }
    
    float getPct() {
//      Serial.print("Lerp PCT: ");
//      Serial.println(currentPct);
      return currentPct;
    }
    
    boolean hasChanged() {
//      uint8_t p = pow(10, _precision);
      uint8_t p = 1.0 / pctIncreaseAmt;
      float currentRounded = roundf(currentPct * p) / p;  
      float lastRounded = roundf(lastPct * p) / p;
      
      if(currentRounded != lastRounded) {
        lastPct = currentPct;
        return true;
      }
      
      return false;
    }
    
    boolean lerpComplete;
    
  protected:
      int v0 = -1;                  // initial color
      uint16_t v1;                  // start value of lerp
      uint16_t v2;                  // end value of lerp
      float pctIncreaseAmt;     // how much to increase every tick (i.e., precision)
      long lastIncreaseTime;    // last time pct was increased
      uint16_t intervalIn, intervalOut;        // how often to increment/decrement pct by pctIncreaseAmt
      float currentPct;
      float lastPct;
      boolean bLoop, bReverse, bPingPong;
      
      typedef enum Direction_t {
        IN,
        OUT
      } Direction_t;
      Direction_t dir;
};

class ColorLerper {
  public:
    ColorLerper() {
      r = new Lerper();
      g = new Lerper();
      b = new Lerper();
    }
    
    ~ColorLerper() {
      delete r;
      delete g;
      delete b;
    }
    
    void start() {
      r->start();
      b->start();
      g->start();
    }
    
    void pause() {
      r->pause();
      g->pause();
      b->pause();
    }
    
    void setup(Color_t startColor, Color_t endColor, long inTime, long outTime, bool loop = true, float precision = 0.01) {
      r->setup(startColor.r, endColor.r, inTime, outTime, loop, precision);
      g->setup(startColor.g, endColor.g, inTime, outTime, loop, precision);
      b->setup(startColor.b, endColor.b, inTime, outTime, loop, precision);
      
      Serial.print("Lerping ");
      Serial.print(startColor.r); Serial.print(", ");
      Serial.print(startColor.g); Serial.print(", ");
      Serial.print(startColor.b); Serial.print(" to ");
      Serial.print(endColor.r); Serial.print(", ");
      Serial.print(endColor.g); Serial.print(", ");
      Serial.print(endColor.b); Serial.println();
    }
    
    void setStartColor(Color_t c) {
      r->setStart(c.r);
      g->setStart(c.g);
      b->setStart(c.b);
    }
    
    void setEndColor(Color_t c) {
      r->setEnd(c.r);
      g->setEnd(c.g);
      b->setEnd(c.b);
    }
    
    void update() {
      r->update();
      g->update();
      b->update();
    }
    
    boolean hasChanged() {
      return (r->hasChanged() || g->hasChanged() || b->hasChanged());
    }
     
    Color_t getLerpedColor() {
      return Color_t((uint8_t)r->getCurrentLerped(), (uint8_t)g->getCurrentLerped(), (uint8_t)b->getCurrentLerped());
    }
    
    boolean isLerping() {
      return r->isLerping() || g->isLerping() || b->isLerping();
    }
    
    void print(uint16_t _delay = 500) {
      Serial.print("pct: ");
      Serial.print(r->getPct()); Serial.print(", ");
      Serial.print(g->getPct()); Serial.print(", ");
      Serial.print(b->getPct()); Serial.println();
      
      Serial.print("Color: ");
      Color_t c = getLerpedColor();
      Serial.print(c.r); Serial.print(", ");
      Serial.print(c.g); Serial.print(", ");
      Serial.print(c.b); Serial.println();
      
      delay(_delay);
    }
  
  private:
    Lerper* r;
    Lerper* g;
    Lerper* b;
    Color_t currentColor;
};
