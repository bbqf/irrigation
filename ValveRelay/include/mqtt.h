#ifndef __MQTT_RELAY_HPP__
#define __MQTT_RELAY_HPP__

void setupMQTT();
void subscribeToCommand();
uint16_t publish(const char* payload);
bool isMQTTConnected();

#endif