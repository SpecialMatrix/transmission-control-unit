#ifndef MAIN_CPP
#define MAIN_CPP

#include <Arduino.h>
#include "../lib/Storage/Storage.h"
#include "../lib/AnalogInput/AnalogInput.h"
#include "../lib/Transmission/Transmission.h"
#include "../lib/Button/Button.h"
#include "../lib/Can/Can.h"

#ifdef NATIVE
    #include "../test/mock/MockFlexCAN_T4.h"
    #include "../test/mock/MockServo.h"
    #include "../test/mock/MockAdafruit_NeoPixel.h"

    fakeit::Mock<Servo> mockServo;
    Servo& servo = mockServo.get();

    fakeit::Mock<FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>> mockInterface;
    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>& interface = mockInterface.get();
#else
    #include <FlexCAN_T4.h>
    #include <Servo.h>
    #include <Adafruit_NeoPixel.h>

    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16> interface;
    Servo servo;
#endif

Adafruit_NeoPixel pixels(1, 5, NEO_GRB + NEO_KHZ800);
Can can(interface);
Transmission transmission(servo);
Button up;
Button down;
AnalogInput clutchRight(512);

void setupCallbacks();
void broadcast();

void setup() {
    pixels.begin();
    pixels.setPixelColor(0, pixels.Color(0, 140, 0));
    pixels.show();

    can.begin();
    storage.begin();
    
    up.begin(storage.UP);
    down.begin(storage.DOWN);

    clutchRight.begin(storage.CLUTCH_RIGHT);
    clutchRight.minDeadzone(10);
    clutchRight.maxDeadzone(20);

    transmission.begin();

    setupCallbacks();
}

void loop() {
    // Read CAN
    can.update();

    // Handle input
    up.update();
    down.update();
    clutchRight.update();

    if(up.pressed()) {
        transmission.shift(Transmission::Direction::UP);
    } else if(down.pressed()) {
        transmission.shift(Transmission::Direction::DOWN);
    }

    transmission.clutchInput(clutchRight.travel());

    // Update finite-state machines
    transmission.update();
    
    // Send data over CAN
    broadcast();
}

void setupCallbacks() {
    can.onGroup0([](int rpm){
        transmission.rpm = rpm;
    });

    can.onShiftSettings([](int upDelay, int downDelay, int output, int timeout){
        storage.upDelay(upDelay);
        storage.downDelay(downDelay);
        storage.output(output);
        storage.timeout(timeout);
    });

    can.onClutchSettings([](int start, int end, int friction, int autoLaunch){
        storage.start(start);
        storage.end(end);
        storage.friction(friction);
        storage.autoLaunch(autoLaunch);
    });

    can.onSetClutch([](int state, int position){
        transmission.state(state);
        transmission.clutchInput(position);
    });
}

void broadcast() {
    can.broadcastShiftSettings(
        storage.upDelay(),
        storage.downDelay(),
        storage.output(),
        storage.timeout()
    );

    can.broadcastClutchSettings(
        storage.start(),
        storage.end(),
        storage.friction(),
        storage.autoLaunch()
    );

    can.broadcastClutch(
        transmission.clutchPosition(),
        transmission.clutchPercentage()
    );

    can.broadcastAnalogInput(
        clutchRight.travel(),
        analogRead(storage.CLUTCH_RIGHT)
    );
}

#endif