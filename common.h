#ifndef __COMMON_H__
#define __COMMON_H__

#include <Arduino.h>
#include <SheetsLogger.h>
#include "secrets.h"

// Board stuff
#define STATUS_LED BUILTIN_LED
#define PWR_LED 16
#define LED_ON LOW
#define LED_OFF HIGH

// Heat pump stuff
#define HP_CONNECTED false
#define HP_MANUFACTURER "Mitsubishi-Electric"
#define HP_NAME "Split-System-Heat-Pump"
#define HP_MODEL "MSZ-GL06NA"
// #define HP_SERIALNUM "88N11458" // Living room
#define HP_SERIALNUM "14E32127" // Master bed room
#define HP_UNIQUE_NAME HP_NAME "-" HP_SERIALNUM
#define HP_DEFAULT_HEAT_THRESH 18.8889 // 66 Deg F
#define HP_DEFAULT_COOL_THRESH 23.3333 // 74 Deg F

// Homekit stuff
#define SKETCH_VER "0.0.1"

// State handling
#define STATUS_ERROR 0
#define STATUS_OK 1
#define STATUS_NO_WIFI 2
#define STATUS_NO_OTA 3
#define STATUS_NO_HOMEKIT 4
#define STATUS_NO_HEAT_PUMP 5
#define STATUS_OTA_PROGRESS 6

// Logging
#define HK_DEBUG_LEVEL_NONE -1
#define HK_DEBUG_LEVEL_ERROR 0
#define HK_DEBUG_LEVEL_INFO 1

#define HK_DEBUG HK_DEBUG_LEVEL_INFO

#if HK_DEBUG >= HK_DEBUG_LEVEL_ERROR
    #if !HP_CONNECTED
        #define HK_ERROR_LINE(message, ...) sl_printf(SHEETS_URL, HP_SERIALNUM, "ERR [%7lu][%.2fkb] %s: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), HP_SERIALNUM, ##__VA_ARGS__)
    #else 
        #define HK_ERROR_LINE(message, ...) sl_printCloud(SHEETS_URL, HP_SERIALNUM, "ERR [%7lu][%.2fkb] %s: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), HP_SERIALNUM, ##__VA_ARGS__)
    #endif
#else
    #define HK_ERROR_LINE(message, ...)
#endif

#if HK_DEBUG >= HK_DEBUG_LEVEL_INFO && !HP_CONNECTED
    #define HK_INFO_LINE(message, ...) printf(">>> [%7lu][%.2fkb] %s: " message "\n", millis(), (system_get_free_heap_size() * 0.001f), HP_SERIALNUM, ##__VA_ARGS__)
#else
    #define HK_INFO_LINE(message, ...)
#endif

#endif