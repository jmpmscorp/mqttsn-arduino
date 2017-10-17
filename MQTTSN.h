#ifndef MQTTSN_H_
#define MQTTSN_H_

#include <Arduino.h>
#include <Stream.h>



#ifndef MQTTSN_MAX_PACKET_SIZE
    #define MQTTSN_MAX_PACKET_SIZE 128
#endif

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


class MQTTSN{
    public:
        
        virtual boolean connect(const char * id) = 0;
        virtual void disconnect() = 0;
        virtual boolean publish(const char * topic, const char * payload) = 0;
        virtual boolean searchGateway() = 0;
        virtual boolean subscribe(const char * topic) = 0;
        virtual boolean unsubscribe(const char * topic) = 0;
                
    protected:
        byte buffer[MQTTSN_MAX_PACKET_SIZE];
         
        uint8_t buildSearchGWFrame();    
};

#endif