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

class MQTTSNCommon{
    public:
        
        virtual boolean connect(const char * clientId, int keepAlive = 15);
        virtual void disconnect(uint16_t duration = 0) = 0;
        virtual boolean publish(const char * topic, const char * data) = 0;
        virtual boolean publish(uint16_t topic, const char * data) = 0;
        virtual boolean searchGateway();
        virtual boolean subscribe(const char * topic) = 0;
        virtual boolean subscribe(uint16_t topic);
        virtual boolean unsubscribe(const char * topic) = 0;
        virtual boolean unsubscribe(uint16_t topic) = 0;
        virtual boolean pingReq(const char * clientId) = 0;

        // Sets the optional "Diagnostics and Debug" stream.
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
        void setDebugStream(Stream *stream) { _debugStream = stream; }
                
        //MQTTSNParser * mqttsnParser = new MQTTSNParser();
    protected:
        uint16_t nextMsgId = 1;
        uint8_t * responseBuffer;
        Stream * _debugStream;

        MQTTSNParser * mqttsnParser;

        void setClientId(const char * clientId);
              
        
    private:
        virtual void _saveGatewayAddress() {};
        virtual uint8_t _sendPacket(uint8_t length, boolean broadcast = false) {};
        virtual uint8_t _sendBroadcastPacket(uint8_t length) {};
        virtual boolean _waitResponsePacket(int timeout = 2000) {};        

        char _clientId [MAX_LENGTH_CLIENT_ID];
};

#endif