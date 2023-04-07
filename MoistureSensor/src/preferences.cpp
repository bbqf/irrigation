#include <Arduino.h>
#include <Preferences.h>

struct {
    String ssid;
    String password;
    String ntpServer;
    String mqttHost;
    uint16_t mqttPort = -1;
    uint16_t timeToSleep = -1;
    uint16_t timeToComm = -1;
    uint16_t updatePeriod = -1;
    uint16_t cMeasures = -1;
    uint16_t airValue = -1;
    uint16_t waterValue = -1;
    uint16_t battFullAbs = -1;
    uint16_t battChargeAbs = -1;
    uint16_t battEmptyAbs = -1;
    uint16_t battCorrValue = -1;
    uint16_t sensorPin = -1;
    uint16_t batteryPin = -1;
    String mqttClientId;
    String mqttStatTopic;
} prefs;

const char *getSSID() { return prefs.ssid.c_str(); };
const char *getPassword() { return prefs.password.c_str(); };
const char *getNtpServer() { return prefs.ntpServer.c_str(); };
const char *getMqttHost() { return prefs.mqttHost.c_str(); };
const uint16_t getMqttPort() { return prefs.mqttPort; };
const uint16_t getTimeToSleep() { return prefs.timeToSleep; };
const uint16_t getTimeToComm() { return prefs.timeToComm; };
const uint16_t getUpdatePeriod() { return prefs.updatePeriod; };
const uint16_t getCMeasures() { return prefs.cMeasures; };
const uint16_t getAirValue() { return prefs.airValue; };
const uint16_t getWaterValue() { return prefs.waterValue; };
const uint16_t getBattFullAbs() { return prefs.battFullAbs; };
const uint16_t getBattChargeAbs() { return prefs.battChargeAbs; };
const uint16_t getBattEmptyAbs() { return prefs.battEmptyAbs; };
const uint16_t getBattCorrValue() { return prefs.battCorrValue; };
const uint16_t getSensorPin() { return prefs.sensorPin; };
const uint16_t getBatteryPin() { return prefs.batteryPin; };
const char *getMqttClientId() { return prefs.mqttClientId.c_str(); };
const char *getMqttStatTopic() { return prefs.mqttStatTopic.c_str(); };

// Global preferences, same for all devices on a network
void readIOT()
{
    Preferences preferences;
    if (preferences.begin("iot")) {

        prefs.ssid = preferences.getString("ssid");
        prefs.password = preferences.getString("password");
        prefs.ntpServer = preferences.getString("ntpServer");
        prefs.mqttHost = preferences.getString("mqttHost");
        prefs.mqttPort = preferences.getUShort("mqttPort");
        prefs.timeToSleep = preferences.getUShort("timeToSleep");
        prefs.timeToComm = preferences.getUShort("timeToComm");
        prefs.updatePeriod = preferences.getUShort("updatePeriod");

        preferences.end();
    } else {
        log_e("Cannot read preferences from the namespace iot");
    }
    log_d("Read preferences:");
    log_d("ssid=%s", getSSID());
    log_d("password=%s", getPassword());
    log_d("ntpServer=%s", getNtpServer());
    log_d("mqttHost=%s", getMqttHost());
    log_d("mqttPort=%d", getMqttPort());
    log_d("timeToSleep=%d", getTimeToSleep());
    log_d("timeToComm=%d", getTimeToComm());
    log_d("updatePeriod=%d", getUpdatePeriod());
}

// Device specific preferences
void readSensor() {
    Preferences preferences;

    if (preferences.begin("moistureSensor")) {

        prefs.cMeasures = preferences.getUShort("cMeasures");
        prefs.airValue = preferences.getUShort("airValue");
        prefs.waterValue = preferences.getUShort("waterValue");
        prefs.battFullAbs = preferences.getUShort("battFullAbs");
        prefs.battChargeAbs = preferences.getUShort("battChargeAbs");
        prefs.battEmptyAbs = preferences.getUShort("battEmptyAbs");
        prefs.battCorrValue = preferences.getUShort("battCorrValue");
        prefs.sensorPin = preferences.getUShort("sensorPin");
        prefs.batteryPin = preferences.getUShort("batteryPin");
        prefs.mqttClientId = preferences.getString("mqttClientId");;
        prefs.mqttStatTopic = preferences.getString("mqttStatTopic");;

        preferences.end();
    } else {
        log_e("Cannot read preferences from the namespace moistureSensor");
    }
    log_d("Read preferences:");
    log_d("cMeasures=%d", getCMeasures());
    log_d("airValue=%d", getAirValue());
    log_d("waterValue=%d", getWaterValue());
    log_d("battFullAbs=%d", getBattFullAbs());
    log_d("battChargeAbs=%d", getBattChargeAbs());
    log_d("battEmptyAbs=%d", getBattEmptyAbs());
    log_d("battCorrValue=%d", getBattCorrValue());
    log_d("sensorPin=%d", getSensorPin());
    log_d("batteryPin=%d", getBatteryPin());
    log_d("mqttClientId=%s", getMqttClientId());
    log_d("mqttStatTopic=%s", getMqttStatTopic());
}

void readPreferences() {

    readIOT();
    readSensor();
    
}
