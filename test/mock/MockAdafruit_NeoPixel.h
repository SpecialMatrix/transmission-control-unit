#if !defined(MOCKADAFRUIT_NEOPIXEL_H) && defined(NATIVE)
#define MOCKADAFRUIT_NEOPIXEL_H

#include <Arduino.h>

#define NEO_GRB ((1 << 6) | (1 << 4) | (0 << 2) | (2))
#define NEO_KHZ800 0x0000

class Adafruit_NeoPixel {
  public:
    typedef uint8_t neoPixelType;
    
    Adafruit_NeoPixel(uint16_t n, int16_t pin, neoPixelType type = NEO_GRB + NEO_KHZ800) { };
    void begin() { };
    void setPixelColor(uint16_t n, uint32_t c) { };
    void show() { };
    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
        return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
    }
};

#endif