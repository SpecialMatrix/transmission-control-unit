#include "Button.h"

void Button::begin(int pin, int debounce) {
    _pin = pin;
    _debounce = debounce;
    pinMode(_pin, INPUT_PULLUP);
}

int Button::pin() {
    return _pin;
}

unsigned long Button::debounce() {
    return _debounce;
}

void Button::update() {
    bool pressed = false;

    const bool state = digitalRead(pin());
    const unsigned long time = micros();

    if(state != lastButtonState) {
        lastDebounceTime = time;
    }

    if((time - lastDebounceTime) >= _debounce) {
        if(state != buttonState) {
            buttonState = state;
            if(buttonState == LOW) {
                pressed = true;
            }
        }
    }

    lastButtonState = state;
    buttonPressed = pressed;
}

bool Button::pressed() {
    return buttonPressed;
}
