#include "common.h"

// struct HeatPumpSlatsAccessory : Service::Slat
// {
//     SpanCharacteristic *curState;
//     SpanCharacteristic *type;
//     SpanCharacteristic *swingMode;
//     SpanCharacteristic *curAngle;
//     SpanCharacteristic *tarAngle;
//     HeatPump *hp;

//     HeatPumpSlatsAccessory(HeatPump *inHp) : Service::Slat()
//     { 
//         hp = inHp;
//         curState = new Characteristic::CurrentSlatState(); // 0 fixed, 1 jammed, 2 swinging
//         type = new Characteristic::SlatType(1);            // 0 horizontal, 1 vertical
//         swingMode = new Characteristic::SwingMode();       // 0 disabled, 1 enabled
//         curAngle = new Characteristic::CurrentTiltAngle(); // -90 to 90 where -90 is straight out and 90 is straight down
//         tarAngle = new Characteristic::TargetTiltAngle();
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

//     bool updateHeatPumpState() {
//         const char *slatSetting;
        
//         // Angle
//         slatSetting = String(((90 + tarAngle->getNewVal()) / 45) + 1).c_str();

//         // Swing
//         if (swingMode->getNewVal())
//             slatSetting = "SWING";

//         hp->setVaneSetting(slatSetting);
//         return hp->update();
//     }

//     void updateHomekitState() {
//         /* Air direction (vertical): 1-5, SWING, or AUTO */
//         const char *slatMode = hp->getVaneSetting();
//         bool slatSwing = slatMode == "SWING";
//         bool slatAuto = slatMode == "AUTO";
//         int slatState = slatSwing ? 2 : 0;
//         int slatAngle = 0;

//         if (slatAuto) {
//             const char *hpMode = hp->getModeSetting();
//             if (hpMode == "HEAT")
//                 slatAngle = 90;
//             else if (hpMode == "COOL" || hpMode == "DRY")
//                 slatAngle = -90;
//         } else if (!slatSwing)
//             slatAngle = -90 + ((atoi(slatMode) - 1) * 45);

//         // State
//         if (curState->getVal() != slatState)
//             curState->setVal(slatState);

//         // Swing
//         if (swingMode->getVal() != slatSwing)
//             swingMode->setVal(slatSwing);

//         // Angle
//         if (curAngle->getVal() != slatAngle)
//             curAngle->setVal(slatAngle);
//     }

//     void printDiagnostic() {
//         Serial.println();

//         Serial.print("swingMode: ");
//         Serial.println(swingMode->getNewVal() ? "On" : "Off");

//         Serial.print("curAngle: ");
//         Serial.println(curAngle->getNewVal());

//         Serial.print("tarAngle: ");
//         Serial.println(tarAngle->getNewVal());
//     }
// };