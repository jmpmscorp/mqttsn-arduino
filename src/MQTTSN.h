#pragma once

#ifndef __MQTTSN_H__
#define __MQTTSN_H__

#include "Clients/MqttsnClient.h"

#if defined(ZIGBEE_MQTTSN_CLIENT)
    #include "Clients/ZigbeeMqttsnClient.h"    
#endif

#include "MqttsnDebug.h"

class MQTTSN {
    
    public:
        MQTTSN(MqttsnClient &mqttsnClient);
        ~MQTTSN() {}

        bool searchGateway();
        void setDebugStream(Stream &stream);

    private:
        Stream * _debugStream;
        MqttsnClient * _mqttsnClient;
};

#endif // __MQTTSN_H__