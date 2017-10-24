#include "MQTTSNCommon.h"

boolean MQTTSNCommon::connect(const char * clientId, int keepAlive){
    uint8_t frameLength = mqttsnParser->connectFrame(clientId, keepAlive);
    uint8_t xbeeResponse = _sendPacket(frameLength);

    if(!_waitResponsePacket()) return false;

    mqttsn_msg_connack * msg = (mqttsn_msg_connack *) responseBuffer;

    //Packet received should be: LENGTH(always 3) | CONNACK(0x05) | RETURN_CODE
    if(msg->length != 3) return false;

    if(msg->type != CONNACK) return false;

    if(msg->returnCode != ACCEPTED) return false;

    return true;
}

boolean MQTTSNCommon::searchGateway(){
    uint8_t frameLength = mqttsnParser->searchGWFrame();
    uint8_t response = _sendBroadcastPacket(frameLength);

    if(!_waitResponsePacket()) return false;

    mqttsn_msg_searchgw * msg = (mqttsn_msg_searchgw *) responseBuffer;

    //Packet should be Length | GWINFO (0x02) | gatewayId
    //uint8_t length = packet->length;
    
    if( msg->type != GWINFO){
        return false;
    } else{
        _saveGatewayAddress();
    }        
       
    return true;
}

void MQTTSNCommon::setClientId(const char * clientId){
    uint8_t maxLength = strlen(clientId) > MAX_LENGTH_CLIENT_ID ? MAX_LENGTH_CLIENT_ID : strlen(clientId);

    strncpy(_clientId, clientId, maxLength);
}