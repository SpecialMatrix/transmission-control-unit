#include "Storage.h"

using namespace std;

void Storage::begin() {
    read(UP_DELAY_ADDRESS, 50);
    read(DOWN_DELAY_ADDRESS, 200);
    read(OUTPUT_ADDRESS, 90);
    read(TIMEOUT_ADDRESS, 300);
    read(START_ADDRESS, 1600);
    read(END_ADDRESS, 1300);
    read(FRICTION_ADDRESS, 1500);
    read(AUTO_LAUNCH_ADDRESS, 0);
}

int Storage::read(int address, uint16_t defaultValue) {
    uint16_t data = read(address);

    if(data == 0xFFFF) {
        save(address, defaultValue);
        data = defaultValue;
    }
    
    return data;
}

int Storage::read(int address) const {
    uint16_t data = (EEPROM.read(address) << 8) | EEPROM.read(address + 1);
    return data;
}

void Storage::save(int address, uint16_t value) {
    int a = (value >> 8) & 0xFF;
    int b = value & 0xFF;

    EEPROM.update(address, a);
    EEPROM.update(address + 1, b);
}

int Storage::upDelay() const { return read(UP_DELAY_ADDRESS); }
bool Storage::upDelay(int value) {
    if(value < 0 || 500 < value) {
        return false;
    } else {
        save(UP_DELAY_ADDRESS, value);
        return true;
    }
}

int Storage::downDelay() const { return read(DOWN_DELAY_ADDRESS); }
bool Storage::downDelay(int value) {
    if(value < 0 || 500 < value) {
        return false;
    } else {
        save(DOWN_DELAY_ADDRESS, value);
        return true;
    }
}

int Storage::output() const { return read(OUTPUT_ADDRESS); }
bool Storage::output(int value) {
    if(value < 0 || 150 < value) {
        return false;
    } else {
        save(OUTPUT_ADDRESS, value);
        return true;
    }
}

int Storage::timeout() const { return read(TIMEOUT_ADDRESS); }
bool Storage::timeout(int value) {
    if(value < 0 || 500 < value) {
        return false;
    } else {
        save(TIMEOUT_ADDRESS, value);
        return true;
    }
}

int Storage::start() const { return read(START_ADDRESS); }
bool Storage::start(int value) {
    if(value < 0 || 2400 < value) {
        return false;
    } else {
        save(START_ADDRESS, value);
        return true;
    }
}

int Storage::end() const { return read(END_ADDRESS); }
bool Storage::end(int value) {
    if(value < 0 || 2400 < value) {
        return false;
    } else {
        save(END_ADDRESS, value);
        return true;
    }
}

int Storage::friction() const { return read(FRICTION_ADDRESS); }
bool Storage::friction(int value) {
    if(value < 0 || 2400 < value) {
        return false;
    } else {
        save(FRICTION_ADDRESS, value);
        return true;
    }
}

bool Storage::autoLaunch() const { return read(AUTO_LAUNCH_ADDRESS); }
bool Storage::autoLaunch(bool value) {
    save(AUTO_LAUNCH_ADDRESS, value);
    return true;
}
