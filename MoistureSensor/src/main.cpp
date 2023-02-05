#include <Arduino.h>

#include <ArduinoJson.h>

#include "wifi_ops.h"
#include "mqttRelay.h"

#include "localConfig.h"

#include "globals.h"

#define SENSOR_PIN 35
#define BATT_PIN 34

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_COMM 3
#define UPDATE_EVERY 10 // How often to send updates in seconds

const int AirValue = 2500;   //you need to replace this value with Value_1
const int WaterValue = 1300;  //you need to replace this value with Value_2

const int ChargingValue = 2000;  //you need to replace this value with Value_2
const int EmptyValue = 1750;  //you need to replace this value with Value_2


#ifdef __cplusplus
extern "C"
{
#endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

time_t lastStatusSent;

int convertSensorRel(int val) {
  return(map(val, AirValue, WaterValue, 0, 100));
}

int readSensorAbs() {
  int soilMoistureValue = 0;
 
  soilMoistureValue = analogRead(SENSOR_PIN);  //put Sensor insert into soil
  Serial.printf("MoistureAbs: %d\n", soilMoistureValue);
  return soilMoistureValue;
}

int convertBatteryRel(int val) {
  return(map(val, EmptyValue, ChargingValue, 0, 100));
}

int readBattAbs() {
  int battValue = 0;
 
  battValue = analogRead(BATT_PIN);  
  Serial.printf("BatteryAbs: %d\n",battValue);
  return battValue;
}


void sendStatus() {
  log_e("Enter");
  const size_t capacity = JSON_OBJECT_SIZE(7) + 128;
  
  DynamicJsonDocument doc(capacity);

  lastStatusSent = getEpochTime();
  doc["time"] = lastStatusSent;
  doc["temperature"] = (float)(temprature_sens_read() - 32) * 5 / 9;
  int moisture = readSensorAbs();
  doc["moistureAbs"] = moisture;
  doc["moisture"] = convertSensorRel(moisture);
  int batt = readBattAbs();
  doc["batteryAbs"] = batt;
  doc["battery"] = convertBatteryRel(batt);

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
  log_e("Enter");
  // put your setup code here, to run once:
  Serial.begin(115200);
  // pinMode(SENSOR_PIN, ANALOG);
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
  
  // Serial.print(readSensor());
  // Serial.println("%");

  delay(500); // We don't want to burn too many CPU cycles, so sleep for 1/2 sec if not allowed to deep sleep
  log_d("Exit");
}