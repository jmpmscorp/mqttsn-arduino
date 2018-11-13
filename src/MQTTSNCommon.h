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
#define N_RETRY 3       //Number retries
#define KEEP_ALIVE 15
#define T_PINGRESP 5    //PingReq waiting seconds when MQTTSN_AWAKE

enum MQTTSN_STATES{
    MQTTSN_DISCONNECTED,
    MQTTSN_ACTIVE,
    MQTTSN_ASLEEP,
    MQTTSN_AWAKE,
    MQTTSN_LOST
};

enum MQTTSN_ERROR_CODE{
    MQTTSN_NO_ERROR = 0x00,
    MQTTSN_MAX_RETRIES_REACH = 0x10,
    MQTTSN_PINGRESP_TIMEOUT = 0x20,
    MQTTSN_NO_COMMAND_ARRIVE = 0xFF
};

enum STABLISH_CONNECTION_STATE{
    SEARCHING_GW,
    TRYING_CONNECT
};

class MQTTSNCommon{
    public:
         
        boolean connect(const char * clientId, uint16_t keepAlive = 15);
        void disconnect(uint16_t duration = 0);
        
        bool publish(const char * topic, bool retain, const char * data, uint8_t qos = 0);
        bool publish(uint16_t topic, bool predefined, bool retain, const char * data, uint8_t qos = 0);
        bool searchGateway();
        bool searchGwAndConnect(const char * clientId, uint16_t keepAlive = 15);
        uint8_t searchGwAndConnectAsync(const char * clientId, uint16_t keepAlive = 15);
        bool pingReq();
        void pingResp();
        void pubAck(uint16_t topicId, uint16_t msgId, uint8_t returnCode = ACCEPTED);
        bool subscribe(const char * topic, uint16_t * topicIdOut);
        bool subscribe(uint16_t topic, uint16_t * topicIdOut);
        bool unsubscribe(const char * topic);
        bool unsubscribe(uint16_t topic);

        bool isWaitingPingResp();
        uint8_t continuosAsyncTask();
        void sleep(uint16_t duration);
        bool awake();

        uint8_t getState();
        unsigned long getLastReceived();
        void setState(uint8_t state);
        
        void setTopicMsgCallback(void(*f)(uint16_t topicId, uint8_t topicIdType, const char * data, uint16_t dataLength, bool retained));
        void setShortTopicCallback(void(*f)(const char * topicName, const char * data, uint16_t dataLength, bool retained));
        void setOnDisconnectCallback(void(*f)(void));
        
        void setWdtChecker(void (*f)());
        void setCustomDelay (void (*f)(unsigned long));
        // Sets the optional "Diagnostics and Debug" stream.
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
        void setDebugStream(Stream *stream) { _debugStream = stream; }
                
    protected:
        uint16_t nextMsgId = 1;
        uint8_t responseBuffer[MQTTSN_MAX_PACKET_SIZE];
        Stream * _debugStream;
        //Maybe this buffer could be erase but without it, Arduino is out of memory
        char _onDataBuffer[70];
        MQTTSNParser * mqttsnParser;

        unsigned long _lastSent = 0;
        unsigned long _lastReceived = 0;

        void (*_wdtChecker)();
        void (*_delay)(unsigned long) = delay;
        void setClientId(const char * clientId);
              
        
    private:
        char _clientId [MAX_LENGTH_CLIENT_ID + 1];
        uint8_t _state = MQTTSN_DISCONNECTED;
        uint8_t _retries = 0;
        uint8_t _keepAlive = KEEP_ALIVE;

        bool _waitingPingResp = false;
        
        //When device is in Sleep Cicle, number of PingResp no received to considered LOST
        uint8_t _pingRespRetries = 0;

        bool _publishCommon(uint8_t frameLength, uint8_t qos);
        bool _subscribeCommon(uint8_t frameLength, uint16_t * topicIdOut);
        bool _unsubscribeCommon(uint8_t frameLength);
        
        boolean _handleConnack();
        boolean _handlePuback(uint16_t lastMsgId);

        void _setKeepAlive(uint16_t keepAlive);
        boolean _checkBeforePublish();
        void _setLost();
        virtual void _incrementNextMsgId() = 0;
        virtual void _saveGatewayAddress() = 0;
        virtual uint8_t _sendPacket(uint8_t length, bool broadcast = false) = 0;
        virtual uint8_t _sendBroadcastPacket(uint8_t length) = 0;
        virtual bool _waitResponsePacket(uint16_t timeout = 2000) = 0;
        virtual bool _continuosWait() = 0;

                
        void (*_onTopicMsgCallback)(uint16_t topicId, uint8_t topicIdType, const char * data, uint16_t dataLength, bool retain);
        void (*_onShortTopicCallback)(const char * topicName, const char * data, uint16_t dataLength, bool retain);
        void (*_onDisconnectCallback)(void);
        
};

#endif