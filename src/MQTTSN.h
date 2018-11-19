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

// Time in seconds
#define T_ADV 960
#define N_ADV 3
#define T_SEARCH_GW 5
#define T_GW_INFO 5
#define T_WAIT 360
#define T_RETRY 10
#define N_RETRY 5

enum class MqttsnState : uint8_t {
    DISCONNECTED,
    ACTIVE,
    ASLEEP,
    AWAKE,
    LOST
};

enum class MqttsnOperationError : int {
    NO_ERROR,
    TIMEOUT = 0x80,
    NO_EXPECTED_MSG_TYPE
};

class Mqttsn {
    
    public:
        Mqttsn(MqttsnClient &mqttsnClient);
        ~Mqttsn() {}

        void loop();

        int searchGateway();
        int connect(const char * clientId, bool cleanSession = true, uint16_t keepAlive = 60);
        void setAutoreconnect(bool autoreconnect);      
        void setWillOptions(const char * willTopic, const char * willMsg, bool retain = false, MQTTSN_QoS qos = MQTTSN_QoS::QoS0 );
        
        void disconnect(uint16_t duration);
        
        void setDebugStream(Stream &stream);        

    private:
        void _sendSearchGw();
        void _sendConnect(const char * clientId, bool cleanSession, uint16_t keepAlive);

        int _handleGwInfo();
        int _handleConnack();
        int _handleWillTopicReq();
        int _handleWillMsgReq();

        bool _reconnect();
        void _disconnectedLoop();       

        void _activeLoop();
        void _awakeLoop();
        
        void _sendMsg(uint8_t * fixedMsgSection, uint8_t fixedMsgSectionLength, bool broadcast = false);
        void _sendMsg(uint8_t * fixedMsgSection, uint8_t fixedMsgSectionLength, 
                        uint8_t * variableMsgSection, uint8_t variableMsgSectionLength, bool broadcast = false);
        
        bool _waitMsg(int milliseconds = T_RETRY);
        uint8_t _computeFlags(bool dup, MQTTSN_QoS qos, bool retain, bool will, bool cleanSession, TOPIC_ID_TYPE topicIdType);
        
        MqttsnClient * _mqttsnClient;
        MqttsnMsg _mqttsnMsg;
        MqttsnState _state = MqttsnState::DISCONNECTED;

        char _clientId[MAX_LENGTH_CLIENT_ID + 1];
        uint16_t _keepAlive;
        
        
        bool _autoreconnect = false;
        

        const char * _willTopic;
        const char * _willMsg;
        uint8_t _willFlags;

        unsigned long _lastMsgSentMillis;
        unsigned long _lastMsgReceiveMillis;
        
        Stream * _debugStream;
        uint8_t _retries = 0;

        
};

#endif // __MQTTSN_H__