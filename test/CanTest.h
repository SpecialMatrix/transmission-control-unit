#if !defined(CAN_TEST_H) && defined(NATIVE)
#define CAN_TEST_H

#include <gtest/gtest.h>
#include <Arduino.h>
#include "../lib/Can/Can.h"
#include "mock/MockFlexCAN_T4.h"

using namespace fakeit;

class CanTest : public ::testing::Test {
protected:
    Mock<FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>> mockInterface;
    FlexCAN_T4<CAN1, RX_SIZE_16, TX_SIZE_16>& interface;
    Can can;

    CanTest() : interface(mockInterface.get()), can(interface) { }

    void SetUp() override {
        ArduinoFakeReset();
        When(Method(mockInterface, begin)).AlwaysReturn();
        When(Method(mockInterface, setBaudRate)).AlwaysReturn();
        When(Method(ArduinoFake(), millis)).AlwaysReturn(0);

        can.begin();
    }
};

TEST_F(CanTest, begin) {
    Verify(Method(mockInterface, begin)).Once();
    Verify(Method(mockInterface, setBaudRate).Using(1000000)).Once();
}

TEST_F(CanTest, group0) {
    When(Method(mockInterface, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x360u,
            .buf = {0x12, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
        };

        return 1;
    });
    
    bool called = false;
    
    can.onGroup0([&](double rpm) {
        EXPECT_EQ(rpm, 0x1234);
        called = true;
    });

    can.update();
    EXPECT_EQ(called, true) << "Callback 'group0' was not called.";
}

TEST_F(CanTest, shiftSettings) {
    When(Method(mockInterface, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x656u,
            .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
        };

        return 1;
    });
    
    bool called = false;

    can.onShiftSettings([&](double setUpDelay, double setDownDelay, double setOutput, double setTimeout) {
        EXPECT_EQ(setUpDelay, 0x1122);
        EXPECT_EQ(setDownDelay, 0x3344);
        EXPECT_EQ(setOutput, 0x5566);
        EXPECT_EQ(setTimeout, 0x7788);
        called = true;
    });

    can.update();

    EXPECT_EQ(called, true) << "Callback 'shiftSettings' was not called.";
}

TEST_F(CanTest, clutchSettings) {
    When(Method(mockInterface, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x658u,
            .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x01}
        };
        
        return 1;
    });
    
    bool called = false;

    can.onClutchSettings([&](double setStart, double setEnd, double setFriction, double setAutoLaunch) {
        EXPECT_EQ(setStart, 0x1122);
        EXPECT_EQ(setEnd, 0x3344);
        EXPECT_EQ(setFriction, 0x5566);
        EXPECT_EQ(setAutoLaunch, 0x0001);
        called = true;
    });

    can.update();
    EXPECT_EQ(called, true) << "Callback 'clutchSettings' was not called.";
}

TEST_F(CanTest, setClutch) {
    When(Method(mockInterface, read)).Do([](CAN_message_t &msg) -> int {
        msg = CAN_message_t {
            .id = 0x65au,
            .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00, 0x01}
        };

        return 1;
    });
    
    bool called = false;

    can.onSetClutch([&](double setPosition, double setState) {
        EXPECT_EQ(setPosition, 0x1122);
        EXPECT_EQ(setState, 0x3344);
        called = true;
    });

    can.update();
    EXPECT_EQ(called, true) << "Callback 'setClutch' was not called.";
}

TEST_F(CanTest, canTimeout) {
    When(Method(mockInterface, read)).AlwaysReturn(0);
    When(Method(ArduinoFake(), millis)).AlwaysReturn(100);

    int rpm = -1;
    can.onGroup0([&](double a){
        rpm = a;
    });

    can.update();
    EXPECT_EQ(rpm, 0);
}

#pragma region broadcastShiftSettings
TEST_F(CanTest, broadcastShiftSettings) {
    const CAN_message_t reference = {
        .id = 0x655u,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
    };

    CAN_message_t written;

    When(Method(mockInterface, write)).Do([&](const CAN_message_t& msg) -> int {
        written = msg;
        return 1;
    });

    can.broadcastShiftSettings(0, 0, 0, 0);
    When(Method(ArduinoFake(), millis)).AlwaysReturn(200);
    can.broadcastShiftSettings(0x1122, 0x3344, 0x5566, 0x7788);

    Verify(Method(mockInterface, write)).Once();
    EXPECT_EQ(written, reference) << "Incorrect 'shift_settings' message written to CAN interface";
}

TEST_F(CanTest, broadcastShiftSettingsFrequency) {
    When(Method(mockInterface, write)).AlwaysReturn(1);

    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastShiftSettings(0, 0, 0, 0);
    }

    Verify(Method(mockInterface, write)).Exactly(5);
}
#pragma endregion

#pragma region broadcastClutchSettings
TEST_F(CanTest, broadcastClutchSettings) {
    const CAN_message_t reference = {
        .id = 0x0657,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
    };

    CAN_message_t written;

    When(Method(mockInterface, write)).Do([&](const CAN_message_t& msg) -> int {
        written = msg;
        return 1;
    });

    can.broadcastClutchSettings(0, 0, 0, 0);
    When(Method(ArduinoFake(), millis)).AlwaysReturn(200);
    can.broadcastClutchSettings(0x1122, 0x3344, 0x5566, 0x7788);

    Verify(Method(mockInterface, write)).Once();
    EXPECT_EQ(written, reference) << "Incorrect 'analog_input' message written to CAN interface";
}

TEST_F(CanTest, broadcastClutchSettingsFrequency) {
    When(Method(mockInterface, write)).AlwaysReturn(1);

    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastClutchSettings(0, 0, 0, 0);
    }

    Verify(Method(mockInterface, write)).Exactly(5);
}
#pragma endregion

#pragma region broadcastClutch
TEST_F(CanTest, broadcastClutch) {
    const CAN_message_t reference = {
        .id = 0x659u,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x00, 0x00, 0x00, 0x00}
    };

    CAN_message_t written;

    When(Method(mockInterface, write)).Do([&](const CAN_message_t& msg) -> int {
        written = msg;
        return 1;
    });

    can.broadcastClutch(0, 0);
    When(Method(ArduinoFake(), millis)).AlwaysReturn(50);
    can.broadcastClutch(0x1122, 0x3344);

    Verify(Method(mockInterface, write)).Once();
    EXPECT_EQ(written, reference) << "Incorrect 'clutch' message written to CAN interface";
}

TEST_F(CanTest, broadcastClutchFrequency) {
    When(Method(mockInterface, write)).AlwaysReturn(1);

    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastClutch(0, 0);
    }

    Verify(Method(mockInterface, write)).Exactly(20);
}
#pragma endregion

#pragma region broadcastAnalogInput
TEST_F(CanTest, broadcastAnalogInput) {
    const CAN_message_t reference = {
        .id = 0x65bu,
        .buf = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88}
    };

    CAN_message_t result;

    When(Method(mockInterface, write)).Do([&](const CAN_message_t& msg) -> int {
        result = msg;
        return 1;
    });

    When(Method(ArduinoFake(), millis)).AlwaysReturn(100);
    can.broadcastAnalogInput(0x1122, 0x3344, 0x5566, 0x7788);

    EXPECT_EQ(result, reference) << "Incorrect 'analog_input' message written to CAN interface";
}

TEST_F(CanTest, broadcastAnalogInputFrequency) {
    When(Method(mockInterface, write)).AlwaysReturn(1);

    for(int i=0; i<1000; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        can.broadcastAnalogInput(0, 0, 0, 0);
    }

    Verify(Method(mockInterface, write)).Exactly(20);
}
#pragma endregion

#endif