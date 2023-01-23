#include <Arduino.h>
#include <AsyncMqttClient.h>

#include "main.h"
#include "mqttRelay.h"
#include "localConfig.h"
#include "globals.h"


AsyncMqttClient mqtt;

void onMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    if (!strncmp(payload, "ON", len<2?len:2)) {
        relayOn();
        preventSleep();
    } else if (!strncmp(payload, "OFF", len<3?len:3)) {
        relayOff();
        allowSleep();
    } else {
        String pl(payload,len);
        log_d("Invalid payload: %s", pl);
    }
}

void onConnect(bool sessionPresent) {
    allowSleep();
    log_d("Called. Status: %s", mqtt.connected() ? "true" : "false");
}

void subscribeToCommand() {
    mqtt.subscribe(MQTT_COMMAND_TOPIC, 2);
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
    bool wakeFromSleep = esp_sleep_get_wakeup_cause() != 0; // if 0 it means reboot, everything rest is some kind of wakeup
    
    mqtt.setClientId(MQTT_CLIENT_ID);
    mqtt.setServer(MQTT_HOST, MQTT_PORT);
    if (wakeFromSleep == 0) { // After full reboot clean all messages in the queue first, we don't want to process any messages that are weeks old. 
    // Another option would be to introduce expiry time in messages.
        log_i("Cleaning queue for client '%s'", mqtt.getClientId());
        mqtt.setCleanSession(true); // clear messages
        log_d("Connecting...");
        mqtt.connect();
        log_d("Disconnecting...");
        mqtt.disconnect();
    }

    // Set callbacks
    mqtt.onMessage(onMessage);
    mqtt.onConnect(onConnect); // basically this is only not to go into deep sleep while connecting.
        
    mqtt.setCleanSession(false); // do not clear messages
    log_d("Connecting...");
    mqtt.connect();
    
    log_d("Exiting. Status: %s", mqtt.connected() ? "true" : "false");
}