#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Preferences.h>

#include "wifi_ops.h"
#include "localConfig.h"

bool doNotSleep;

onConnCallBack onConnCB;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_POOL, 3600);

void WiFionStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFionGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFionStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

// getEpochTime() uses whatever methods to get the Unix Epoch time, currently in wifi.cpp since NTP is the main method.
time_t getEpochTime() {
  time_t t = 0;
  if (WiFi.isConnected()) {
    if (timeClient.isTimeSet()) {
      t = timeClient.getEpochTime();
    } else {
      timeClient.begin();
      timeClient.update();
      t = timeClient.getEpochTime();
    }
  }

  return(t);
}

void setupWiFi(onConnCallBack cb)
{
  onConnCB = cb;
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.setSleep(false);
  WiFi.begin(preferences.getString("ssid", "").c_str(), preferences.getString("password", "").c_str());
  WiFi.onEvent(WiFionGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFionStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  while (!WiFi.isConnected())
  {
    delay(500);
    Serial.print(".");
  }

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
   WiFi.begin(preferences.getString("ssid", "").c_str(), preferences.getString("password", "").c_str());
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
