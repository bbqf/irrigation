#include <Arduino.h>
#include <AsyncMqttClient.h>

#include "main.h"
#include "mqtt.h"
#include "localConfig.h"
#include "globals.h"


AsyncMqttClient mqtt;

void onConnect(bool sessionPresent) {
    allowSleep();
    log_d("Called. Status: %s", mqtt.connected() ? "true" : "false");
}

uint16_t publish(const char* payload) {
    return(mqtt.publish(MQTT_STATUS_TOPIC, 0, false, payload));
}

bool isMQTTConnected() {
    bool c = mqtt.connected();
    log_d("Returning %s", c ? "true" : "false");
    return(c);
}

void setupMQTT() {
    log_d("Enter");
    preventSleep();

    mqtt.setClientId(MQTT_CLIENT_ID);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
 
    // Set callbacks
    mqtt.onConnect(onConnect); // basically this is only not to go into deep sleep while connecting.
        
    mqtt.setCleanSession(true); // clear messages - we don't plan to subscribe anyway
    log_d("Connecting...");
    mqtt.connect();
    
    log_d("Exiting. Status: %s", mqtt.connected() ? "true" : "false");
}