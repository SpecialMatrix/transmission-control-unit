#if !defined(TRANSMISSION_TEST_H) && defined(NATIVE)
#define TRANSMISSION_TEST_H

#include <gtest/gtest.h>
#include "../lib/Storage/Storage.h"
#include "../lib/Transmission/Transmission.h"

using namespace fakeit;
using namespace std;

class TransmissionTest : public ::testing::Test {
  protected:
    Mock<Storage> mockStorage;
    Mock<Servo> mockServo;

    Storage& storage;
    Servo& servo;

    Transmission transmission;

    TransmissionTest() : storage(mockStorage.get()), servo(mockServo.get()), transmission(storage, servo) {}

    float half = 0;

    void updateRepeat(int n) {
        for(int i=0; i<n; i++) {
            transmission.update();
        }
    }

    void shiftBoth() {
        transmission.shift(Transmission::Direction::UP);
        transmission.shift(Transmission::Direction::DOWN);
    }

    void SetUp() override {
        // Storage
        When(Method(mockStorage, upDelay)).AlwaysReturn(50);
        When(Method(mockStorage, downDelay)).AlwaysReturn(200);
        When(Method(mockStorage, output)).AlwaysReturn(90);
        When(Method(mockStorage, timeout)).AlwaysReturn(50);
        When(Method(mockStorage, start)).AlwaysReturn(1600);
        When(Method(mockStorage, end)).AlwaysReturn(1300);

        half = (storage.start() + storage.end()) * 0.5f;

        // Servo
        When(Method(mockServo, attach)).AlwaysReturn(1);
        When(Method(mockServo, writeMicroseconds)).AlwaysReturn();
        When(Method(mockServo, readMicroseconds)).AlwaysReturn(storage.start());

        When(Method(ArduinoFake(), pinMode)).AlwaysReturn();
        When(Method(ArduinoFake(), digitalWrite)).AlwaysReturn();
        When(Method(ArduinoFake(), millis)).AlwaysReturn(0);
        When(Method(ArduinoFake(), map)).AlwaysDo([](long x, long in_min, long in_max, long out_min, long out_max) {
            return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
        });

        When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String&))).AlwaysReturn(0);
        When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const char *))).AlwaysReturn(0);

        transmission.begin();
    }

    void TearDown() override {
        ArduinoFakeReset();
    }
};

TEST_F(TransmissionTest, begin) {
    // Servo should be attached and at the start position
    Verify(Method(mockServo, attach).Using(Storage::SERVO)).Once();
    Verify(Method(mockServo, writeMicroseconds).Using(storage.start())).Once();

    // ECU_UP, IA, IB, and LED should all be set to OUTPUT
    Verify(Method(ArduinoFake(), pinMode).Using(storage.ECU_UP, OUTPUT)).Once();
    Verify(Method(ArduinoFake(), pinMode).Using(storage.IA, OUTPUT)).Once();
    Verify(Method(ArduinoFake(), pinMode).Using(storage.IB, OUTPUT)).Once();
    Verify(Method(ArduinoFake(), pinMode).Using(13, OUTPUT)).Once();

    // ECU_UP should be HIGH
    Verify(Method(ArduinoFake(), digitalWrite).Using(storage.ECU_UP, HIGH)).Once();

    // Transmission state should be CLUTCH_MANUAL and the clutch position should be at the start
    EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_MANUAL);
    EXPECT_EQ(transmission.clutchPosition(), storage.start());
}

TEST_F(TransmissionTest, clutchManual) {
    std::vector<Transmission::State> states = {
        Transmission::State::CLUTCH_ADJUST,
        Transmission::State::SPARK_CUT,
        Transmission::State::CLUTCH_IN,
        Transmission::State::ENABLE_SOLENOID,
        Transmission::State::DISABLE_SOLENOID,
        Transmission::State::CLUTCH_OUT,
    };

    // Test that the clutch can only be manually adjusted from the CLUTCH_MANUAL state
    transmission.state(Transmission::State::CLUTCH_MANUAL);
    transmission.clutchInput(50);
    transmission.update();

    
    for(auto state : states) {
        transmission.state(state);
        transmission.clutchInput(50);
        transmission.update();
    }

    Verify(Method(mockServo, writeMicroseconds).Using(half)).Once();
}

TEST_F(TransmissionTest, clutchSetPosition) {
    When(Method(mockServo, writeMicroseconds)).AlwaysReturn();
    transmission.setClutchPosition(1200);
    Verify(Method(mockServo, writeMicroseconds).Using(1200)).Once();
}

TEST_F(TransmissionTest, clutchPosition) {
    mockServo.ClearInvocationHistory();

    // Thest that the clutch is at the start
    When(Method(mockServo, readMicroseconds)).AlwaysReturn(storage.start());
    EXPECT_EQ(transmission.clutchPosition(), storage.start());

    // Test that the clutch is halfway actuated
    When(Method(mockServo, readMicroseconds)).AlwaysReturn(half);
    EXPECT_EQ(transmission.clutchPosition(), half);

    // Tst that the clutch is at the end
    When(Method(mockServo, readMicroseconds)).AlwaysReturn(storage.end());
    EXPECT_EQ(transmission.clutchPosition(), storage.end());
}

TEST_F(TransmissionTest, clutchPercentage) {
    mockServo.ClearInvocationHistory();

    // Test that the clutchPercentage method returns the right values
    When(Method(mockServo, readMicroseconds)).AlwaysReturn(storage.start());
    EXPECT_EQ(transmission.clutchPercentage(), 0.0f);
    
    When(Method(mockServo, readMicroseconds)).AlwaysReturn(half);
    EXPECT_EQ(transmission.clutchPercentage(), 50.0f);
    
    When(Method(mockServo, readMicroseconds)).AlwaysReturn(storage.end());
    EXPECT_EQ(transmission.clutchPercentage(), 100);
}

TEST_F(TransmissionTest, up) {
    ArduinoFake().ClearInvocationHistory();

    int time = 0;

    // Shift 10 times at 4000 RPM
    for(int i=1; i<=10; i++) {
        transmission.setRpm(4000);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += 0);
        transmission.shift(Transmission::Direction::UP);
        EXPECT_EQ(transmission.state(), Transmission::State::SPARK_CUT);
        transmission.setRpm(3999);

        // Another shift should not change states
        shiftBoth();

        // UP_SPARK_CUT
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.upDelay());
        updateRepeat(10);
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.ECU_UP, LOW)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::ENABLE_SOLENOID);

        // Another shift should not change states
        shiftBoth();
        
        // UP_ENABLE_SOLENOID
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.output());
        transmission.update();
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.IA, HIGH)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::DISABLE_SOLENOID);

        // Another shift should not change states
        shiftBoth();

        // UP_DISABLE_SOLENOID
        transmission.update();
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.ECU_UP, HIGH)).Exactly(i);
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.IA, HIGH)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_MANUAL);
    }
}

TEST_F(TransmissionTest, upLowRPM) {
    int time = 0;
    
    // Shift 10 times below 4000 RPM
    for(int i=1; i<=10; i++) {
        transmission.setRpm(3999);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += 0);
        transmission.shift(Transmission::Direction::UP);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_IN);
        transmission.setRpm(4000);

        // Another shift should not change states
        shiftBoth();
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_IN);

        // CLUTCH_IN
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.downDelay());
        updateRepeat(10);
        EXPECT_EQ(transmission.state(), Transmission::State::ENABLE_SOLENOID);

        // Another shift should not change states
        shiftBoth();
        EXPECT_EQ(transmission.state(), Transmission::State::ENABLE_SOLENOID);

        // ENABLE_SOLENOID
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.output());
        updateRepeat(10);
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.IA, HIGH)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::DISABLE_SOLENOID);

        // Another shift should not change states
        shiftBoth();
        EXPECT_EQ(transmission.state(), Transmission::State::DISABLE_SOLENOID);

        // DISABLE_SOLENOID
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.timeout());
        transmission.update();
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.IA, LOW)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_OUT);

        // CLUTCH_MANUAL
        transmission.update();
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_MANUAL);
    }

    Verify(Method(ArduinoFake(), digitalWrite).Using(storage.ECU_UP, LOW)).Exactly(0);
}

TEST_F(TransmissionTest, down) {
    int time = 0;

    // Shift 10 times
    for(int i=1; i<=10; i++) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += 0);
        transmission.shift(Transmission::Direction::DOWN);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_IN);

        // Shift again
        transmission.shift(Transmission::Direction::UP);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_IN);

        // CLUTCH_IN
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.downDelay());
        updateRepeat(10);
        EXPECT_EQ(transmission.state(), Transmission::State::ENABLE_SOLENOID);

        // Shift again
        transmission.shift(Transmission::Direction::UP);
        EXPECT_EQ(transmission.state(), Transmission::State::ENABLE_SOLENOID);

        // ENABLE_SOLENOID
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.output());
        updateRepeat(10);
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.IB, HIGH)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::DISABLE_SOLENOID);

        // Shift again
        transmission.shift(Transmission::Direction::UP);
        EXPECT_EQ(transmission.state(), Transmission::State::DISABLE_SOLENOID);

        // DISABLE_SOLENOID
        updateRepeat(10);
        When(Method(ArduinoFake(), millis)).AlwaysReturn(time += storage.timeout());
        transmission.update();
        Verify(Method(ArduinoFake(), digitalWrite).Using(storage.IB, LOW)).Exactly(i);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_OUT);

        // CLUTCH_MANUAL
        updateRepeat(10);
        EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_MANUAL);
    }

    Verify(Method(ArduinoFake(), digitalWrite).Using(storage.ECU_UP, LOW)).Exactly(0);
}

TEST_F(TransmissionTest, earlyShiftUp) {
    transmission.setRpm(3999);
    transmission.state(Transmission::State::CLUTCH_OUT);
    transmission.shift(Transmission::Direction::UP);
    EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_IN);
}
TEST_F(TransmissionTest, earlyShiftDown) {
    transmission.state(Transmission::State::CLUTCH_OUT);
    transmission.shift(Transmission::Direction::DOWN);
    EXPECT_EQ(transmission.state(), Transmission::State::CLUTCH_IN);
}


TEST_F(TransmissionTest, unknownState) {
    transmission.state(404);
    transmission.update();
    Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String&))).Once();
}

#endif
