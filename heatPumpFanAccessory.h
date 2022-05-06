#include "common.h"
#include <HeatPump.h>
#include <homekit/characteristics.h>

extern "C" homekit_characteristic_t fanActive;
extern "C" homekit_characteristic_t fanState; // 0 inactive, 1 idle, 2 blowing air
extern "C" homekit_characteristic_t fanAuto;  // 0 manual, 1 auto
extern "C" homekit_characteristic_t fanSpeed; // 0-100

String fanSpeedToSetting() {
    int speed0_4 = floor((fanSpeed.value.float_value / 25) + 0.5);
    return speed0_4 != 0 ? String(speed0_4) : "QUIET";
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
    HK_INFO_LINE("Setting HP fan active: %s", value.bool_value ? "True" : "False");

    fanActive.value = value;
    
    #if HP_CONNECTED
        const char *fanSpeedSetting = fanSpeedToSetting().c_str();
        hp.setFanSpeed(fanActive.value.bool_value ? fanSpeedSetting : "QUIET");
    #endif
}

void fanAutoSetter(homekit_value_t value) {
    HK_INFO_LINE("Setting HP fan auto: %s", value.bool_value ? "True" : "False");

    fanAuto.value = value;
    if (fanAuto.value.bool_value && !fanActive.value.bool_value) { // if auto, tell homekit fan is on
        fanActive.value.bool_value = true;
        homekit_characteristic_notify(&fanActive, fanActive.value);
    }
    
    #if HP_CONNECTED
        const char *fanSpeedSetting = fanSpeedToSetting().c_str();
        hp.setFanSpeed(fanAuto.value.bool_value ? "AUTO" : fanSpeedSetting);
    #endif
}

void fanSpeedSetter(homekit_value_t value) {
    HK_INFO_LINE("Setting HP fan speed: %f", value.float_value);

    fanSpeed.value = value;
    
    #if HP_CONNECTED
        hp.setFanSpeed(fanSpeedToSetting().c_str());
    #endif
}

void initHeatPumpFanAccessory() {
    fanActive.setter = fanActiveSetter;
    fanAuto.setter = fanAutoSetter;
    fanSpeed.setter = fanSpeedSetter;
}