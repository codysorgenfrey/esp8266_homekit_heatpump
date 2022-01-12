#include "common.h"
#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
	;/* Identify heat pump here */
}

homekit_characteristic_t active = HOMEKIT_CHARACTERISTIC_(ACTIVE, 0);
homekit_characteristic_t roomTemp = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
homekit_characteristic_t curState = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATER_COOLER_STATE, 0);
homekit_characteristic_t tarState = HOMEKIT_CHARACTERISTIC_(TARGET_HEATER_COOLER_STATE, 0);
homekit_characteristic_t coolingThresholdTemp = HOMEKIT_CHARACTERISTIC_(COOLING_THRESHOLD_TEMPERATURE, HP_DEFAULT_COOL_THRESH);
homekit_characteristic_t heatingThresholdTemp = HOMEKIT_CHARACTERISTIC_(HEATING_THRESHOLD_TEMPERATURE, HP_DEFAULT_HEAT_THRESH);

homekit_characteristic_t fanActive = HOMEKIT_CHARACTERISTIC_(ACTIVE, 0);
homekit_characteristic_t fanState = HOMEKIT_CHARACTERISTIC_(CURRENT_FAN_STATE, 0);
homekit_characteristic_t fanAuto = HOMEKIT_CHARACTERISTIC_(TARGET_FAN_STATE, 0);
homekit_characteristic_t fanSpeed = HOMEKIT_CHARACTERISTIC_(ROTATION_SPEED, 0);

homekit_characteristic_t slatType = HOMEKIT_CHARACTERISTIC_(SLAT_TYPE, 1);
homekit_characteristic_t slatState = HOMEKIT_CHARACTERISTIC_(CURRENT_SLAT_STATE, 0);
homekit_characteristic_t slatCurAngle = HOMEKIT_CHARACTERISTIC_(CURRENT_TILT_ANGLE, 0);
homekit_characteristic_t slatTarAngle = HOMEKIT_CHARACTERISTIC_(TARGET_TILT_ANGLE, 0);
homekit_characteristic_t slatSwingMode = HOMEKIT_CHARACTERISTIC_(SWING_MODE, 0);

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_air_conditioner, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, HP_UNIQUE_NAME),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, HP_MANUFACTURER),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, HP_SERIALNUM),
            HOMEKIT_CHARACTERISTIC(MODEL, HP_MODEL),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, SKETCH_VER),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(HEATER_COOLER, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            &active,
            &roomTemp,
            &curState,
            &tarState,
            &coolingThresholdTemp,
            &heatingThresholdTemp,
            NULL
        }),
        HOMEKIT_SERVICE(FAN2, .primary=false, .characteristics=(homekit_characteristic_t*[]) {
            &fanActive,
            &fanState,
            &fanAuto,
            &fanSpeed,
            NULL
        }),
        HOMEKIT_SERVICE(SLAT, .primary=false, .characteristics=(homekit_characteristic_t*[]) {
            &slatType,
            &slatState,
            &slatCurAngle,
            &slatTarAngle,
            &slatSwingMode,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
		.accessories = accessories,
		.password = HK_PASSWORD
};