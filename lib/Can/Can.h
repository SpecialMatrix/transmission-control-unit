#ifndef CAN_H
#define CAN_H

#ifdef NATIVE
#include "../../test/mock/MockFlexCAN_T4.h"
#else
#include <FlexCAN_T4.h>
#endif

#include <Arduino.h>
#include <functional>
#include "../dbc/r3.h"
#include "../dbc/tcs.h"

class Can {
  public:
    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16> &interface;
    
    Can(FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16> &interfaceRef) : interface(interfaceRef) { }
    void begin();
    void update();
    
    void onGroup0(const std::function<void(int rpm)> &function) {
        group0 = function;
    }

    void onShiftSettings(const std::function<void(int upDelay, int downDelay, int output, int timeout)> &function) {
        shiftSettings = function;
    }

    void onClutchSettings(const std::function<void(int start, int end, int friction, int autoLaunch)> &function) {
        clutchSettings = function;
    }

    void onSetClutch(const std::function<void(int position, int state)> &function) {
        setClutch = function;
    }

    void broadcastShiftSettings(int upDelay, int downDelay, int output, int timeout);
    void broadcastClutchSettings(int start, int end, int friction, int autoLaunch);    
    void broadcastClutch(int position, int percentage);
    void broadcastAnalogInput(int rightTravel, int rightRaw, int leftTravel=0, int leftRaw=0);

  private:
    std::function<void(int)> group0 = nullptr;
    std::function<void(int, int, int, int)> shiftSettings = nullptr;
    std::function<void(int, int, int, bool)> clutchSettings = nullptr;
    std::function<void(int, int)> setClutch = nullptr;

    void handleGroup0(const CAN_message_t &msg);
    void handleShiftSettings(const CAN_message_t &msg);
    void handleClutchSettings(const CAN_message_t &msg);
    void handleSetClutch(const CAN_message_t &msg);

    unsigned long lastEcuUpdate = 0;
};

#endif
