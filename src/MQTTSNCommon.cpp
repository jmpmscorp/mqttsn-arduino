#include "MQTTSNCommon.h"

boolean MQTTSNCommon::connect(const char * clientId, int keepAlive){
    uint8_t frameLength = mqttsnParser->connectFrame(clientId, keepAlive);
    uint8_t result = _sendPacket(frameLength);

    if(!_waitResponsePacket()) return false;

    mqttsn_msg_connack * msg = (mqttsn_msg_connack *) responseBuffer;

    //Packet received should be: LENGTH(always 3) | CONNACK(0x05) | RETURN_CODE
    if(msg->length != 3) return false;

    if(msg->type != CONNACK) return false;

    if(msg->returnCode != ACCEPTED) return false;

    nextMsgId = 1;
    setClientId(clientId);

    _state = MQTTSN_ACTIVE;

    return true;
}

void MQTTSNCommon::disconnect(uint16_t duration){
    uint8_t frameLength = mqttsnParser->disconnectFrame(duration);
    uint8_t result = _sendPacket(frameLength);

    if(duration > 0){
        _state = MQTTSN_ASLEEP;
    } else{
        _state = MQTTSN_DISCONNECTED;
    }
    
}

boolean MQTTSNCommon::pingReq(){
    debugPrintLn("SENDING PINGREQ");
    uint8_t frameLength = mqttsnParser->pingReqFrame(_clientId);
    uint8_t result = _sendPacket(frameLength);

    if(_state == MQTTSN_ACTIVE){
        if(!_waitResponsePacket()) return false;
        
        mqttsn_msg_header * msg = (mqttsn_msg_header *) responseBuffer;
    
        if(msg->length != 2) return false;
    
        if(msg->type != PINGRESP) return false;
        
        debugPrintLn("RECEIVE PINGRESP")
        return true;
    }

    return true;   
}

void MQTTSNCommon::pingResp(){
    uint8_t frameLength = mqttsnParser->pingRespFrame();
    uint8_t result = _sendPacket(frameLength);
}

void MQTTSNCommon::pubAck(uint8_t topicId, uint8_t msgId, uint8_t returnCode){
    uint8_t frameLength = mqttsnParser->pubAckFrame(topicId, msgId, returnCode);
    uint8_t result = _sendPacket(frameLength);
}

boolean MQTTSNCommon::publish(uint16_t topic, boolean predefined, boolean retain, const char * data, uint8_t qos){
    uint8_t frameLength = mqttsnParser->publishFrame(topic, predefined, retain, data, nextMsgId, qos);
    //Serial.println("Hola");
    
    return _publishCommon(frameLength, qos);
}

boolean MQTTSNCommon::publish(const char * topic, boolean retain, const char * data, uint8_t qos){
    uint8_t frameLength = mqttsnParser->publishFrame(topic,retain, data, nextMsgId, qos);
    return _publishCommon(frameLength, qos);
}

boolean MQTTSNCommon::_publishCommon(uint8_t frameLength, uint8_t qos){
    uint8_t result = _sendPacket(frameLength);
    debugPrintLn(F("==PUBLISH=="));
    
    uint8_t lastMsgId = nextMsgId;
    _incrementNextMsgId();
    
    if(qos == 0) return true;

    if(!_waitResponsePacket()){
        debugPrintLn(F("\tNO RESPONSE"));
        return false;
    } 

    mqttsn_msg_puback * msg = (mqttsn_msg_puback *) responseBuffer;
    
    if(msg->type != PUBACK) return false;
    
    if(msg->returnCode != ACCEPTED) return false;
    
    if(mqttsnParser->_bswap(msg->messageId) != lastMsgId) return false;
    
    return true;
}

void MQTTSNCommon::sleep(uint16_t duration){
    disconnect(duration);
}

boolean MQTTSNCommon::subscribe(const char * topic, uint16_t * topicIdOut){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, true);
    return _subscribeCommon(frameLength, topicIdOut);
}

boolean MQTTSNCommon::subscribe(uint16_t topic, uint16_t * topicIdOut){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, true);
    return _subscribeCommon(frameLength, topicIdOut);
}

boolean MQTTSNCommon::_subscribeCommon(uint8_t frameLength, uint16_t * topicIdOut){
    debugPrintLn(F("==PUBLISH=="));
    uint8_t result = _sendPacket(frameLength);
    uint8_t lastMsgId = nextMsgId;
    _incrementNextMsgId();
    
    if(!_waitResponsePacket()) return false;
    
    mqttsn_msg_suback * msg = (mqttsn_msg_suback *) responseBuffer;

    //if(msg->type != SUBACK)
    if(responseBuffer[1] != SUBACK){
        debugPrintLn(msg->type);
        debugPrintLn(F("\tNO SUBACK HEADER"));
        return false;
    } 

    //if(msg->returnCode != ACCEPTED)
    if(responseBuffer[7] != ACCEPTED){
        debugPrintLn(msg->returnCode);
        debugPrintLn(F("\tPUBLISH REJECTED"));
        return false;
    } 

    if(mqttsnParser->_bswap(msg->messageId) != lastMsgId){
        debugPrintLn(F("\tBAD MSG ID"));
        return false;
    } 

    *topicIdOut = mqttsnParser->_bswap( msg->topicId );
    return true;
}

boolean MQTTSNCommon::awake(){
    _state = MQTTSN_AWAKE;
    return pingReq();
}

uint8_t MQTTSNCommon::getState(){
    return _state;
}

boolean MQTTSNCommon::searchGateway(){
    uint8_t retries = 0;

    uint8_t frameLength = mqttsnParser->searchGWFrame();

    do{        
        debugPrintLn(F("==SEARCHGW=="));
        uint8_t result = _sendBroadcastPacket(frameLength);
    
        if(_waitResponsePacket( T_SEARCHGW * 1000)){
            mqttsn_msg_searchgw * msg = (mqttsn_msg_searchgw *) responseBuffer;

            if( msg->type == GWINFO){
                _saveGatewayAddress();
                return true;
            }  
        }
        
        retries ++ ;       
    }while(retries < N_RETRY);
       
    return false;
}

uint8_t MQTTSNCommon::loopTask(){
    if(_continuosWait()){
        mqttsn_msg_header * msgHeader = (mqttsn_msg_header *) responseBuffer;

        if(msgHeader->type == PUBLISH){
            mqttsn_msg_publish * msg = (mqttsn_msg_publish *) responseBuffer;
            //uint8_t flags = msg->flags;
            uint8_t topicIdType = msg->flags & 0x03;
                        
            uint8_t dataLength = msg->length - 7;
            
            for(int i = 0; i < dataLength; i++){
               _onDataBuffer[i] = (char )responseBuffer[7 + i];
            }

            _onDataBuffer[dataLength] = '\0';

            if( (topicIdType == PREDEFINED_TOPIC_ID || topicIdType == NORMAL_TOPIC_ID) && _onTopicMsgCallback != NULL){
                (*_onTopicMsgCallback)(mqttsnParser->_bswap(msg->topicId), topicIdType, _onDataBuffer, dataLength);
                pubAck(msg->topicId, msg->messageId);
            } else if(topicIdType == SHORT_TOPIC_NAME && _onShortTopicCallback != NULL){
                (*_onShortTopicCallback)(msg->topicName, _onDataBuffer, dataLength);
                pubAck(0, msg->messageId);
            }

            return PUBLISH;
        }
        else if( msgHeader->type == PINGREQ){
            _lastSent = millis();
            pingResp();

            return PINGREQ;
        }
        else if( msgHeader->type == PINGRESP && _state == MQTTSN_AWAKE){
            return PINGRESP;
        }
    }

    unsigned long now = millis();

    if(_state == MQTTSN_ACTIVE && now > _lastSent + 10000){
        pingReq();
    }

    return 0xFF;
}

void MQTTSNCommon::setTopicMsgCallback(void(*f)(uint16_t topicId, uint16_t topicIdType, const char * data, uint16_t dataLength)){
    _onTopicMsgCallback = f;
}

void MQTTSNCommon::setShortTopicCallback(void(*f)(const char * topicName, const char * data, uint16_t dataLength)){
    _onShortTopicCallback = f;
}

boolean MQTTSNCommon::searchGwAndConnect(const char * clientId, int keepAlive){
    if(searchGateway()){
        return connect(clientId, keepAlive);
    }

    return false;
}
void MQTTSNCommon::setClientId(const char * clientId){
    uint8_t maxLength = strlen(clientId) > MAX_LENGTH_CLIENT_ID ? MAX_LENGTH_CLIENT_ID : strlen(clientId);

    strncpy(_clientId, clientId, maxLength);
}