#pragma once

#ifndef __MQTTSN_MSG_H__
#define __MQTTSN_MSG_H__

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

enum class TOPIC_ID_TYPE : uint8_t {
    NORMAL_TOPIC,
    PREDEFINED_TOPIC,
    SHORT_TOPIC_NAME
};

enum MQTTSN_QoS : uint8_t {
    QoS0,
    QoS1,
    QoS2,
    QoS3  // -1
};

enum class ReturnCode : uint8_t {
    ACCEPTED,
    CONGESTION,
    INVALID_TOPIC_ID,
    NOT_SUPPORTED
};

class MqttsnMsg {
    public:
        MqttsnMsg() {}       
        
        uint8_t getLength() const { return _inputBuffer[0]; }        
        uint8_t getMsgType() const { return _inputBuffer[1]; }

        uint8_t * getOutputBuffer() { return _outputBuffer; }
        void setInputBuffer(uint8_t * buffer) { _inputBuffer = buffer; }
        
        virtual void build(uint8_t length, uint8_t msgType) {
            setLength(length);
            setMsgType(msgType);
        }       
    
    protected:
        uint8_t _outputBuffer[8];
        uint8_t * _inputBuffer;
    
    private:
        void setLength(uint8_t length) { _outputBuffer[0] = length; }
        void setMsgType(uint8_t msgType) { _outputBuffer[1] = msgType; }
};

class SearchGwMqttsnMsg : public MqttsnMsg {
    public:
        SearchGwMqttsnMsg() : MqttsnMsg() {}
        uint8_t getRadius() const { return _inputBuffer[2]; }

        void build(uint8_t radius) {
            MqttsnMsg::build(0x03, SEARCHGW);
            setRadius(radius);
        }
    
    private:
        void setRadius(uint8_t radius) { _outputBuffer[2] = radius; }

};

class GwInfoMqttsnMsg : public MqttsnMsg {
    public:
        GwInfoMqttsnMsg() : MqttsnMsg() {}
        
        uint8_t getGwId() const { return _inputBuffer[2]; }
        uint8_t * getGwAddr() const { return & _inputBuffer[3]; }

        void build(uint8_t length, uint8_t gwId) {
            MqttsnMsg::build(length, GWINFO);
            setGwId(gwId);
        }
    
    private:
        void setGwId(uint8_t gwId) { _outputBuffer[2] = gwId; }
        // void setGwAddr(uint8_t * gwAddr, int length) { memcpy(&_outputBuffer[3], gwAddr, length); }

};

class ConnectMqttsnMsg : public MqttsnMsg {
    public:
        ConnectMqttsnMsg() : MqttsnMsg() {}
        uint8_t getFlags() const { return _inputBuffer[2]; }
        uint8_t getProtocolId() const { return _inputBuffer[3]; }        
        uint16_t getDuration() const { return _inputBuffer[4] * 256 + _inputBuffer[5]; }
        const char * getClient() { return reinterpret_cast<const char *>(&_inputBuffer[6]); }

        void build(uint8_t length, uint8_t flags, uint16_t duration) {
            MqttsnMsg::build(length, CONNECT);
            setFlags(flags);
            setProtocolId();
            setDuration(duration);
        }
    
    private:
        void setFlags(uint8_t flags) { _outputBuffer[2] = flags; }
        void setProtocolId(uint8_t protocolId = PROTOCOL_ID) { _outputBuffer[3] = protocolId; }
        void setDuration(uint16_t duration) {
            _outputBuffer[4] = duration / 256;
            _outputBuffer[5] = duration % 256;
        }

        // void setClientId(const char * clientId) { memcpy(&_outputBuffer[6], clientId, strlen(clientId)); }

};

class ConnackMqttsnMsg : public MqttsnMsg {
    public:
        ConnackMqttsnMsg() : MqttsnMsg() {}
        ReturnCode getReturnCode() const { return static_cast<ReturnCode>(_inputBuffer[2]); }
        void build(ReturnCode returnCode) { 
            MqttsnMsg::build(0x03, CONNACK);
            setReturnCode(returnCode);
        }
    private:
        void setReturnCode(ReturnCode returnCode) { _outputBuffer[2] = static_cast<uint8_t>(returnCode); }
};

/*class DisconnectMqttsnMsg : public MqttsnMsg {
    public:
        DisconnectMqttsnMsg : 

};*/

#endif // __MQTTSN_MSG_H__