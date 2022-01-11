#include <arduino_homekit_server.h>
#include <WiFiManager.h>
#include <ArduinoOTA.h>
#include "common.h"
#include <HeatPump.h>
#include "heatPumpAccessory.h"
#include "heatPumpFanAccessory.h"
#include "heatPumpSlatsAccessory.h"

extern "C" homekit_server_config_t config;

WiFiManager wm;

unsigned long int boardStatus = STATUS_NO_WIFI;

int statusBlinkPattern[7] = {
    10, // Error
    0,  // ok
    1,  // no wifi
    2,  // No OTA
    3,  // No Homkit
    4,  // no heat pump
    3   // OTA progress
};
int statusPatternRate[7] = {
    2000, // error
    1000, // ok
    1000, // no wifi
    1000, // No OTA
    1000, // No homekit
    1000, // no heat pump
    2000  // OTA progress
};
unsigned long lastBlinkMs = 0;

void handleStatus() {
    unsigned long nowMs = millis();
    if ((nowMs - lastBlinkMs) >= statusPatternRate[boardStatus]) {
        if (boardStatus == STATUS_OK && digitalRead(STATUS_LED) == LED_ON) {
            digitalWrite(STATUS_LED, LED_OFF);
        } else {
            for (int x = 0; x < statusBlinkPattern[boardStatus]; x++) {
                digitalWrite(STATUS_LED, LED_ON);
                delay(99);
                digitalWrite(STATUS_LED, LED_OFF);
                delay(99);
            }
        }

        lastBlinkMs = nowMs;
    }
}

void heatPumpTellHomekitWhatsUp() {
    heatPumpAccessorySettingsChanged();
    heatPumpFanAccessorySettingsChanged();
    heatPumpSlatsAccessorySettingsChanged();
}

void setup()
{
    // Setup output LED
    pinMode(STATUS_LED, OUTPUT); // set up status LED
    pinMode(PWR_LED, OUTPUT);    // set up power LED
    digitalWrite(STATUS_LED, LED_OFF);
    digitalWrite(PWR_LED, LED_ON);

    #if HK_DEBUG
        Serial.begin(115200);
    #endif

    if (boardStatus == STATUS_NO_WIFI) {
        // Connect to wifi
        wm.setDebugOutput(HK_DEBUG);
        wm.setConfigPortalBlocking(false);
        wm.setSaveConfigCallback([](){ 
            boardStatus = STATUS_NO_OTA;
            setup();
        });

        boardStatus = wm.autoConnect(HP_UNIQUE_NAME) ? STATUS_NO_OTA : STATUS_NO_WIFI;
    }
    
    if (boardStatus == STATUS_NO_OTA) {
        // Connect OTA
        ArduinoOTA.setHostname(HP_UNIQUE_NAME);
        ArduinoOTA.setPassword(OTA_PASS);
        ArduinoOTA.setRebootOnSuccess(true);
        ArduinoOTA.onStart([](){ boardStatus = STATUS_OTA_PROGRESS; });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { handleStatus(); });
        ArduinoOTA.onError([](ota_error_t error) { boardStatus = STATUS_ERROR; });
        ArduinoOTA.begin();
        
        // Connect to Homekit
        boardStatus = STATUS_NO_HOMEKIT;
        initHeatPumpAccessory();
        initHeatPumpFanAccessory();
        initHeatPumpSlatsAccessory();
        arduino_homekit_setup(&config);
        
        // Connect to heat pump
        boardStatus = STATUS_NO_HEAT_PUMP;
        #if !HP_DISCONNECTED
            boardStatus = hp.connect(&Serial) ? STATUS_OK : STATUS_NO_HEAT_PUMP;
            hp.enableExternalUpdate();
            hp.setSettingsChangedCallback(heatPumpTellHomekitWhatsUp);
            hp.setStatusChangedCallback([](heatpumpStatus status){ heatPumpTellHomekitWhatsUp(); });
            hp.setOnConnectCallback(heatPumpTellHomekitWhatsUp);
        #else
            boardStatus = STATUS_OK;
        #endif
    }
}

void loop()
{
    // Status blinks
    handleStatus();

    // Handle WiFi
    if (boardStatus == STATUS_NO_WIFI) {
        wm.process();
        return;
    }

    // Handle OTA 
    if (boardStatus != STATUS_NO_OTA) ArduinoOTA.handle();

    // Handle Homekit
    if (boardStatus != STATUS_NO_HOMEKIT) arduino_homekit_loop();

    // Handle heat pump
    #if !HP_DISCONNECTED
        if (boardStatus != STATUS_NO_HEAT_PUMP) hp.sync();
    #endif
}