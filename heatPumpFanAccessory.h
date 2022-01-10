#include "common.h"

// struct HeatPumpFanAccessory : Service::Fan
// {
//     SpanCharacteristic *active;
//     SpanCharacteristic *speed;
//     SpanCharacteristic *curState;
//     SpanCharacteristic *fanAuto;
//     HeatPump *hp;

//     HeatPumpFanAccessory(HeatPump *inHp) : Service::Fan()
//     { 
//         hp = inHp;
//         active = new Characteristic::Active(1);
//         speed = new Characteristic::RotationSpeed();      // 0-100
//         curState = new Characteristic::CurrentFanState(); // 0 inactive, 1 idle, 2 blowing air
//         fanAuto = new Characteristic::TargetFanState();  // 0 manual, 1 auto
//     }

//     boolean update()
//     { 
//         #if DEBUG_HOMEKIT
//             printDiagnostic();
//         #endif
        
//         #if TESTING_HP
//             updateHeatPumpState();
//             return true;
//         #else
//             return updateHeatPumpState();
//         #endif
//     }

//     void loop() {
//         #if !TESTING_HP
//             if (millis() % (HK_UPDATE_TIMER * 1000) == 0)
//                 updateHomekitState();
//         #endif
//     }

//     void updateHomekitState() {
//         bool hpFanState = hp->getOperating() ? 2 : 1;
//         const char *hpFanMode = hp->getFanSpeed();
//         bool hpFanAuto = hpFanMode == "AUTO";
//         bool hpFanActive = hpFanMode != "QUIET";

//         double hpFanSpeed = 0.0;
//         if (!hpFanAuto && hpFanActive) {
//             char *end;
//             hpFanSpeed = strtod(hpFanMode, &end) * 25.0; 
//         }

//         // Active
//         if (active->getVal() != hpFanActive) // Fan is always on
//             active->setVal(hpFanActive);

//         // State
//         if (curState->getVal() != hpFanState)
//             curState->setVal(hpFanState);

//         // Auto
//         if (fanAuto->getVal() != hpFanAuto)
//             fanAuto->setVal(hpFanAuto);

//         // Speed
//         if (speed->getVal() != hpFanSpeed)
//             speed->setVal(hpFanSpeed);
        
//     }

//     bool updateHeatPumpState() {
//         /* Fan speed: 1-4, AUTO, or QUIET */
//         const char *fanSpeed;
//         int speed0_4 = roundFanSpeed(speed->getNewVal());
//         fanSpeed = speed0_4 != 0 ? String(speed0_4).c_str() : "QUIET";

//         if (fanAuto->getNewVal())
//             fanSpeed = "AUTO";

//         if (!active->getNewVal()) // order matters here, active trumps auto
//             fanSpeed = "QUIET";

//         hp->setFanSpeed(fanSpeed);
//         return hp->update();
//     }

//     int roundFanSpeed(double speed) {
//         return floor((speed / 25) + 0.5);
//     }

//     void printDiagnostic() {
//         Serial.println();

//         Serial.print("Active: ");
//         Serial.println(active->getNewVal() ? "On" : "Off");

//         Serial.print("Speed: ");
//         Serial.println(speed->getNewVal());

//         Serial.print("TarState: ");
//         Serial.println(fanAuto->getNewVal() ? "Manual" : "Auto");
//     }
// };