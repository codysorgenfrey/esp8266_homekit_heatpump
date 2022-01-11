#ifndef __COMMON_H__
#define __COMMON_H__

#include "secrets.h"
#include <homekit/characteristics.h>

// Board stuff
#define STATUS_LED BUILTIN_LED
#define PWR_LED 16
#define LED_ON LOW
#define LED_OFF HIGH

// OTA Stuff
#define SKETCH_VER "0.0.1"

// Heat pump stuff
#define HP_DISCONNECTED false
#define HP_MANUFACTURER "Mitsubishi-Electric"
#define HP_NAME "Split-System-Heat-Pump"
#define HP_MODEL "MSZ-GL12NA"
#define HP_SERIALNUM "88N11458"
#define HP_UNIQUE_NAME HP_NAME "-" HP_SERIALNUM
#define HP_DEFAULT_HEAT_THRESH 18.8889 // 66 Deg F
#define HP_DEFAULT_COOL_THRESH 23.3333 // 74 Deg F

// Homekit stuff
#define HK_DEBUG false
#define HK_UPDATE_TIMER 15 // Update homekit every X seconds
#if !HK_DEBUG
    #define HOMEKIT_LOG_LEVEL HOMEKIT_NO_LOG // turn off logs for Heat Pumps
#endif

// State handling
#define STATUS_ERROR 0
#define STATUS_OK 1
#define STATUS_NO_WIFI 2
#define STATUS_NO_HEAT_PUMP 3
#define STATUS_OTA_PROGRESS 4
#define STATUS_NO_HOMEKIT 5
#define STATUS_NO_OTA 6

#endif