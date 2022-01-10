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
    2,  // no heat pump
    3,  // OTA progress
    2,  // No Homkit
    2   // No OTA
};
int statusPatternRate[7] = {
    2000, // error
    1000, // ok
    1000, // no wifi
    1000, // no heat pump
    2000, // OTA progress
    1000, // No homekit
    1000  // No OTA
};
unsigned long lastBlinkMs = 0;

void handleStatus() {
    unsigned long nowMs = millis();
    if ((nowMs - lastBlinkMs) >= statusPatternRate[boardStatus]) {
        if (boardStatus == STATUS_OK && digitalRead(LED) == LED_OFF) {
            digitalWrite(LED, LED_ON);
        } else {
            for (int x = 0; x < statusBlinkPattern[boardStatus]; x++) {
                digitalWrite(LED, LED_ON);
                delay(99);
                digitalWrite(LED, LED_OFF);
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
    pinMode(LED, OUTPUT); // set up onboard LED
    digitalWrite(LED, LED_OFF);

    #if HP_DISCONNECTED
        Serial.begin(115200);
    #endif

    switch (boardStatus)
    {
    case STATUS_NO_WIFI:
        // Connect to wifi
        wm.setDebugOutput(false);
        wm.setConfigPortalBlocking(false);
        wm.setSaveConfigCallback([](){ boardStatus = STATUS_NO_OTA; });

        boardStatus = wm.autoConnect(HP_UNIQUE_NAME, WIFI_AP_PASS) ? STATUS_NO_OTA : STATUS_NO_WIFI;
        break;

    case STATUS_NO_OTA:
        // Connect OTA
        ArduinoOTA.setHostname(HP_UNIQUE_NAME);
        ArduinoOTA.setPassword(OTA_PASS);
        ArduinoOTA.setRebootOnSuccess(true);
        ArduinoOTA.onStart([](){ boardStatus = STATUS_OTA_PROGRESS; });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { handleStatus(); });
        ArduinoOTA.onError([](ota_error_t error) { boardStatus = STATUS_ERROR; });
        ArduinoOTA.begin();

        boardStatus = STATUS_NO_HOMEKIT;
        break;

    case STATUS_NO_HOMEKIT:
        // Connect to Homekit
        initHeatPumpAccessory();
        initHeatPumpFanAccessory();
        initHeatPumpSlatsAccessory();
        arduino_homekit_setup(&config);

        boardStatus = STATUS_NO_HEAT_PUMP;
        break;

    case STATUS_NO_HEAT_PUMP:
        // Connect to heat pump
        Serial.flush();
        Serial.end();
        hp.enableExternalUpdate();
        hp.setSettingsChangedCallback(heatPumpTellHomekitWhatsUp);
        hp.setOnConnectCallback(heatPumpTellHomekitWhatsUp);
        #if !HP_DISCONNECTED
            boardStatus = hp.connect(&Serial) ? STATUS_OK : STATUS_NO_HEAT_PUMP;
        #else
            boardStatus = STATUS_OK;
        #endif

        break;

    default:
        boardStatus = STATUS_ERROR;
        Serial.println("***ERROR: UNKONWN STATUS***");
        break;
    }
}

void loop()
{
    // Status blinks
    handleStatus();

    switch (boardStatus)
    {
    case STATUS_NO_WIFI:
        wm.process();
        break;
    
    case STATUS_NO_OTA:
        setup();
        break;

    case STATUS_NO_HEAT_PUMP:
        setup();
        break;

    case STATUS_NO_HOMEKIT:
        setup();
        break;

    case STATUS_OK:
        // Handle OTA 
        ArduinoOTA.handle();

        // Handle Homekit
        arduino_homekit_loop();

        // Handle heat pump
        #if !HP_DISCONNECTED
            hp.sync();
        #endif
        break;
    
    default:
        break;
    }
}