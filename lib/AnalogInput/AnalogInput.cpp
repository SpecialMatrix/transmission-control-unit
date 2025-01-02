#include "AnalogInput.h"

using namespace std;

AnalogInput::AnalogInput(uint16_t samples) : FiniteStateMachine(READING), _samples(samples) {
    
}

void AnalogInput::begin(int pin) {
    _pin = pin;
    pinMode(_pin, INPUT_PULLUP);

    for (int i=0; i<_samples; i++) {
        runningAverage.add(analogRead(_pin));
    }

    findMinMax(runningAverage.getFastAverage());
}

void AnalogInput::minDeadzone(float percent) {
    _minDeadzone = percent / 100.0f;
}

void AnalogInput::maxDeadzone(float percent) {
    _maxDeadzone = percent / 100.0f;
}

void AnalogInput::findMinMax(int value) {
    // Update the maximum if a larger value is found
    if(value > max) {
        max = value;
    }

    // Update the minimum if a smaller value is found
    // and is 30 less than the maximum
    if(value < min && value <= max - 30) {
        min = value;
    }
}

void AnalogInput::update() {
    switch(state()) {
        case READING: {
            const int reading = analogRead(_pin);

            if(reading >= recalibrateThreshold) {
                state(IDLE);
            } else {
                runningAverage.add(reading);
                float average = runningAverage.getFastAverage();
                findMinMax(average);
            }
            break;
        }

        case IDLE: {
            if(analogRead(_pin) < recalibrateThreshold) {
                state(RECALIBRATE);
            }
            break;
        }

        case RECALIBRATE: {
            min = numeric_limits<float>::max();
            max = 0;
            delay(50);
            for (int i=0; i<_samples; i++) {
                runningAverage.add(analogRead(_pin));
            }

            state(READING);
            break;
        }

        default:
            warnUnknownState("AnalogInput");
            break;
        }
}

float AnalogInput::travel() {
    float average = runningAverage.getFastAverage();

    float range = abs(max - min);
    float minDeadzoneScaled = min + range * _minDeadzone;
    float maxDeadzoneScaled = max - range * _maxDeadzone;

    float percentage = map(average, minDeadzoneScaled, maxDeadzoneScaled, 0.0f, 100.0f);
    percentage = constrain(percentage, 0.0f, 100.0f);

    return percentage;
}
