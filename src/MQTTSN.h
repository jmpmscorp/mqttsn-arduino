#pragma once

#ifndef __MQTTSN_H__
#define __MQTTSN_H__

#ifndef CUSTOM_STREAM_DEBUG
    #define CUSTOM_STREAM_DEBUG
#endif

#include "MqttsnDebug.h"

#include "Clients/MqttsnClient.h"

#if defined(ZIGBEE_MQTTSN_CLIENT)
    #include "Clients/ZigbeeMqttsnClient.h"    
#endif

#include <Arduino.h>
#include "MqttsnMsg.h"


#define T_ADV 960
#define N_ADV 3
#define T_SEARCH_GW 5
#define T_GW_INFO 5
#define T_WAIT 360
#define T_RETRY 15
#define N_RETRY 5



class Mqttsn {
    
    public:
        Mqttsn(MqttsnClient &mqttsnClient);
        ~Mqttsn() {}

        bool searchGateway();
        bool connect(const char * clientId, uint16_t keepAlive = 60);
        bool connect(const char * clientId, bool willFlag, bool cleanSession = true, uint16_t keepAlive = 15);
        void setDebugStream(Stream &stream);

    private:
        bool _waitResponse(int milliseconds);
        uint8_t _computeFlags(bool dup, MQTTSN_QoS qos, bool retain, bool will, bool cleanSession, TOPIC_ID_TYPE topicIdType);
        Stream * _debugStream;
        MqttsnClient * _mqttsnClient;

        uint8_t _retries = 0;

        MqttsnMsg _mqttsnMsg;
};

#endif // __MQTTSN_H__