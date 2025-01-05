#if !defined(FINITE_STATE_MACHINE_TEST_H) && defined(NATIVE)
#define FINITE_STATE_MACHINE_TEST_H

#include <gtest/gtest.h>
#include "../lib/FiniteStateMachine/FiniteStateMachine.h"

using namespace fakeit;

class FiniteStateMachineTest : public ::testing::Test {
  protected:
    class TestFSM : public FiniteStateMachine {
      public:
        enum State {
            IDLE,
            ONE,
            TWO,
            THREE,
        };

        TestFSM() : FiniteStateMachine(IDLE) {

        }
    };

    TestFSM fsm;

    void SetUp() override {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(0);
        When(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String&))).AlwaysReturn(0);
    }

    void TearDown() override {
        ArduinoFakeReset();
    }
};

TEST_F(FiniteStateMachineTest, changeState) {
    EXPECT_TRUE(fsm.state() == TestFSM::State::IDLE);
    fsm.state(TestFSM::State::ONE);
    EXPECT_TRUE(fsm.state() == TestFSM::State::ONE);
}

TEST_F(FiniteStateMachineTest, runOnce) {
    int count = 0;

    for(int i=0; i<10; i++) fsm.runOnce([&](){ count++; });
    EXPECT_EQ(count, 1);

    fsm.state(TestFSM::State::ONE);
    for(int i=0; i<10; i++) fsm.runOnce([&](){ count++; });
    EXPECT_EQ(count, 2);

    fsm.state(TestFSM::State::TWO);
    for(int i=0; i<10; i++) fsm.runOnce([&](){ count++; });
    EXPECT_EQ(count, 3);

    fsm.state(TestFSM::State::THREE);
    for(int i=0; i<10; i++) fsm.runOnce([&](){ count++; });
    EXPECT_EQ(count, 4);
}

TEST_F(FiniteStateMachineTest, waitAndSetState) {
    bool called = false;

    for(int i=0; i<=100; i+=10) {
        When(Method(ArduinoFake(), millis)).AlwaysReturn(i);
        EXPECT_FALSE(called);
        fsm.waitAndSetState(100, [&](){ called = true; }, TestFSM::State::ONE);
    }
    
    EXPECT_TRUE(called);
    EXPECT_EQ(fsm.state(), TestFSM::State::ONE);

    int count = 0;
    for(int i=0; i<10; i++) fsm.runOnce([&](){ count++; });

    EXPECT_EQ(count, 1);
}

TEST_F(FiniteStateMachineTest, unknownState) {
    fsm.warnUnknownState("TestFSM");
    Verify(OverloadedMethod(ArduinoFake(Serial), println, size_t(const String&))).Once();
}

#endif
