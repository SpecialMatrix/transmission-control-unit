#ifndef FINITE_STATE_MACHINE_H
#define FINITE_STATE_MACHINE_H

#include <Arduino.h>
#include <functional>

class FiniteStateMachine {
  public:
    typedef int State;
    
    FiniteStateMachine() = delete;
    FiniteStateMachine(State initialState);
    State state() const;
    void state(State newState);
    void warnUnknownState(const char* name);

    // Run the action once. Reset by changing states.
    void runOnce(const std::function<void()> &action);

    // Run the action after waiting the duration, then change states
    void waitAndSetState(unsigned long duration, const  std::function<void()> &action, State newState);
    
  private:
    State currentState;
    unsigned long lastActionTime = 0;
    unsigned long lastStateChangeTime = 0;
    bool runAction = true;
};

#endif
