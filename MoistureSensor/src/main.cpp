#include <Arduino.h>

#include <ArduinoJson.h>

#include "wifi_ops.h"
#include "mqtt.h"

#include "localConfig.h"

#include "globals.h"

#define SENSOR_PIN 35
#define BATT_PIN 34

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 1800       /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_COMM 3
#define UPDATE_EVERY 60 // How often to send updates in seconds

const int AirValue = 1800;   //you need to replace this value with Value_1
const int WaterValue = 850;  //you need to replace this value with Value_2

const int BatteryFullValue = 2350;  // Abs reading of full battery
const int BatteryChargingValue = 2110;  // Abs reading of the battery when charging (from empty) starts
const int BatteryEmptyValue = 1900;  // Abs reading of an empty battery = shutdown now!

const int BatteryCorrectionValue = 297; // Correction for Abs to V conversion - might be dependent on the hardware

typedef  enum  {
  OP_EMPTY,
  OP_CHARGING,
  OP_DISCHARGING,
  OP_FULL
} OpStatus;

float convertBatteryAbsToV(int batteryAbs) {
  return ((float)(batteryAbs - BatteryCorrectionValue)*2/1000);
}

time_t lastStatusSent;

int convertSensorRel(int val) {
  int mapped = map(val, AirValue, WaterValue, 0, 100);
  if (mapped < 0) {
    mapped = 0;
  } else if (mapped > 100) {
    mapped = 100;
  }
  return(mapped);
}

int readSensorAbs() {
  int soilMoistureValue = 0;
 
  soilMoistureValue = analogRead(SENSOR_PIN);  //put Sensor insert into soil
  Serial.printf("MoistureAbs: %d\n", soilMoistureValue);
  return soilMoistureValue;
}

int convertBatteryRel(int val) {
  return(map(val, BatteryEmptyValue, BatteryFullValue, 0, 100));
}

int readBattAbs() {
  int battValue = 0;
 
  battValue = analogRead(BATT_PIN);  
  Serial.printf("BatteryAbs: %d\n",battValue);
  return battValue;
}

const OpStatus getOperationalStatus() {
  int batt = readBattAbs();
  OpStatus status = OP_DISCHARGING;
  if (batt <= BatteryEmptyValue) {
    status = OP_EMPTY;
  } else if (batt >= BatteryFullValue) {
    status = OP_FULL;
  }

  return status;
}

const char * convertOpStatusToString(OpStatus status) {

  switch (status) {
    case OP_EMPTY: return("Empty"); break;
    case OP_CHARGING: return("Charging"); break;
    case OP_DISCHARGING: return("Discharging"); break;
    case OP_FULL: return("Full"); break;
  }

  return("Unknown");
}

void sendStatus() {
  log_e("Enter");
  const size_t capacity = JSON_OBJECT_SIZE(9) + 128;
  
  DynamicJsonDocument doc(capacity);

  lastStatusSent = getEpochTime();
  doc["time"] = lastStatusSent;
  doc["temperature"] = String(temperatureRead(), 1);
  int moisture = readSensorAbs();
  doc["moistureAbs"] = moisture;
  doc["moisture"] = convertSensorRel(moisture);
  int batt = readBattAbs();
  // doc["batteryAbs"] = batt;
  doc["batteryPct"] = convertBatteryRel(batt);
  doc["batteryV"] = String(convertBatteryAbsToV(batt), 2);

  doc["status"] = convertOpStatusToString(getOperationalStatus());

  uint16_t ret = publish(doc.as<String>().c_str());
  log_d("publish() returned: %d", ret);
  if (ret == 0) {
    allowSleep();
  }
  log_e("Exit");
}

const char * isSleepAllowedStr() {
  return(isSleepAllowed() ? "true" : "false");
}

void setup()
{
  Serial.begin(115200);
  log_d("Enter");
  // put your setup code here, to run once:

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
  // In Debug mode we assume the battery is not connected (while USB is) and disable sleep on enpty battery
  if (getOperationalStatus() == OP_EMPTY) {
    log_e("Battery empty! Deep Sleep for %d seconds", TIME_TO_SLEEP);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
  }
#endif

  log_d("Starting main setup");
  preventSleep();
  
  setupWiFi(setupMQTT);
  log_d("Setups done.");

  log_d("Exit");
}

void loop()
{
  log_d("Enter. sleepAllowed: %s", isSleepAllowedStr());
 
  if ((lastStatusSent == 0 || lastStatusSent + UPDATE_EVERY <= getEpochTime())) {
    sendStatus();
  }
  
  if (isSleepAllowed()) {
    log_i("Sleeping for %d seconds to complete async communication", TIME_TO_COMM);
    delay(TIME_TO_COMM * 1000);
    
    log_i("Deep Sleep for %d seconds", TIME_TO_SLEEP);
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    
  }

  delay(500); // We don't want to burn too many CPU cycles, so sleep for 1/2 sec if not allowed to deep sleep
  log_d("Exit");
}