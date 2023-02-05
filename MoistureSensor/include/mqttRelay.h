#ifndef __MQTT_RELAY_HPP__
#define __MQTT_RELAY_HPP__

void setupMQTT();
uint16_t publish(const char* payload);
bool isMQTTConnected();

#endif