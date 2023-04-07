#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

#include "wifi_ops.h"
#include "nvs_preferences.h"

bool doNotSleep;

onConnCallBack onConnCB;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

void WiFionStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFionGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFionStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

// getEpochTime() uses whatever methods to get the Unix Epoch time, currently in wifi.cpp since NTP is the main method.
time_t getEpochTime() {
  time_t t = 0;
  if (WiFi.isConnected()) {
    log_d("WiFi connected");
    if (timeClient.isTimeSet()) {
      log_d("timeClient time is set");
      t = timeClient.getEpochTime();
    } else {
      log_d("timeClient time is not set");
      timeClient.begin();
      bool success = timeClient.update();
      log_d("time updated, result: %d", success);
      t = timeClient.getEpochTime();
    }
  }

  log_d("returning time: %ld", t);
  return(t);
}

void setupWiFi(onConnCallBack cb)
{
  onConnCB = cb;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  WiFi.begin(getSSID(), getPassword());
  WiFi.onEvent(WiFionGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFionStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  while (!WiFi.isConnected())
  {
    delay(500);
    Serial.print(".");
  }
  timeClient = NTPClient(ntpUDP, getNtpServer());
  time_t t = getEpochTime();
  struct tm  ts;
  char       buf[80];
  // Format time, "ddd yyyy-mm-dd hh:mm:ss zzz"
  ts = *localtime(&t);
  strftime(buf, sizeof(buf), "%a %Y-%m-%d %H:%M:%S %Z", &ts);

  log_i("Time: %s", buf); 
}

void WiFionGotIP(WiFiEvent_t event, WiFiEventInfo_t info)
{
  log_i("Hostname: %s", WiFi.getHostname());
  if (onConnCB != 0)
  {
    onConnCB();
  }
}

void WiFionStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info)
{
  // Serial.println("Disconnected from WiFi access point");
  //  Serial.println("Waiting for 10s");
  log_e("WiFi lost connection. Reason: %d", info.wifi_sta_disconnected.reason);
  log_i("Trying to Reconnect");
   WiFi.begin(getSSID(), getPassword());
}


void preventSleep() {
  log_d("Preventing sleep");
  doNotSleep = true;
}

void allowSleep() {
  log_d("Allowing sleep");
  doNotSleep = false;
}

bool isSleepAllowed() {
  return(!doNotSleep);
}
