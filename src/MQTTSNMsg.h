#ifndef MQTTSNMSG_H_
#define MQTTSNMSG_H_

#include <Arduino.h>

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

enum ReturnCode{
ACCEPTED,
CONGESTION,
INVALID_TOPIC_ID,
NOT_SUPPORTED
};

enum TopicIdType{
NORMAL_TOPIC_ID,
PREDEFINED_TOPIC_ID,
SHORT_TOPIC_NAME
};

#pragma region Messages Structures
struct mqttsn_msg_header {
    uint8_t length;
    uint8_t type;
};

struct mqttsn_msg_advertise : public mqttsn_msg_header {
    uint8_t gwId;
    uint16_t duration;
};

struct mqttsn_msg_searchgw : public mqttsn_msg_header {
    uint8_t radius;
};

struct mqttsn_msg_gwinfo : public mqttsn_msg_header {
    uint8_t gwId;
    char gwAdd[0];
};

struct mqttsn_msg_connect : public mqttsn_msg_header {
    uint8_t flags;
    uint8_t protocolId;
    uint16_t keepAlive;
    char clientId[0];
};

struct mqttsn_msg_connack : public mqttsn_msg_header {
    uint8_t returnCode;
};

struct mqttsn_msg_willtopic : public mqttsn_msg_header {
    uint8_t flags;
    char willTopic[0];
};

struct mqttsn_msg_willmsg : public mqttsn_msg_header {
    char willmsg[0];
};

struct mqttsn_msg_register : public mqttsn_msg_header {
    unsigned int topicId;
    unsigned int messageId;
    char * topicName;
};

struct mqttsn_msg_regack : public mqttsn_msg_header {
    unsigned int topicId;
    unsigned int messageId;
    uint8_t returnCode;
};

struct mqttsn_msg_publish : public mqttsn_msg_header {
    uint8_t flags;
    union{
        unsigned int topicId;
        char topicName [2];
    };    
    unsigned int messageId;
    char data[0];
};

struct mqttsn_msg_puback : public mqttsn_msg_header {
    unsigned int topicId;
    unsigned int messageId;
    uint8_t returnCode;
};

struct mqttsn_msg_pubqos2 : public mqttsn_msg_header {
    unsigned int messageId;
};

struct mqttsn_msg_subOrUnsubscribe : public mqttsn_msg_header {
    uint8_t flags;
    unsigned int messageId;
    union {
        char topicName[0];
        unsigned int topicId;
    };
};

struct mqttsn_msg_suback : public mqttsn_msg_header {
    uint8_t flags;
    unsigned int topicId;
    unsigned int messageId;
    uint8_t returnCode;
};

struct mqttsn_msg_unsubscribe : public mqttsn_msg_header {
    uint8_t flags;
    unsigned int message_id;
    union {
        char topic_name[0];
        unsigned int topic_id;
    };
};

struct mqttsn_msg_unsuback : public mqttsn_msg_header {
    unsigned int messageId;
};

struct mqttsn_msg_pingreq : public mqttsn_msg_header {
    char clientId[0];
};

struct mqttsn_msg_disconnect : public mqttsn_msg_header {
    unsigned int duration;
};

struct mqttsn_msg_willtopicresp : public mqttsn_msg_header {
    uint8_t returnCode;
};

struct mqttsn_msg_willmsgresp : public mqttsn_msg_header {
    uint8_t return_code;
};
#pragma endregion

class MQTTSNParser{
    public:
        

        MQTTSNParser();

        uint8_t connectFrame(const char * clienId, int keepAlive = 10);
        uint8_t disconnectFrame(unsigned int duration = 0);
        uint8_t searchGWFrame();    
        uint8_t pingReqFrame(const char * clientId);
        uint8_t pingRespFrame();
        uint8_t pubAckFrame(uint8_t topicId, uint8_t msgId, uint8_t returnCode);
        uint8_t publishFrame(unsigned int topic, boolean predefined, boolean retain, const char * data, unsigned int nextMsgId, uint8_t qos = 0);
        uint8_t publishFrame(const char * topic, boolean retain, const char * data, unsigned int nextMsgId, uint8_t qos = 0);
        uint8_t subscribeOrUnsubscribeFrame(const char * topic, unsigned int nextMsgId, boolean IsSubscription);
        uint8_t subscribeOrUnsubscribeFrame(unsigned int topic, unsigned int nextMsgId, boolean IsSubscription);
        
        byte buffer[MQTTSN_MAX_PACKET_SIZE];
  
        unsigned int _bswap(const unsigned int val);
    
    private:
        //uint8_t publishFrameCommon(const char * data, unsigned int nextMsgId);
        
};

#endif

