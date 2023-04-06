#include <Arduino.h>

#include <ArduinoJson.h>

#include <Preferences.h>

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
#define REV(x) #x

const int cMeasurements = 5; // count of measurements to take to reduce the noise

const int AirValue = 2760;   //you need to replace this value with Value_1
const int WaterValue = 900;  //you need to replace this value with Value_2

const int BatteryFullValue = 2310;  // Abs reading of full battery corresponds to 4.02v
const int BatteryChargingValue = 2250;  // Abs reading of the battery when charging (from empty) starts, corresponds to 3.9v
const int BatteryEmptyValue = 2000;  // Abs reading of an empty battery = shutdown now! Corresponds to 3.4v

const int BatteryCorrectionValue = 297; // Correction for Abs to V conversion - might be dependent on the hardware

typedef  enum  {
  OP_EMPTY,
  OP_CHARGING,
  OP_DISCHARGING,
  OP_FULL
} OpStatus;

int initialBatteryReading = 0;

Preferences preferences;

#include <stdio.h>

// Functions does cMeausrements measures on the specified pin and returns the average cutting off max and min values
uint16_t readAverage(int pin) {
    uint16_t arr[cMeasurements] = {0};
    uint16_t i, sum = 0, max = 0, min = -1, count = 0;
    float avg;

    // Find maximum and minimum values in the array
    for (i = 0; i < cMeasurements; i++) {
        arr[i] = analogRead(pin);
        if (arr[i] > max) {
            max = arr[i];
        }
        if (arr[i] < min) {
            min = arr[i];
        }
        delay(10);
    }

    log_i("Max: %d, Min: %d", max, min);
    // Calculate the sum of the array excluding the maximum and minimum values
    for (i = 0; i < cMeasurements; i++) {
        if (arr[i] != max && arr[i] != min) {
            sum += arr[i];
            count++;
        }
    }
    log_i("Count: %d", count);
    if (count==0) {
      count=1;
      sum=arr[0];
    }
    // Calculate the average
    avg = (float) sum / count;

    return (uint16_t) avg;
}

float convertBatteryAbsToV(int batteryAbs) {
  return ((float)(batteryAbs - BatteryCorrectionValue)*2/1000);
}

int convertBatteryRel(int val) {
  int battPct = map(val, BatteryEmptyValue, BatteryFullValue, 0, 100);
  if (battPct < 0) {
    battPct = 0;
  } else if (battPct > 100) {
    battPct = 100;
  }
  return(battPct);
}

int readBattAbs() {
  int battValue = 0;
 
  battValue = readAverage(BATT_PIN);  
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
  } else if (batt >= BatteryChargingValue) {
    status = OP_CHARGING;
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
 
  soilMoistureValue = readAverage(SENSOR_PIN);

  Serial.printf("MoistureAbs: %d\n", soilMoistureValue);
  return soilMoistureValue;
}

void sendStatus() {
  log_e("Enter");
  const size_t capacity = JSON_OBJECT_SIZE(10) + 128;
  
  DynamicJsonDocument doc(capacity);

  lastStatusSent = getEpochTime();
  doc["time"] = lastStatusSent;
  doc["temperature"] = String(temperatureRead(), 1);
  int moisture = readSensorAbs();
  doc["moistureAbs"] = moisture;
  doc["moisture"] = convertSensorRel(moisture);
  int batt = readBattAbs();
#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_DEBUG
    doc["batteryAbs"] = batt;
    doc["initialBatteryAbs"] = initialBatteryReading;
#endif
  doc["batteryPct"] = convertBatteryRel(batt);
  doc["batteryV"] = String(convertBatteryAbsToV(batt), 2);

  doc["status"] = convertOpStatusToString(getOperationalStatus());
  doc["version"] = REV(REVISION);

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
  initialBatteryReading = readBattAbs();

  Serial.begin(115200);
  log_d("Enter");
  // put your setup code here, to run once:

  preferences.begin("iot", true);
  

#if ARDUHAL_LOG_LEVEL < ARDUHAL_LOG_LEVEL_DEBUG
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