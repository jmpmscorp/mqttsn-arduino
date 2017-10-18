#include "MQTTSN.h"


#pragma region Connect
uint8_t MQTTSN::buildConnectFrame(const char * clientId, int keepAlive){
    
    buffer[1] = CONNECT; //MsgType
    buffer[2] = 0x03; //Flags (clean session for now)
    buffer[3] = PROTOCOL_ID;
    buffer[4] = keepAlive / 256;  //keepAlive MSB
    buffer[5] = keepAlive % 256;  //keepAlive LSB

    setClientId(clientId);
    uint8_t clientIdLength = strlen(_clientId);

    for (uint8_t i = 0; i < clientIdLength; i++){
        buffer[6 + i] = clientId[i];
    }

    buffer[0] = 6 + clientIdLength;

    return buffer[0];

}
#pragma endregion Connect

uint8_t MQTTSN::buildDisconnectFrame(uint16_t duration){
    buffer[1] = DISCONNECT;  //MsgType
    
    if(duration > 0 ){
        buffer[0] = 4;      //Length
        buffer[2] = duration / 256;
        buffer[3] = duration % 256;
    }else  {
        buffer[0] = 2;
    }

    return buffer[0];
}

uint8_t MQTTSN::buildPingReqFrame(){
    buffer[0] = 2 + strlen(_clientId);
    buffer[1] = PINGREQ;

    for(uint8_t i = 0; i < strlen(_clientId); i++){
        buffer[2 + i] = _clientId[i];
    }

    return buffer[0];
}

uint8_t MQTTSN::buildPublishFrame(const char * topic, const char * data){
    buffer[2] = SHORT_TOPIC_NAME; //TopicId Flag
    buffer[3] = topic[0];   //Short name first character
    buffer[4] = topic[1];   //Short name second character

    return buildPublishFrameCommon(data);
}

uint8_t MQTTSN::buildPublishFrame(uint16_t topic, const char * data){
    buffer[2] = PREDEFINED_TOPIC_ID; //TopicIdType Flag
    buffer[3] = topic / 256; //MSB topic
    buffer[4] = topic % 256; //LSB Topic

    return buildPublishFrameCommon(data);
}

uint8_t MQTTSN::buildPublishFrameCommon(const char * data){
    buffer[1] = PUBLISH; //MsgType
    //buffer[2..4] are built before this one
    buffer[5] = nextMsgId / 256;
    buffer[6] = nextMsgId % 256;

    uint8_t dataLength = strlen(data) > (MQTTSN_MAX_PACKET_SIZE - 7) ? MQTTSN_MAX_PACKET_SIZE - 7 : strlen(data);

    for (uint8_t i = 0; i < dataLength; i++){
        buffer[7 + i] = data[i];
    }

    buffer[0] = 7 + dataLength;

    return buffer[0];
}

uint8_t MQTTSN::buildSearchGWFrame(){
    this->buffer[0] = 0x03; //Length Byte
    this->buffer[1] = SEARCHGW; //MsgType
    this->buffer[2] = 0x00; //Radius     
    
    return this->buffer[0];
}

uint8_t MQTTSN::buildSubscribeOrUnsubscribeFrame(const char * topic, boolean IsSubscription){
    uint8_t topicLength = strlen(topic) > (MQTTSN_MAX_PACKET_SIZE - 5) ? MQTTSN_MAX_PACKET_SIZE - 5 : strlen(topic);
    
    buffer[1] = IsSubscription ? SUBSCRIBE : UNSUBSCRIBE;
    buffer[2] = topicLength > 2 ? NORMAL_TOPIC_ID : SHORT_TOPIC_NAME; //TopicIdType Flag
    buffer[3] = nextMsgId / 256;
    buffer[4] = nextMsgId % 256;
    for(uint8_t i = 0; i < topicLength; i++){
        buffer[5 + i] = topic[i];
    }

    buffer[0] = 5 + topicLength;

    return buffer[0];
}

uint8_t MQTTSN::buildSubscribeOrUnsubscribeFrame(uint16_t topic, boolean IsSubscription){
    buffer[0] = 7;
    buffer[1] = IsSubscription ? SUBSCRIBE : UNSUBSCRIBE;
    buffer[2] = PREDEFINED_TOPIC_ID;
    buffer[3] = nextMsgId / 256;
    buffer[4] = nextMsgId % 256;
    buffer[5] = topic / 256;
    buffer[6] = topic % 256;

    return buffer[0];
}

void MQTTSN::setClientId(const char * clientId){
    uint8_t maxLength = strlen(clientId) > MAX_LENGTH_CLIENT_ID ? MAX_LENGTH_CLIENT_ID : strlen(clientId);

    strncpy(_clientId, clientId, maxLength);
}