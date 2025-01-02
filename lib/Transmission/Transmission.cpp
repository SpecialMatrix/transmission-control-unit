#include "Transmission.h"

using namespace std;

Transmission::Transmission(Servo &servoRef) : FiniteStateMachine(CLUTCH_MANUAL), servo(servoRef) {
    
}

void Transmission::begin() {
    servo.attach(storage.SERVO);
    writeMicroseconds(storage.start());

    pinMode(storage.ECU_UP, OUTPUT);
    pinMode(storage.IA, OUTPUT);
    pinMode(storage.IB, OUTPUT);
    pinMode(13, OUTPUT);

    digitalWrite(storage.ECU_UP, HIGH);
}

void Transmission::shift(Direction direction) {
    if(state() != CLUTCH_MANUAL && state() != CLUTCH_OUT) {
        Serial.println("\nAdditional shift attempted\n");
        return;
    }
    
    shiftStartTime = millis();

    const ShiftParameters up = { Direction::UP, storage.IA, storage.upDelay(), 0, false };
    const ShiftParameters upClutch = { Direction::UP, storage.IA, storage.downDelay(), 50, true };
    const ShiftParameters down = { Direction::DOWN, storage.IB, storage.downDelay(), 50, true };

    if(direction == Direction::UP) {
        Serial.println("\nUP");
        if(4000 <= rpm) {
            parameters = up;
            state(SPARK_CUT);
        } else {
            parameters = upClutch;
            state(CLUTCH_IN);
        }
        
    } else if(direction == Direction::DOWN) {
        Serial.println("\nDOWN");
        parameters = down;
        state(CLUTCH_IN);
    }
}

void Transmission::clutchInput(int value) {
    _input = constrain(value, 0.0f, 100.0f);
}

int Transmission::clutchPosition() {
    return servo.readMicroseconds();
}

int Transmission::clutchPercentage() {
    float normalizedValue = (float)(servo.readMicroseconds() - storage.start()) / (storage.end() - storage.start());
    normalizedValue = round(normalizedValue * 100);
    return normalizedValue;
}

void Transmission::writeMicroseconds(int value) {
    value = constrain(value, storage.end(), storage.start());
    servo.writeMicroseconds(value);
}

void Transmission::update() {
    switch(state()) {
        case CLUTCH_MANUAL: {
            int writeValue = map(_input, 0.0f, 100.0f, storage.start(), storage.end());
            writeMicroseconds(writeValue);
            break;
        }

        case SPARK_CUT: {
            runOnce([&](){
                Serial.println("SPARK CUT: " + String(millis() - shiftStartTime));
                digitalWrite(storage.ECU_UP, LOW);
            });
            
            // Wait for the up delay before enabling the solenoid
            waitAndSetState(storage.upDelay(), []{}, ENABLE_SOLENOID);
            break;
        }

        case CLUTCH_IN: {
            runOnce([&](){
                Serial.println("CLUTCH IN: " + String(millis() - shiftStartTime));
                writeMicroseconds(storage.end());
            });

            // Wait for the down delay before powering the solenoid
            waitAndSetState(parameters.delay, []{}, ENABLE_SOLENOID);
            break;
        }

        case ENABLE_SOLENOID: {
            runOnce([&](){
                Serial.println("SOLENOID ENABLE: " + String(millis() - shiftStartTime));
                digitalWrite(parameters.pin, HIGH);
                digitalWrite(13, HIGH);
            });

            // Wait the output duration before disabling the solenoid
            waitAndSetState(storage.output(), []{}, DISABLE_SOLENOID);
            break;
        }

        case DISABLE_SOLENOID: {
            runOnce([&](){
                Serial.println("SOLENOID DISABLE: " + String(millis() - shiftStartTime));
                digitalWrite(parameters.pin, LOW);
                digitalWrite(storage.ECU_UP, HIGH);
                digitalWrite(13, LOW);
            });
            
            if(parameters.useClutch) {
                // Wait the timeout before switching states if using the clutch
                waitAndSetState(storage.timeout(), []{}, CLUTCH_OUT);
            } else {
                state(CLUTCH_MANUAL);
            }
            break;
        }

        case CLUTCH_OUT: {
            runOnce([&](){
                Serial.println("CLUTCH OUT: " + String(millis() - shiftStartTime));
                writeMicroseconds(storage.start());
            });

            state(CLUTCH_MANUAL);
            break;
        }

        default: {
            warnUnknownState("AnalogInput");
            break;
        }
    }
}
