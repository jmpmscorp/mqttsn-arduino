#include "MQTTSNMsg.h"

MQTTSNParser::MQTTSNParser(){

}
#pragma region Connect
uint8_t MQTTSNParser::connectFrame(const char * clientId, int keepAlive){
    mqttsn_msg_connect * msg = (mqttsn_msg_connect *) buffer;
    
    msg->type = CONNECT;
    msg->flags = 0x03; //Flags (clean session for now)
    msg->protocolId = PROTOCOL_ID;
    msg->keepAlive = _bswap(keepAlive);

    uint8_t clientIdLength = strlen(clientId);

    for (uint8_t i = 0; i < clientIdLength; i++){
        msg->clientId[i] = clientId[i];
    }

    msg->length = 6 + clientIdLength;

    return msg->length;

}
#pragma endregion Connect


uint8_t MQTTSNParser::disconnectFrame(uint16_t duration){
    mqttsn_msg_disconnect * msg = (mqttsn_msg_disconnect *) buffer;
    
    msg->type = DISCONNECT;

    if(duration > 0 ){
        msg->length = 4;
        msg->duration = _bswap(duration);
    }else  {
        msg->length = 2;
    }

    return msg->length;
}

uint8_t MQTTSNParser::pingReqFrame(const char * clientId){
    mqttsn_msg_pingreq * msg = (mqttsn_msg_pingreq *) buffer;
    
    msg->length = 2 + strlen(clientId);
    msg->type = PINGREQ;

    for(uint8_t i = 0; i < strlen(clientId); i++){
        msg->clientId[i] = clientId[i];
    }

    return msg->length;
}

uint8_t MQTTSNParser::publishFrame(const char * topic, const char * data, uint16_t nextMsgId){
    mqttsn_msg_publish * msg = (mqttsn_msg_publish *) buffer;

    msg->flags = SHORT_TOPIC_NAME; //TopicId Flag
    msg->topicName[0] = topic[0];   //Short name first character
    msg->topicName[1] = topic[1];   //Short name second character

    return publishFrameCommon(data, nextMsgId);
}

uint8_t MQTTSNParser::publishFrame(uint16_t topic, const char * data, uint16_t nextMsgId){
    mqttsn_msg_publish * msg = (mqttsn_msg_publish *) buffer;
    
    msg->flags = topic >= (0xFFFF - 1000) ? PREDEFINED_TOPIC_ID : NORMAL_TOPIC_ID; //TopicIdType Flag
    msg->topicId = _bswap(topic);
    
    return publishFrameCommon(data, nextMsgId);
}

uint8_t MQTTSNParser::publishFrameCommon(const char * data, uint16_t nextMsgId){
    mqttsn_msg_publish * msg = (mqttsn_msg_publish *) buffer;

    msg->type = PUBLISH; //MsgType
    //buffer[2..4] are built before this one
    msg->messageId = _bswap(nextMsgId);

    uint8_t dataLength = strlen(data) > (MQTTSN_MAX_PACKET_SIZE - 7) ? MQTTSN_MAX_PACKET_SIZE - 7 : strlen(data);

    for (uint8_t i = 0; i < dataLength; i++){
        msg->data[i] = data[i];
    }

    msg->length = 7 + dataLength;

    return msg->length;
}

uint8_t MQTTSNParser::searchGWFrame(){
    mqttsn_msg_searchgw * msg = (mqttsn_msg_searchgw *) buffer;

    msg->length = 0x03;
    msg->type = SEARCHGW;
    msg->radius = 0;

    return msg->length;
}

uint8_t MQTTSNParser::subscribeOrUnsubscribeFrame(const char * topic, uint16_t nextMsgId, boolean IsSubscription){
    mqttsn_msg_subOrUnsubscribe * msg = (mqttsn_msg_subOrUnsubscribe *) buffer;

    uint8_t topicLength = strlen(topic) > (MQTTSN_MAX_PACKET_SIZE - 5) ? MQTTSN_MAX_PACKET_SIZE - 5 : strlen(topic);
    
    msg->type = IsSubscription ? SUBSCRIBE : UNSUBSCRIBE;
    msg->flags = topicLength > 2 ? NORMAL_TOPIC_ID : SHORT_TOPIC_NAME; //TopicIdType Flag
    msg->messageId = _bswap(nextMsgId);

    for(uint8_t i = 0; i < topicLength; i++){
        msg->topicName[i] = topic[i];
    }

    msg->length = 5 + topicLength;

    return msg->length;
}

uint8_t MQTTSNParser::subscribeOrUnsubscribeFrame(uint16_t topic, uint16_t nextMsgId, boolean IsSubscription){
    mqttsn_msg_subOrUnsubscribe * msg = (mqttsn_msg_subOrUnsubscribe *) buffer;
    
    msg->length = 7;
    msg->type = IsSubscription ? SUBSCRIBE : UNSUBSCRIBE;
    msg->flags = PREDEFINED_TOPIC_ID;
    msg->messageId = _bswap(nextMsgId);
    msg->topicId = _bswap(topic);

    return buffer[0];
}

uint16_t MQTTSNParser::_bswap(const uint16_t val){
    return (val << 8) | (val >> 8);
}