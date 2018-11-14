#pragma once

#ifndef __MQTTSN_CLIENT__
#define __MQTTSN_CLIENT__

#include "Stream.h"
#include "inttypes.h"

#define CUSTOM_STREAM_DEBUG
#include "../MqttsnDebug.h"

class MqttsnClient {
    public: 
        virtual bool packetAvailable() = 0;
        // virtual int readPacket(int waitMilliseconds);
        virtual uint8_t * getMqttsnMsgPtr() = 0;
        virtual int readPacket() = 0;
        virtual int sendPacket(uint8_t * buffer1, size_t bufferLength, uint8_t * buffer2, size_t buffer2Length, bool broadcast = false) = 0;
        virtual int sendPacket(uint8_t * buffer, size_t bufferLength, bool broadcast = false) = 0;
        virtual void saveGatewayAddress() = 0;

        virtual void setSerial(Stream &stream) {};
        
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
    
    protected:
        Stream * _debugStream = nullptr;

    private:
        

};

#endif //__MQTTSN_CLIENT__