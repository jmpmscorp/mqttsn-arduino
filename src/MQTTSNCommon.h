#ifndef MQTTSNCOMMON_H_
#define MQTTSNCOMMON_H_

#include <Arduino.h>
#include <Stream.h>
#include "MQTTSNMsg.h"

#define DEBUG

#ifdef DEBUG
    #define debugPrintLn(...) { if (this->_debugStream) this->_debugStream->println(__VA_ARGS__); }
    #define debugPrint(...) { if (this->_debugStream) this->_debugStream->print(__VA_ARGS__); }
    #warning "Debug mode is ON"
#else
    #define debugPrintLn(...)
    #define debugPrint(...)
#endif

#define T_ADV 15 //Minutes
#define N_ADV 3  //Advertise retries
#define T_SEARCHGW 5    //Waiting SearchGW response in seconds
#define T_GWINFO 5      //Waiting GWinfo response in seconds
#define T_RETRY 10      //Seconds between pingReq retries before sleep
#define N_RETRY 3       //Retry number sleep->pingReq

enum MQTTSN_STATES{
    MQTTSN_DISCONNECTED,
    MQTTSN_ACTIVE,
    MQTTSN_ASLEEP,
    MQTTSN_AWAKE
};

class MQTTSNCommon{
    public:
         
        boolean connect(const char * clientId, int keepAlive = 15);
        void disconnect(uint16_t duration = 0);
        
        boolean publish(const char * topic, boolean retain, const char * data, uint8_t qos = 0);
        boolean publish(uint16_t topic, boolean predefined, boolean retain, const char * data, uint8_t qos = 0);
        boolean searchGateway();
        boolean pingReq();
        void pingResp();
        void pubAck(uint8_t topicId, uint8_t msgId, uint8_t returnCode = ACCEPTED);
        boolean subscribe(const char * topic, uint16_t * topicIdOut);
        boolean subscribe(uint16_t topic, uint16_t * topicIdOut);
        //virtual boolean unsubscribe(const char * topic) = 0;
        //virtual boolean unsubscribe(uint16_t topic) = 0;

        uint8_t getState();
        boolean searchGwAndConnect(const char * clientId, int keepAlive = 15);
        void setTopicMsgCallback(void(*f)(uint16_t topicId, uint16_t topicIdType, const char * data, uint16_t dataLength));
        void setShortTopicCallback(void(*f)(const char * topicName, const char * data, uint16_t dataLength));
        void sleep(uint16_t duration);
        boolean awake();
        uint8_t loopTask();

        // Sets the optional "Diagnostics and Debug" stream.
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
        void setDebugStream(Stream *stream) { _debugStream = stream; }
                
        //MQTTSNParser * mqttsnParser = new MQTTSNParser();
    protected:
        uint16_t nextMsgId = 1;
        uint8_t responseBuffer[MQTTSN_MAX_PACKET_SIZE];
        Stream * _debugStream;
        //Maybe this buffer could be erase but without it, Arduino is out of memory
        char _onDataBuffer[70];
        MQTTSNParser * mqttsnParser;

        unsigned long _lastSent = 0;
        void setClientId(const char * clientId);
              
        
    private:
        boolean _publishCommon(uint8_t frameLength, uint8_t qos);
        boolean _subscribeCommon(uint8_t frameLength, uint16_t * topicIdOut);
        
        virtual void _incrementNextMsgId() = 0;
        virtual void _saveGatewayAddress() = 0;
        virtual uint8_t _sendPacket(uint8_t length, boolean broadcast = false) = 0;
        virtual uint8_t _sendBroadcastPacket(uint8_t length) = 0;
        virtual boolean _waitResponsePacket(int timeout = 2000) = 0;
        virtual boolean _continuosWait() = 0;

                
        void (*_onTopicMsgCallback)(uint16_t topicId, uint16_t topicIdType, const char * data, uint16_t dataLength);
        void (*_onShortTopicCallback)(const char * topicName, const char * data, uint16_t dataLength);
        char _clientId [MAX_LENGTH_CLIENT_ID + 1];
        uint8_t _state = MQTTSN_DISCONNECTED;
        
};

#endif