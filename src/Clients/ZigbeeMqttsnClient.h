#pragma once

#ifndef __ZIGBEE_MQTTSN_CLIENT__
#define __ZIGBEE_MQTTSN_CLIENT__


#include "MqttsnClient.h"

#define START_BYTE 0x7E
#define ESCAPE 0x7D
#define XON 0x11
#define XOFF 0x13

#define BROADCAST_ADDRESS_MSB   0x00000000
#define BROADCAST_ADDRESS_LSB   0x0000FFFF   

#define TRANSMIT_REQUEST_FRAME_TYPE     0x10
#define MODEM_STATUS_FRAME_TYPE         0x8A
#define TRANSMIT_STATUS_FRAME_TYPE      0x8B
#define RECEIVE_PACKET_FRAME_TYPE       0x90

#define FRAME_DATA_OFFSET 4
#define RECEIVE_PACKET_FRAME_MQTTSN_DATA_OFFSET 11

#ifndef MAX_ZIGBEE_BUFFER_SIZE
    #define MAX_ZIGBEE_BUFFER_SIZE 100
#endif

enum class DeliveryStatus : uint8_t {
    SUCCESS,
    MAC_ACK_FAILURE,
    COLLISION_AVOIDANCE_FAILURE,
    NETWORK_ACK_FAILURE = 0x21,
    ROUTE_NOT_FOUND = 0x25,
    INTERNAL_RESOURCE_ERROR = 0x31,
    INTERNAL_ERROR = 0x32
};

class ZigbeeMqttsnClient : public MqttsnClient {
    public:
        ZigbeeMqttsnClient(Stream &stream);
        void setSerial(Stream &stream);
        uint8_t * getMqttsnMsgPtr();
        bool packetAvailable();
        int readPacket();
        int sendPacket(uint8_t * buffer1, size_t bufferLength, uint8_t * buffer2, size_t buffer2Length, bool broadcast = false);
        int sendPacket(uint8_t * buffer, size_t bufferLength, bool broadcast = false);
        bool hasGatewayAddress();
        void saveGatewayAddress();
        void clearGatewayAddress();

    
    private:
        void _sendAddress(uint32_t msb, uint32_t lsb);
        void _sendByte(uint8_t b, bool escape);

        Stream * _serial = nullptr;
        uint8_t _bytePos = 0;
        bool _byteEscape = false;
        
        uint8_t _receiveBuffer[MAX_ZIGBEE_BUFFER_SIZE];
        uint16_t _frameLength;
        uint8_t _apiFrameType;
        uint8_t _checksum;
        uint8_t _frameIdCounter = 1;

        uint32_t _gatewayAddressMsb;
        uint32_t _gatewayAddressLsb;

        bool _receivePacketAvailable = false;
        bool _packetAvailable = false;
        bool _error = false;


};

#endif // __ZIGBEE_MQTTSN_CLIENT__