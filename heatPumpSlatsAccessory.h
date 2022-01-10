#include "common.h"
#include <HeatPump.h>

extern "C" homekit_characteristic_t slatType;      // 0 horizontal, 1 vertical
extern "C" homekit_characteristic_t slatState;     // 0 fixed, 1 jammed, 2 swinging
extern "C" homekit_characteristic_t slatCurAngle;  // -90 to 90 where -90 is straight out and 90 is straight down
extern "C" homekit_characteristic_t slatTarAngle;  // -90 to 90 where -90 is straight out and 90 is straight down
extern "C" homekit_characteristic_t slatSwingMode; // 0 disabled, 1 enabled

void heatPumpSlatsAccessorySettingsChanged() {
    /* Air direction (vertical): 1-5, SWING, or AUTO */
    const char *slatMode = hp.getVaneSetting();
    bool hpSlatSwing = slatMode == "SWING";
    bool hpSlatAuto = slatMode == "AUTO";
    int hpSlatState = hpSlatSwing ? 2 : 0;
    int hpSlatAngle = 0;

    if (hpSlatAuto) {
        const char *hpMode = hp.getModeSetting();
        if (hpMode == "HEAT")
            hpSlatAngle = 90;
        else if (hpMode == "COOL" || hpMode == "DRY")
            hpSlatAngle = -90;
    } else if (!hpSlatSwing)
        hpSlatAngle = -90 + ((atoi(slatMode) - 1) * 45);

    // State
    if (slatState.value.int_value != hpSlatState) {
        slatState.value.int_value = hpSlatState;
        homekit_characteristic_notify(&slatState, slatState.value);
    }

    // Swing
    if (slatSwingMode.value.bool_value != hpSlatSwing) {
        slatSwingMode.value.bool_value = hpSlatSwing;
        homekit_characteristic_notify(&slatSwingMode, slatSwingMode.value);
    }

    // Angle
    if (slatCurAngle.value.int_value != hpSlatAngle) {
        slatCurAngle.value.int_value = hpSlatAngle;
        homekit_characteristic_notify(&slatCurAngle, slatCurAngle.value);
    }
}

void slatTarAngleSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP slat target angle: ");
        Serial.println(value.int_value);
    #endif

    slatTarAngle.value = value;

    #if !HP_DISCONNECTED
        hp.setVaneSetting(slatSwingMode.value.bool_value ? "SWING" : String(((90 + slatTarAngle.value.float_value) / 45) + 1).c_str());
        hp.update();
    #endif
}

void slatSwingModeSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP slat swing mode: ");
        Serial.println(value.bool_value);
    #endif

    slatSwingMode.value = value;

    #if !HP_DISCONNECTED
        hp.setVaneSetting(slatSwingMode.value.bool_value ? "SWING" : String(((90 + slatTarAngle.value.float_value) / 45) + 1).c_str());
        hp.update();
    #endif
}

void initHeatPumpSlatsAccessory() {
    slatTarAngle.setter = slatTarAngleSetter;
    slatSwingMode.setter = slatSwingModeSetter;
}