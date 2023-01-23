#ifndef __MQTT_HPP__
#define __MQTT_HPP__

#include <AsyncMqttClient.h>
#include <String>

class MQTTProcessor : public AsyncMqttClient {
    private:
        IPAddress mqttHost;
        String mqttHostName;
        int mqttPort;

        bool cleanQueue;

        void reconnectToMQTT();

        bool opRunning;
        void stopOp();

    public:
        virtual void onMqttConnect(bool sessionPresent);
        virtual void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
        virtual void onMqttSubscribe(uint16_t packetId, uint8_t qos);
        virtual void onMqttUnsubscribe(uint16_t packetId);
        virtual void onMqttMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
        virtual void onMqttPublish(uint16_t packetId);
        virtual void rebindCallbacks() = 0;
        void setupMQTT();
        void startOp();
        bool isOpRunning();
        MQTTProcessor(const char *cliendID, const char *mqttHost,  int mqttPort, bool cleanQueue);
        ~MQTTProcessor();

};

#endif