#ifndef ANALOG_INPUT_H
#define ANALOG_INPUT_H

#include <Arduino.h>
#include <RunningAverage.h>
#include <limits>
#include "../FiniteStateMachine/FiniteStateMachine.h"

class AnalogInput : public FiniteStateMachine {
  public:
    const int recalibrateThreshold = 800;

    enum State {
        READING,
        IDLE,
        RECALIBRATE,
    };

    AnalogInput() = delete;
    AnalogInput(uint16_t samples);
    void begin(int pin);
    void minDeadzone(float percent);
    void maxDeadzone(float percent);
    void update();
    float travel();
    
  private:
    int _pin;
    uint16_t _samples;
    RunningAverage runningAverage = RunningAverage(_samples);

    float _minDeadzone = 0;
    float _maxDeadzone = 0;

    float min = std::numeric_limits<float>::max();
    float max = 0;

    void findMinMax(int value);
};

#endif
