#include <Arduino.h>

#include <ArduinoJson.h>

#include "wifi_ops.h"
#include "mqtt.h"

#include "localConfig.h"

#include "globals.h"

#define RELAY_PIN 4

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 60       /* Time ESP32 will go to sleep (in seconds) */
#define TIME_TO_COMM 3
#define UPDATE_EVERY 10 // How often to send updates in seconds

#ifdef __cplusplus
extern "C"
{
#endif

  uint8_t temprature_sens_read();

#ifdef __cplusplus
}
#endif

bool isRelayOn;
time_t lastStatusSent;

void sendStatus() {
  log_e("Enter");
  const size_t capacity = JSON_OBJECT_SIZE(4) + 128;
  
  DynamicJsonDocument doc(capacity);

  lastStatusSent = getEpochTime();
  doc["time"] = lastStatusSent;
  doc["status"] = isRelayOn ? "ON" : "OFF";
  doc["temperature"] = (float)(temprature_sens_read() - 32) * 5 / 9;

  uint16_t ret = publish(doc.as<String>().c_str());
  log_d("publish() returned: %d", ret);
  if (ret == 0) {
    allowSleep();
  }
  log_e("Exit");
}

void relayOn()
{
  digitalWrite(RELAY_PIN, LOW);
  isRelayOn = true;
  Serial.println("RELAY is on");
  sendStatus();
}

void relayOff()
{
  digitalWrite(RELAY_PIN, HIGH);
  isRelayOn = false;
  Serial.println("RELAY is off");
  sendStatus();
}


const char * isSleepAllowedStr() {
  return(isSleepAllowed() ? "true" : "false");
}

void setup()
{
  log_e("Enter");
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  log_d("Starting main setup");
  preventSleep();
  isRelayOn = false;
  relayOff();
  
  setupWiFi(setupMQTT);
  log_d("Setups done.");

  subscribeToCommand();
  log_d("Subscribed to command topic.");

  log_d("Exit");
}

void loop()
{
  log_d("Enter. isRelayOn=%s, sleepAllowed: %s", isRelayOn ? "true" : "false", isSleepAllowedStr());
  // log_d("lastStatusSent=%d, getEpochTime()=%d, delta=%d", lastStatusSent, getEpochTime(), getEpochTime() - lastStatusSent);
  if ((lastStatusSent == 0 || lastStatusSent + UPDATE_EVERY <= getEpochTime())) {
    sendStatus();
  }
  if (!isRelayOn) { // Do not sleep at all if Relay is on
    while (!isSleepAllowed()) // If Relay is off, we wait until all communication/operations are completed. Warning: wait is endless!
    {
      delay(500);
    }

    log_i("Sleeping for %d seconds to complete async communication", TIME_TO_COMM);
    delay(TIME_TO_COMM * 1000);
    if (!isRelayOn) { // Important: check if relay has just been turned on.
      log_i("Deep Sleep for %d seconds", TIME_TO_SLEEP);
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      esp_deep_sleep_start();
    }
  }
  delay(500); // We don't want to burn too many CPU cycles, so sleep for 1/2 sec if relay is on.
  log_d("Exit");
}