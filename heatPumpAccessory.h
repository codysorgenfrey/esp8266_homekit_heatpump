#include "common.h"
#include <HeatPump.h>

#ifndef __HEATPUMPACCESSORY_H__
#define __HEATPUMPACCESSORY_H__

HeatPump hp;

extern "C" homekit_characteristic_t active;
extern "C" homekit_characteristic_t roomTemp;
extern "C" homekit_characteristic_t curState;             // 0 inactive, 1 idle, 2 heating, 3 cooling
extern "C" homekit_characteristic_t tarState;             // 0 auto, 1 heating, 2 cooling, 3 off
extern "C" homekit_characteristic_t coolingThresholdTemp; // 10-35
extern "C" homekit_characteristic_t heatingThresholdTemp; // 0-25

const char *hpModes[4] = {
    "AUTO",
    "HEAT",
    "COOL",
    "OFF"
};

double getMedianTemp() {
    return ((heatingThresholdTemp.value.float_value - coolingThresholdTemp.value.float_value) / 2) + coolingThresholdTemp.value.float_value;
}

double getTempFromState() {
    double hkTemp;
    switch (tarState.value.int_value)
    {
    case 1: // Heating
        hkTemp = heatingThresholdTemp.value.float_value;
        break;

    case 2: // Cooling
        hkTemp = coolingThresholdTemp.value.float_value;
        break;
    
    default: // Auto and Off
        hkTemp = getMedianTemp(); // set temp in the middle
        break;
    }
    
    /* Temperature Between 16 and 31 for Heat Pump */
    hkTemp = max(hkTemp, 16.0);
    hkTemp = min(hkTemp, 31.0);
    
    return hkTemp;
}

void heatPumpAccessorySettingsChanged() {
    //////////////////////////////////////////////////////////////////
    // Get variables from Heat Pump & Homekit                       //
    //////////////////////////////////////////////////////////////////
    
    double hpTemp = hp.getTemperature();
    double hpRoomTemp = hp.getRoomTemperature();
    const char *hpMode = hp.getModeSetting();

    int hpTarState;
    for (hpTarState = 0; hpTarState < 3; hpTarState += 1) { // has to be 1 less then count since we use hpTarState later
        if (hpModes[hpTarState] == hpMode)
            break;
    }
    if (hpTarState == 3) hpTarState = 0; // Unsupported modes, setting Homekit to auto
    
    int hpCurState;
    double hkTemp;
    switch (hpTarState)
    {
    case 1: // Heating
        hkTemp = heatingThresholdTemp.value.float_value;
        hpCurState = 2;
        break;

    case 2: // Cooling
        hkTemp = coolingThresholdTemp.value.float_value;
        hpCurState = 3;
        break;
    
    default: // Auto and Off
        hkTemp = getMedianTemp(); // set temp in the middle
        hpCurState = hkTemp >= hpRoomTemp ? 2 : 3;
        break;
    }
    
    if (!hp.getOperating() && !(hpCurState == 0)) // Idle
        hpCurState = 1;

    //////////////////////////////////////////////////////////////////
    // Update homekit if things don't match                         //
    //////////////////////////////////////////////////////////////////

    // Active
    if (active.value.bool_value != hp.getPowerSettingBool()) {
        active.value.bool_value = hp.getPowerSettingBool();
        homekit_characteristic_notify(&active, active.value);
    }

    // Mode
    if (hpModes[hpTarState] != hp.getModeSetting()) {
        tarState.value.int_value = hpTarState;
        homekit_characteristic_notify(&tarState, tarState.value);
    }

    // State
    if (hpCurState != curState.value.int_value) {
        curState.value.int_value = hpCurState;
        homekit_characteristic_notify(&curState, curState.value);
    }

    // Temperature
    if (hkTemp != hpTemp)
        switch (hpTarState)
        {
        case 1: // Heating
            heatingThresholdTemp.value.float_value = hpTemp;
            homekit_characteristic_notify(&heatingThresholdTemp, heatingThresholdTemp.value);
            break;

        case 2: // Cooling
            coolingThresholdTemp.value.float_value = hpTemp;
            homekit_characteristic_notify(&coolingThresholdTemp, coolingThresholdTemp.value);
            break;
        
        default: // Auto and Off
            double diff = hpTemp - hkTemp;
            coolingThresholdTemp.value.float_value = (coolingThresholdTemp.value.float_value + diff);
            homekit_characteristic_notify(&coolingThresholdTemp, coolingThresholdTemp.value);
            heatingThresholdTemp.value.float_value = (heatingThresholdTemp.value.float_value + diff);
            homekit_characteristic_notify(&heatingThresholdTemp, heatingThresholdTemp.value);
            break;
        }

    // Room temperature
    if (roomTemp.value.float_value != hpRoomTemp) {
        roomTemp.value.float_value = hpRoomTemp;
        homekit_characteristic_notify(&roomTemp, roomTemp.value);
    }
}

void activeSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP active: ");
        Serial.println(value.bool_value ? "ON" : "OFF");
    #endif

    active.value = value;

    #if !HP_DISCONNECTED
        hp.setPowerSetting(active.value.bool_value ? "ON" : "OFF");
        hp.update();
    #endif
}

void tarStateSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP mode: ");
        Serial.println(hpModes[value.int_value]);
    #endif

    tarState.value = value;
    
    #if !HP_DISCONNECTED
        hp.setModeSetting(hpModes[tarState.value.int_value]);
        hp.update();
    #endif
}

void coolingTempSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP cooling temp: ");
        Serial.println(value.float_value);
    #endif

    coolingThresholdTemp.value = value;

    #if !HP_DISCONNECTED
        hp.setTemperature(getTempFromState());
        hp.update();
    #endif
}

void heatingTempSetter(homekit_value_t value) {
    #if HK_DEBUG
        Serial.print("Setting HP heating temp: ");
        Serial.println(value.float_value);
    #endif

    heatingThresholdTemp.value = value;
    
    #if !HP_DISCONNECTED
        hp.setTemperature(getTempFromState());
        hp.update();
    #endif
}

void initHeatPumpAccessory() {
    // Get data from Homekit
    active.setter = activeSetter;
    tarState.setter = tarStateSetter;
    coolingThresholdTemp.setter = coolingTempSetter;
    heatingThresholdTemp.setter = heatingTempSetter;
}

#endif  