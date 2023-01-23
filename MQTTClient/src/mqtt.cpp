#include <Arduino.h>
#include <AsyncMqttClient.h>
#include <string.h>
#include <WiFi.h>


#include "mqtt.hpp"


void MQTTProcessor::reconnectToMQTT()
{
    log_d("Reconnecting to MQTT at %s:%d as '%s'", mqttHostName, mqttPort, getClientId());
    if (this->connected()) {
        log_d("Disconnecting...");
        startOp();
        disconnect();
        while (isOpRunning()) {delay(100);}
    }
    log_d("Connecting...");
    startOp();
    connect();
    while (isOpRunning()) {delay(100);}
}

void MQTTProcessor::onMqttConnect(bool sessionPresent)
{
    log_d("Connected to MQTT.");
    stopOp();
}

void MQTTProcessor::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    log_d("Disconnected from MQTT. Reason: %d", reason);
    stopOp();
}

void MQTTProcessor::onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    log_d("Subscribe acknowledged.");
    log_d("  packetId: %d", packetId);
    log_d("  qos: %d", qos);
    stopOp();
}

void MQTTProcessor::onMqttUnsubscribe(uint16_t packetId)
{
    log_d("Unsubscribe acknowledged.");
    log_d("  packetId: %d", packetId);
    stopOp();
}

void MQTTProcessor::onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    log_d("Message received.");
    log_d("  topic: %", topic);
    log_d("  qos: %d", properties.qos);
    log_d("  dup: %s", properties.dup?"true":"false");
    log_d("  retain: %s", properties.retain?"true":"false");
    log_d("  len: %d", len);
    log_d("  index: %d", index);
    log_d("  total: %d", total);
    String pl(payload,len);
    log_d("  payload: %s", pl);
    stopOp();
}

void MQTTProcessor::onMqttPublish(uint16_t packetId)
{
    log_d("Publish acknowledged.");
    log_d("  packetId: %d", packetId);
    stopOp();
}

void MQTTProcessor::setupMQTT()
{
    log_d("Setting up MQTT");

    rebindCallbacks();

    // log_d("Using %s as DNS Server", WiFi.dnsIP().toString());
    // int ret;
    // if ((ret = WiFi.hostByName(mqttHostName.c_str(), mqttHost)) == 1)
    {
        setServer(mqttHostName.c_str(), mqttPort);
        if (cleanQueue) { // clean messages once at start
            setCleanSession(true); // clear messages
            log_i("Cleaning queue for client '%s'", getClientId());
            reconnectToMQTT();
            cleanQueue = false;
        }
        setCleanSession(false); // do not clear messages
        reconnectToMQTT();
    }
    // else
    // {
    //     log_e("MQTT Host lookup failed. Aborting...");
    //     log_e("Error: %d", ret);
    // }
}

void MQTTProcessor::rebindCallbacks(){
    onConnect(std::bind(&MQTTProcessor::onMqttConnect, this, std::placeholders::_1));

    onDisconnect(std::bind(&MQTTProcessor::onMqttDisconnect, this, std::placeholders::_1));
    onSubscribe(std::bind(&MQTTProcessor::onMqttSubscribe, this, std::placeholders::_1, std::placeholders::_2));
    onUnsubscribe(std::bind(&MQTTProcessor::onMqttUnsubscribe, this, std::placeholders::_1));
    onMessage(std::bind(&MQTTProcessor::onMqttMessage,
                        this,
                        std::placeholders::_1,
                        std::placeholders::_2,
                        std::placeholders::_3,
                        std::placeholders::_4,
                        std::placeholders::_5,
                        std::placeholders::_6));
    onPublish(std::bind(&MQTTProcessor::onMqttPublish, this, std::placeholders::_1));
}

MQTTProcessor::MQTTProcessor(const char *cliendID, const char *mqttHost,  int mqttPort, bool cleanQueue)
{
    log_i("Recreating MQTTProcessor");
    setClientId(cliendID);
    this->mqttHostName = mqttHost;
    this->mqttPort = mqttPort;
    this->cleanQueue = cleanQueue;
}

MQTTProcessor::~MQTTProcessor()
{
}

void MQTTProcessor::startOp() {
    this->opRunning = true;
}

void MQTTProcessor::stopOp() {
    opRunning = false;
}

bool MQTTProcessor::isOpRunning() {
    return opRunning;
}

