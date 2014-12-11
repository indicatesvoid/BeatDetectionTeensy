#ifndef COLORS_H
#define COLORS_H

#define RED    0xFF0000
#define GREEN  0x00FF00
#define BLUE   0x0000FF
#define YELLOW 0xFFFF00
#define PINK   0xFF1088
#define ORANGE 0xE05800
#define WHITE  0xFFFFFF

typedef struct Color_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  
  Color_t() { };
  Color_t(uint8_t _r, uint8_t _g, uint8_t _b) : r(_r), g(_g), b(_b) { };
  
  // comparison operator overloads //
  bool operator== (Color_t &c2) {
    return (r == c2.r &&
            g == c2.g &&
            b == c2.b);
  }
  
  bool operator!=(Color_t &c2) {
    return !operator==(c2);
  }
  
} Color_t;

#endif
