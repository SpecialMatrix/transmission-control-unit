#include "FiniteStateMachine.h"

using namespace std;

FiniteStateMachine::FiniteStateMachine(State initialState) : currentState(initialState) {

}

FiniteStateMachine::State FiniteStateMachine::state() const {
    return currentState;
}

void FiniteStateMachine::state(State newState) {
    lastStateChangeTime = millis();
    currentState = newState;
    runAction = true;
}

void FiniteStateMachine::warnUnknownState(const char* name) {
    Serial.println(String(name) + ": Unknown state '" + String(state()) + "'");
}

void FiniteStateMachine::runOnce(const function<void()> &action) {
    if(runAction) {
        action();
        runAction = false;
    }
}

void FiniteStateMachine::waitAndSetState(unsigned long duration, const function<void()> &action, State newState) {
    if(millis() - lastStateChangeTime >= duration) {
        action();
        state(newState);
    }
}
