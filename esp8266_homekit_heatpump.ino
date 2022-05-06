#include "common.h"
#include <arduino_homekit_server.h> // need to disable logging in homekit_debug.h
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <HeatPump.h>
#include "heatPumpAccessory.h"
#include "heatPumpFanAccessory.h"
#include "heatPumpSlatsAccessory.h"

extern "C" homekit_server_config_t config;

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
    int diff = max(nowMs, lastBlinkMs) - min(nowMs, lastBlinkMs);
    if (diff >= statusPatternRate[boardStatus]) {
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

    #if !HP_CONNECTED
        Serial.begin(115200);
    #endif

    HK_INFO_LINE("Connecting to WiFi.");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
    }
    HK_INFO_LINE("WiFi connected.");
    boardStatus = STATUS_NO_OTA;

    sl_printCloud(SHEETS_URL, HP_SERIALNUM, "Rebooting...");

    HK_INFO_LINE("Setting up OTA updates.");
    ArduinoOTA.setHostname(HP_UNIQUE_NAME);
    ArduinoOTA.setPassword(OTA_PASS);
    ArduinoOTA.setRebootOnSuccess(true);
    ArduinoOTA.onStart([](){ 
        boardStatus = STATUS_OTA_PROGRESS;
        HK_INFO_LINE("Starting OTA Update");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) { handleStatus(); });
    ArduinoOTA.onError([](ota_error_t error) { 
        boardStatus = STATUS_ERROR;
        HK_ERROR_LINE("OTA error[%u]", error);
        if (error == OTA_AUTH_ERROR) HK_ERROR_LINE("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) HK_ERROR_LINE("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) HK_ERROR_LINE("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) HK_ERROR_LINE("Receive Failed");
        else if (error == OTA_END_ERROR) HK_ERROR_LINE("End Failed");
    });
    ArduinoOTA.begin();
    
    HK_INFO_LINE("Connecting to Homekit.");
    boardStatus = STATUS_NO_HOMEKIT;
    initHeatPumpAccessory();
    initHeatPumpFanAccessory();
    initHeatPumpSlatsAccessory();
    arduino_homekit_setup(&config);
    HK_INFO_LINE("Homekit connected.");
    boardStatus = STATUS_NO_HEAT_PUMP;
    
    #if HP_CONNECTED
        HK_INFO_LINE("Connecting to heatpump.");
        if (hp.connect(&Serial)) {
            hp.enableExternalUpdate(); // implies autoUpdate as well
            hp.setSettingsChangedCallback(heatPumpTellHomekitWhatsUp);
            hp.setStatusChangedCallback([](heatpumpStatus status){ heatPumpTellHomekitWhatsUp(); });
            
            boardStatus = STATUS_OK;
            HK_INFO_LINE("Connected to heatpump.");
        } else {
            boardStatus = STATUS_NO_HEAT_PUMP;
            HK_ERROR_LINE("Error connecting to heatpump.");
        }
    #endif
}

void loop()
{
    // Handle OTA 
    if (boardStatus != STATUS_NO_OTA) ArduinoOTA.handle();
    
    // Status blinks
    handleStatus();

    // Handle WiFi
    if (boardStatus == STATUS_NO_WIFI) return;

    // Handle Homekit
    if (boardStatus != STATUS_NO_HOMEKIT) arduino_homekit_loop();

    // Handle heat pump
    if (boardStatus != STATUS_NO_HEAT_PUMP) hp.sync();
}