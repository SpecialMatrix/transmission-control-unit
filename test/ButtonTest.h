#if !defined(BUTTON_TEST_H) && defined(NATIVE)
#define BUTTON_TEST_H

#include <gtest/gtest.h>
#include <Arduino.h>
#include "../lib/Button/Button.h"

using namespace fakeit;

class ButtonTest : public ::testing::Test {
protected:
    const int pin = 6;
    Button button;

    void SetUp() override {
        ArduinoFakeReset();
        When(Method(ArduinoFake(), pinMode).Using(pin, INPUT_PULLUP)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalRead)).AlwaysReturn(LOW);
        When(Method(ArduinoFake(), micros)).AlwaysReturn(0);
    }
};

TEST_F(ButtonTest, begin) {
    button.begin(pin, 1000);
    Verify(Method(ArduinoFake(), pinMode).Using(pin, INPUT_PULLUP)).Once();
    EXPECT_EQ(button.pin(), pin);
    EXPECT_EQ(button.debounce(), 1000);
}

TEST_F(ButtonTest, update) {
    button.begin(pin);
    button.update();
    Verify(Method(ArduinoFake(), digitalRead).Using(pin)).Once();
    Verify(Method(ArduinoFake(), micros)).Once();
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonTest, defaultDebouncePressed) {
    button.begin(pin);

    When(Method(ArduinoFake(), digitalRead).Using(pin)).Return(LOW, LOW, HIGH);
    When(Method(ArduinoFake(), micros)).Return(5000, 1000, 15000);

    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_TRUE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonTest, defaultBounceDetection) {
    button.begin(pin);

    When(Method(ArduinoFake(), digitalRead).Using(pin)).Return(LOW, HIGH, LOW);
    When(Method(ArduinoFake(), micros)).Return(4000, 8000, 12000);

    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonTest, customBounce) {
    button.begin(pin, 10000);

    When(Method(ArduinoFake(), digitalRead).Using(pin)).Return(LOW, LOW, HIGH);
    When(Method(ArduinoFake(), micros)).Return(10000, 20000, 30000);

    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_TRUE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonTest, customBounceDetection) {
    button.begin(pin, 10000);

    When(Method(ArduinoFake(), digitalRead).Using(pin)).Return(LOW, LOW, HIGH);
    When(Method(ArduinoFake(), micros)).Return(6000, 12000, 16000);

    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonTest, consecutive) {
    button.begin(pin, 1000);

    When(Method(ArduinoFake(), digitalRead).Using(pin)).Return(LOW, LOW, HIGH, HIGH);
    When(Method(ArduinoFake(), micros)).Return(1000, 2000, 3000, 4000);

    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_TRUE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
}

TEST_F(ButtonTest, consecutive2) {
    button.begin(pin, 1000);

    When(Method(ArduinoFake(), digitalRead).Using(pin)).Return(LOW, HIGH, HIGH);
    When(Method(ArduinoFake(), micros)).Return(1000, 2000, 3000);

    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
    button.update();
    EXPECT_FALSE(button.pressed());
}

#endif