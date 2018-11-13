#pragma once

#ifndef __MQTTSN_CLIENT__
#define __MQTTSN_CLIENT__

#include "Stream.h"
#include "inttypes.h"

#ifndef MQTTSN_MAX_PACKET_SIZE
    #define MQTTSN_MAX_PACKET_SIZE 100
#endif

#define CUSTOM_STREAM_DEBUG
#include "../MqttsnDebug.h"

class MqttsnClient {
    public: 
        virtual bool packetAvailable() = 0;
        // virtual int readPacket(int waitMilliseconds);
        virtual int readPacket() = 0;
        virtual int sendPacket(bool broadcast = false) = 0;
        virtual void saveGatewayAddress() = 0;

        virtual void setSerial(Stream &stream) {};
        uint8_t * getMqttsnFrameBuffer() { return _mqttsnFrameBuffer; }
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
    
    protected:
        uint8_t _mqttsnFrameBuffer[MQTTSN_MAX_PACKET_SIZE];
        Stream * _debugStream = nullptr;

    private:
        

};

#endif //__MQTTSN_CLIENT__