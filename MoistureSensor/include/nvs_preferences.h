#ifndef __PREFERENCES_H__
#define __PREFERENCES_H__

#include <stdint.h>
// Getters for the preferences, must be in sync with the config/sensor_config.csv and config/sensor_data.csv

const char *getSSID();
const char *getPassword();
const char *getNtpServer();
const char *getMqttHost();
const uint16_t getMqttPort();
const uint16_t getTimeToSleep();
const uint16_t getTimeToComm();
const uint16_t getUpdatePeriod();
const uint16_t getCMeasures();
const uint16_t getAirValue();
const uint16_t getWaterValue();
const uint16_t getBattFullAbs();
const uint16_t getBattChargeAbs();
const uint16_t getBattEmptyAbs();
const uint16_t getBattCorrValue();
const uint16_t getSensorPin();
const uint16_t getBatteryPin();
const char *getMqttClientId();
const char *getMqttStatTopic();

void readPreferences();

#endif