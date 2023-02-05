#ifndef __WIFI_OPS_H__
#define __WIFI_OPS_H__

typedef std::function<void()> onConnectCallBack;
typedef void(*onConnCallBack)();

void setupWiFi(onConnCallBack cb);
time_t getEpochTime();


#endif