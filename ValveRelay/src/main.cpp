#include <Arduino.h>

#include <ArduinoJson.h>

#include "wifi_ops.h"
#include "mqtt.h"
#include "nvs_preferences.h"

#include "globals.h"

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */

#define REV(x) #x

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
  digitalWrite(getRelayPin(), HIGH);
  isRelayOn = true;
  Serial.println("RELAY is on");
  sendStatus();
}

void relayOff()
{
  digitalWrite(getRelayPin(), LOW );
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
  pinMode(getRelayPin(), OUTPUT);
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
  if ((lastStatusSent == 0 || lastStatusSent + getUpdatePeriod() <= getEpochTime())) {
    sendStatus();
  }

  if (!isRelayOn) { // Do not sleep at all if Relay is on
    while (!isSleepAllowed()) // If Relay is off, we wait until all communication/operations are completed. Warning: wait is endless!
    {
      delay(500);
    }

    log_i("Sleeping for %d seconds to complete async communication", getTimeToComm());
    delay(getTimeToComm() * 1000);
    if (!isRelayOn) { // Important: check if relay has just been turned on.
      log_i("Deep Sleep for %d seconds", getTimeToSleep() );
      esp_sleep_enable_timer_wakeup(getTimeToSleep()  * uS_TO_S_FACTOR);
      esp_deep_sleep_start();
    }
  }
  delay(500); // We don't want to burn too many CPU cycles, so sleep for 1/2 sec if relay is on.
  log_d("Exit");
}