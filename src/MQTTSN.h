#ifndef MQTTSN_H_
#define MQTTSN_H_

#include <Arduino.h>
#include <Stream.h>

#define DEBUG

#ifdef DEBUG
    #define debugPrintLn(...) { if (this->_debugStream) this->_debugStream->println(__VA_ARGS__); }
    #define debugPrint(...) { if (this->_debugStream) this->_debugStream->print(__VA_ARGS__); }
    #warning "Debug mode is ON"
#else
    #define debugPrintLn(...)
    #define debugPrint(...)
#endif

#ifndef MQTTSN_MAX_PACKET_SIZE
    #define MQTTSN_MAX_PACKET_SIZE 128
#endif

#define MAX_LENGTH_CLIENT_ID 23

#define ADVERTISE 0x00
#define SEARCHGW 0x01
#define GWINFO 0x02
//#define reserved 0x03
#define CONNECT 0x04
#define CONNACK 0x05
#define WILLTOPICREQ 0x06
#define WILLTOPIC 0x07
#define WILLMSGREQ 0x08
#define WILLMSG 0x09
#define REGISTER 0x0A
#define REGACK 0x0B
#define PUBLISH 0x0C
#define PUBACK 0x0D
#define PUBCOMP 0x0E
#define PUBREC 0x0F
#define PUBREL 0x10
//#define reserved 0x11
#define SUBSCRIBE 0x12
#define SUBACK 0x13
#define UNSUBSCRIBE 0x14
#define UNSUBACK 0x15
#define PINGREQ 0x16
#define PINGRESP 0x17
#define DISCONNECT 0x18
//#define reserved 0x19
#define WILLTOPICUPD 0x1A
#define WILLTOPICRESP 0x1B
#define WILLMSGUPD 0x1C
#define WILLMSGRESP 0x1D

#define PROTOCOL_ID 0x01

enum TopicIdType{
    NORMAL_TOPIC_ID,
    PREDEFINED_TOPIC_ID,
    SHORT_TOPIC_NAME
};

class MQTTSN{
    public:
        
        virtual boolean connect(const char * clientId) = 0;
        virtual void disconnect(uint16_t duration = 0) = 0;
        virtual boolean publish(const char * topic, const char * data) = 0;
        virtual boolean publish(uint16_t topic, const char * data) = 0;
        virtual boolean searchGateway() = 0;
        virtual boolean subscribe(const char * topic) = 0;
        virtual boolean subscribe(uint16_t topic);
        virtual boolean unsubscribe(const char * topic) = 0;
        virtual boolean unsubscribe(uint16_t topic) = 0;

        // Sets the optional "Diagnostics and Debug" stream.
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
        void setDebugStream(Stream *stream) { _debugStream = stream; }
                
    protected:
        byte buffer[MQTTSN_MAX_PACKET_SIZE];
        
        uint16_t nextMsgId = 1;
        

        void setClientId(const char * clientId);

        uint8_t buildConnectFrame(const char * clienId, int keepAlive = 10);
        uint8_t buildDisconnectFrame(uint16_t duration = 0);
        uint8_t buildPublishFrame(const char * topic, const char * data);
        uint8_t buildSearchGWFrame();    
        uint8_t buildPingReqFrame();    
        uint8_t buildPublishFrame(uint16_t topic, const char * data);
        uint8_t buildSubscribeOrUnsubscribeFrame(const char * topic, boolean IsSubscription);
        uint8_t buildSubscribeOrUnsubscribeFrame(uint16_t topic, boolean IsSubscription);
        
        Stream * _debugStream;
    private:
        uint8_t buildPublishFrameCommon(const char * data);
        
        char _clientId [MAX_LENGTH_CLIENT_ID];
};

#endif