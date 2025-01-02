#if !defined(STORAGE_TEST_H) && defined(NATIVE)
#define STORAGE_TEST_H

#include <gtest/gtest.h>
#include <Arduino.h>
#include "../lib/Storage/Storage.h"

using namespace fakeit;

class StorageTest : public ::testing::Test {
  public:
    static void useDefaults() {
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read)).AlwaysReturn(0xFF);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::update)).AlwaysReturn();

        // upDelay 50
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(0)).AlwaysReturn(0x00);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(1)).AlwaysReturn(0x32);

        // downDelay 200
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(2)).AlwaysReturn(0x00);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(3)).AlwaysReturn(0xC8);

        // output 90
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(4)).AlwaysReturn(0x00);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(5)).AlwaysReturn(0x5A);

        // timeout 50
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(6)).AlwaysReturn(0x00);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(7)).AlwaysReturn(0x32);

        // start 1600
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(8)).AlwaysReturn(0x06);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(9)).AlwaysReturn(0x40);

        // end 1300
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(10)).AlwaysReturn(0x05);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(11)).AlwaysReturn(0x14);

        // friction
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(12)).AlwaysReturn(0x05);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(13)).AlwaysReturn(0x14);

        // autolaunch 0
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(14)).AlwaysReturn(0x00);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(15)).AlwaysReturn(0x00);
    }

  protected:
    void SetUp() override {
        ArduinoFakeReset();
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read)).AlwaysReturn(0xFF);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::update)).AlwaysReturn();
    }
};

TEST_F(StorageTest, defaults) {
    // Test that the defaults can be set
    storage.begin();
    
    // Test that every address is read
    for(int i=0; i<=15; i++) {
        Verify(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(i)).Once();
    }

    // UP_DELAY
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(0, 0x00)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(1, 0x32)).Once();

    // DOWN_DELAY_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(2, 0x00)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(3, 0xC8)).Once();

    // OUTPUT_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(4, 0x00)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(5, 0x5A)).Once();

    // TIMEOUT_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(6, 0x01)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(7, 0x2C)).Once();

    // START_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(8, 0x06)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(9, 0x40)).Once();

    // END_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(10, 0x05)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(11, 0x14)).Once();

    // FRICTION_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(12, 0x05)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(13, 0xDC)).Once();

    // AUTO_LAUNCH_ADDRESS
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(14, 0x00)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(15, 0x00)).Once();
}

TEST_F(StorageTest, setDefaults) {
    int c = 0;
    for(int i=0; i<=14; i+=2) {
        c += 1;
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(i)).AlwaysReturn(c*0x10);
        When(Method(ArduinoFake(EEPROM), EEPROMFake::read).Using(i+1)).AlwaysReturn(c*0x01);
    }

    storage.begin();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::write)).Exactly(0);

    EXPECT_EQ(storage.upDelay(), 0x1001);
    EXPECT_EQ(storage.downDelay(), 0x2002);
    EXPECT_EQ(storage.output(), 0x3003);
    EXPECT_EQ(storage.timeout(), 0x4004);
    EXPECT_EQ(storage.start(), 0x5005);
    EXPECT_EQ(storage.end(), 0x6006);
    EXPECT_EQ(storage.friction(), 0x7007);
    EXPECT_EQ(storage.autoLaunch(), true);
}

TEST_F(StorageTest, setValue) {
    storage.begin();
    ArduinoFake(EEPROM).ClearInvocationHistory();

    // UP_DELAY
    storage.upDelay();
    ASSERT_TRUE(storage.upDelay(0x01F4));
    ASSERT_FALSE(storage.upDelay(501));
    ASSERT_FALSE(storage.upDelay(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(0, 0x01)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(1, 0xF4)).Once();

    // DOWN_DELAY_ADDRESS
    ASSERT_TRUE(storage.downDelay(0x01F4));
    ASSERT_FALSE(storage.downDelay(501));
    ASSERT_FALSE(storage.downDelay(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(2, 0x01)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(3, 0xF4)).Once();

    // OUTPUT_ADDRESS
    ASSERT_TRUE(storage.output(0x0096));
    ASSERT_FALSE(storage.output(151));
    ASSERT_FALSE(storage.output(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(4, 0x00)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(5, 0x96)).Once();

    // TIMEOUT_ADDRESS
    ASSERT_TRUE(storage.timeout(0x01F4));
    ASSERT_FALSE(storage.timeout(501));
    ASSERT_FALSE(storage.timeout(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(6, 0x01)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(7, 0xF4)).Once();

    // START_ADDRESS
    ASSERT_TRUE(storage.start(0x0960));
    ASSERT_FALSE(storage.start(2401));
    ASSERT_FALSE(storage.start(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(8, 0x09)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(9, 0x60)).Once();

    // END_ADDRESS
    ASSERT_TRUE(storage.end(0x0960));
    ASSERT_FALSE(storage.end(2401));
    ASSERT_FALSE(storage.end(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(10, 0x09)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(11, 0x60)).Once();

    // FRICTION_ADDRESS
    ASSERT_TRUE(storage.friction(0x0960));
    ASSERT_FALSE(storage.friction(2401));
    ASSERT_FALSE(storage.friction(-1));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(12, 0x09)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(13, 0x60)).Once();

    // AUTO_LAUNCH_ADDRESS
    ASSERT_TRUE(storage.autoLaunch(0x0001));
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(14, 0x00)).Once();
    Verify(Method(ArduinoFake(EEPROM), EEPROMFake::update).Using(15, 0x01)).Once();
}

#endif