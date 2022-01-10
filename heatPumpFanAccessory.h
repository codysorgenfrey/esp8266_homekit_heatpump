#include "common.h"
#include <HeatPump.h>

extern "C" homekit_characteristic_t fanActive;
extern "C" homekit_characteristic_t fanState; // 0 inactive, 1 idle, 2 blowing air
extern "C" homekit_characteristic_t fanAuto;  // 0 manual, 1 auto
extern "C" homekit_characteristic_t fanSpeed; // 0-100

const char * fanSpeedToSetting() {
    int speed0_4 = floor((fanSpeed.value.float_value / 25) + 0.5);
    return speed0_4 != 0 ? String(speed0_4).c_str() : "QUIET";
}

void heatPumpFanAccessorySettingsChanged() {
    bool hpFanState = hp.getOperating() ? 2 : 1;
    const char *hpFanMode = hp.getFanSpeed();
    bool hpFanAuto = hpFanMode == "AUTO";
    bool hpFanActive = hpFanMode != "QUIET";

    double hpFanSpeed = 0.0;
    if (!hpFanAuto && hpFanActive) {
        char *end;
        hpFanSpeed = strtod(hpFanMode, &end) * 25.0; 
    }

    // Active
    if (fanActive.value.bool_value != hpFanActive) {// Fan is always on
        fanActive.value.bool_value = hpFanActive;
        homekit_characteristic_notify(&fanActive, fanActive.value);
    }

    // State
    if (fanState.value.int_value != hpFanState) {
        fanState.value.int_value = hpFanState;
        homekit_characteristic_notify(&fanState, fanState.value);
    }

    // Auto
    if (fanAuto.value.bool_value != hpFanAuto) {
        fanAuto.value.bool_value = hpFanAuto;
        homekit_characteristic_notify(&fanAuto, fanAuto.value);
    }

    // Speed
    if (fanSpeed.value.float_value != hpFanSpeed) {
        fanSpeed.value.float_value = hpFanSpeed;
        homekit_characteristic_notify(&fanSpeed, fanSpeed.value);
    }
    
}

void fanActiveSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP fan active: ");
        Serial.println(value.bool_value);
    #endif

    fanActive.value = value;
    
    #if !HP_DISCONNECTED
        const char *fanSpeedSetting = fanSpeedToSetting();
        hp.setFanSpeed(fanActive.value.bool_value ? fanSpeedSetting : "QUIET");
        hp.update();
    #endif
}

void fanAutoSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP fan auto: ");
        Serial.println(value.bool_value);
    #endif

    fanAuto.value = value;
    
    #if !HP_DISCONNECTED
        const char *fanSpeedSetting = fanSpeedToSetting();
        hp.setFanSpeed(fanAuto.value.bool_value ? "AUTO" : fanSpeedSetting);
        hp.update();
    #endif
}

void fanSpeedSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP fan speed: ");
        Serial.println(value.float_value);
    #endif

    fanSpeed.value = value;
    
    #if !HP_DISCONNECTED
        hp.setFanSpeed(fanSpeedToSetting());
        hp.update();
    #endif
}

void initHeatPumpFanAccessory() {
    fanActive.setter = fanActiveSetter;
    fanAuto.setter = fanAutoSetter;
    fanSpeed.setter = fanSpeedSetter;
}