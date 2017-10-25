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

    nextMsgId = 1;

    return true;
}

boolean MQTTSNCommon::publish(uint16_t topic, boolean predefined, const char * data, uint8_t qos){
    uint8_t frameLength = mqttsnParser->publishFrame(topic, predefined, data, nextMsgId, qos);
    return _publishCommon(frameLength);
}

/*boolean MQTTSNCommon::publish(const char * topic, const char * data){
    uint8_t frameLength = mqttsnParser->publishFrame(topic, data, nextMsgId);
    return _publishCommon(frameLength);
}*/

boolean MQTTSNCommon::_publishCommon(uint8_t frameLength){
    uint8_t xbeeResponse = _sendPacket(frameLength);
    
    nextMsgId ++;

    if(!_waitResponsePacket()) return false;

    mqttsn_msg_puback * msg = (mqttsn_msg_puback *) responseBuffer;

    if(msg->type != PUBACK) return false;

    if(msg->returnCode != ACCEPTED) return false;
    
    if(mqttsnParser->_bswap(msg->messageId) != nextMsgId - 1) return false;

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