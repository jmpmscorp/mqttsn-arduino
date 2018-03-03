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
         
        boolean connect(const char * clientId, int keepAlive = 15);
        void disconnect(unsigned int duration = 0);
        
        boolean publish(const char * topic, boolean retain, const char * data, uint8_t qos = 0);
        boolean publish(unsigned int topic, boolean predefined, boolean retain, const char * data, uint8_t qos = 0);
        boolean searchGateway();
        boolean searchGwAndConnect(const char * clientId, int keepAlive = 15);
        uint8_t searchGwAndConnectAsync(const char * clientId, int keepAlive = 15);
        boolean pingReq();
        void pingResp();
        void pubAck(uint8_t topicId, uint8_t msgId, uint8_t returnCode = ACCEPTED);
        boolean subscribe(const char * topic, unsigned int * topicIdOut);
        boolean subscribe(unsigned int topic, unsigned int * topicIdOut);
        boolean unsubscribe(const char * topic);
        boolean unsubscribe(unsigned int topic);

        boolean isWaitingPingResp();
        uint8_t continuosAsyncTask();
        void sleep(unsigned int duration);
        boolean awake();

        uint8_t getState();
        unsigned long getLastReceived();
        void setState(uint8_t state);
        
        void setTopicMsgCallback(void(*f)(unsigned int topicId, unsigned int topicIdType, const char * data, unsigned int dataLength, bool retain));
        void setShortTopicCallback(void(*f)(const char * topicName, const char * data, unsigned int dataLength, bool retain));
        void setOnDisconnectCallback(void(*f)(void));
        
        void setWdtChecker(void (*f)());
        void setCustomDelay (void (*f)(unsigned long));
        // Sets the optional "Diagnostics and Debug" stream.
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
        void setDebugStream(Stream *stream) { _debugStream = stream; }
                
    protected:
        unsigned int nextMsgId = 1;
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

        boolean _waitingPingResp = false;
        
        //When device is in Sleep Cicle, number of PingResp no received to considered LOST
        uint8_t _pingRespRetries = 0;

        boolean _publishCommon(uint8_t frameLength, uint8_t qos);
        boolean _subscribeCommon(uint8_t frameLength, unsigned int * topicIdOut);
        boolean _unsubscribeCommon(uint8_t frameLength);
        
        boolean _handleConnack();
        boolean _handlePuback(unsigned int lastMsgId);

        void _setKeepAlive(unsigned int keepAlive);
        boolean _checkBeforePublish();
        virtual void _incrementNextMsgId() = 0;
        virtual void _saveGatewayAddress() = 0;
        virtual uint8_t _sendPacket(uint8_t length, boolean broadcast = false) = 0;
        virtual uint8_t _sendBroadcastPacket(uint8_t length) = 0;
        virtual boolean _waitResponsePacket(int timeout = 2000) = 0;
        virtual boolean _continuosWait() = 0;

                
        void (*_onTopicMsgCallback)(unsigned int topicId, unsigned int topicIdType, const char * data, unsigned int dataLength, bool retain);
        void (*_onShortTopicCallback)(const char * topicName, const char * data, unsigned int dataLength, bool retain);
        void (*_onDisconnectCallback)(void);
        
};

#endif