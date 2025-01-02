#include "Can.h"

using namespace std;

void Can::begin() {
    interface.begin();
    interface.setBaudRate(1000000);
}

void Can::handleGroup0(const CAN_message_t &msg) {
    r3_group0_t buf;
    r3_group0_unpack(&buf, msg.buf, sizeof(msg.buf));

    if(group0) group0(
        r3_group0_rpm_decode(buf.rpm)
    );
}

void Can::handleShiftSettings(const CAN_message_t &msg) {
    tcs_set_shift_settings_t buf;
    tcs_set_shift_settings_unpack(&buf, msg.buf, sizeof(msg.buf));

    if(shiftSettings) shiftSettings(
        tcs_set_shift_settings_set_up_delay_decode(buf.set_up_delay),
        tcs_set_shift_settings_set_down_delay_decode(buf.set_down_delay),
        tcs_set_shift_settings_set_output_decode(buf.set_output),
        tcs_set_shift_settings_set_timeout_decode(buf.set_timeout)
    );
}

void Can::handleClutchSettings(const CAN_message_t &msg) {
    tcs_set_clutch_settings_t buf;
    tcs_set_clutch_settings_unpack(&buf, msg.buf, sizeof(msg.buf));

    if(clutchSettings) clutchSettings(
        tcs_set_clutch_settings_set_start_decode(buf.set_start),
        tcs_set_clutch_settings_set_end_decode(buf.set_end),
        tcs_set_clutch_settings_set_friction_decode(buf.set_friction),
        tcs_set_clutch_settings_set_auto_launch_decode(buf.set_auto_launch)
    );
}

void Can::handleSetClutch(const CAN_message_t &msg) {
    tcs_set_clutch_t buf;
    tcs_set_clutch_unpack(&buf, msg.buf, sizeof(msg.buf));

    if(setClutch) setClutch(
        tcs_clutch_position_decode(buf.set_position),
        tcs_set_clutch_set_state_decode(buf.set_state)
    );
}

void Can::update() {
    // Set the RPM to 0 if an ECU message has not been recieved in 100 ms
    if(millis() - lastEcuUpdate >= 100) {
        if(group0) group0(0);
    }

    CAN_message_t msg;
    if(!interface.read(msg)) return;

    switch(msg.id) {
        case R3_GROUP0_FRAME_ID: {
            lastEcuUpdate = millis();
            handleGroup0(msg);
            break;
        }

        case TCS_SET_SHIFT_SETTINGS_FRAME_ID: {
            handleShiftSettings(msg);
            break;
        }

        case TCS_SET_CLUTCH_SETTINGS_FRAME_ID: {
            handleClutchSettings(msg);
            break;
        }

        case TCS_SET_CLUTCH_FRAME_ID: {
            handleSetClutch(msg);
            break;
        }
    }
}

void Can::broadcastShiftSettings(int upDelay, int downDelay, int output, int timeout) {
    static unsigned long lastBroadastTime = 0;
    if(millis() - lastBroadastTime >= 1000 / 5) {
        CAN_message_t msg;
        msg.id = TCS_SHIFT_SETTINGS_FRAME_ID;

        tcs_shift_settings_t buf;
        buf.up_delay = tcs_shift_settings_up_delay_encode(upDelay);
        buf.down_delay = tcs_shift_settings_down_delay_encode(downDelay);
        buf.output = tcs_shift_settings_output_encode(output);
        buf.timeout = tcs_shift_settings_timeout_encode(timeout);
        tcs_shift_settings_pack(msg.buf, &buf, sizeof(msg.buf));

        interface.write(msg);
        lastBroadastTime = millis();
    }
}

void Can::broadcastClutchSettings(int start, int end, int friction, int autoLaunch) {
    static unsigned long lastBroadastTime = 0;
    if(millis() - lastBroadastTime >= 1000 / 5) {
        CAN_message_t msg;
        msg.id = TCS_CLUTCH_SETTINGS_FRAME_ID;

        tcs_clutch_settings_t buf;
        buf.start = tcs_clutch_settings_start_encode(start);
        buf.end = tcs_clutch_settings_end_encode(end);
        buf.friction = tcs_clutch_settings_friction_encode(friction);
        buf.auto_launch = tcs_clutch_settings_auto_launch_encode(autoLaunch);
        tcs_clutch_settings_pack(msg.buf, &buf, sizeof(msg.buf));

        interface.write(msg);
        lastBroadastTime = millis();
    }
}

void Can::broadcastClutch(int position, int percentage) {
    static unsigned long lastBroadastTime = 0;
    if(millis() - lastBroadastTime >= 1000 / 20) {        
        CAN_message_t msg;
        msg.id = TCS_CLUTCH_FRAME_ID;

        tcs_clutch_t buf;
        buf.position = tcs_clutch_position_encode(position);
        buf.position_percentage = tcs_clutch_position_percentage_encode(percentage);
        tcs_clutch_pack(msg.buf, &buf, sizeof(msg.buf));

        interface.write(msg);
        lastBroadastTime = millis();
    }
}

void Can::broadcastAnalogInput(int rightTravel, int rightRaw, int leftTravel, int leftRaw) {
    static unsigned long lastBroadastTime = 0;
    if(millis() - lastBroadastTime >= 1000 / 20) {
        CAN_message_t msg;
        msg.id = TCS_ANALOG_INPUT_FRAME_ID;

        tcs_analog_input_t buf;
        buf.input_right_travel = tcs_analog_input_input_right_travel_encode(rightTravel);
        buf.input_right_raw = tcs_analog_input_input_right_raw_encode(rightRaw);
        buf.input_left_travel = tcs_analog_input_input_left_travel_encode(leftTravel);
        buf.input_left_raw = tcs_analog_input_input_left_raw_encode(leftRaw);
        tcs_analog_input_pack(msg.buf, &buf, sizeof(msg.buf));

        interface.write(msg);
        lastBroadastTime = millis();
    }
}