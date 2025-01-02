#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
  public:
    void begin(int pin, int debounce=5000);
    int pin();
    unsigned long debounce();
    void update();
    bool pressed();

  private:
    int _pin;
    unsigned long _debounce;
    
    bool buttonState = HIGH;
    bool lastButtonState = HIGH;
    bool buttonPressed = false;
    unsigned long lastDebounceTime = 0;
};

#endif
