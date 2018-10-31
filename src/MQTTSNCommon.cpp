#include "MQTTSNCommon.h"

boolean MQTTSNCommon::connect(const char * clientId, uint16_t keepAlive){
    
    if(_state == MQTTSN_ACTIVE){
        return true;
    }  
    
    uint8_t frameLength = mqttsnParser->connectFrame(clientId, keepAlive);
    
    bool success = false;
    uint8_t retries = 0;
    do {
        uint8_t result = _sendPacket(frameLength);
        debugPrintLn(F("==CONNECT=="));
        if(_waitResponsePacket()){ 

            if(_handleConnack()){
                setClientId(clientId);
                _setKeepAlive(keepAlive);
                
                success = true;
            } else {
                retries ++;
            }
            
        } else {
            debugPrintLn(F("==CONNECT==\n> NO RESPONSE"));
            retries ++;
        }
    }while( !success && retries < N_RETRY);
    
    if(!success) _state = MQTTSN_DISCONNECTED;
    
    return success;
}

void MQTTSNCommon::disconnect(uint16_t duration){
    if(_state == MQTTSN_ACTIVE){
        debugPrintLn(F("==DISCONNECT=="));
        uint8_t frameLength = mqttsnParser->disconnectFrame(duration);
        uint8_t result = _sendPacket(frameLength);
    }
    if(duration > 0){
        if(_state == MQTTSN_ACTIVE || _state == MQTTSN_AWAKE){
            _state = MQTTSN_ASLEEP;
        }           
    } else{
        _state = MQTTSN_DISCONNECTED;
    }
    
}

bool MQTTSNCommon::pingReq(){
    debugPrintLn(F("==PINGREQ=="));
    uint8_t frameLength = mqttsnParser->pingReqFrame(_clientId);
    uint8_t result = _sendPacket(frameLength);

    if(_state == MQTTSN_ACTIVE){
        _pingRespRetries++;
        if(!_waitResponsePacket()){
            debugPrintLn(F("> NO RESPONSE"));
             return false;
        }
        
        mqttsn_msg_header * msg = (mqttsn_msg_header *) responseBuffer;
    
        if(msg->length != 2) return false;
    
        if(msg->type != PINGRESP) {
            debugPrintLn(F("> NO PINGRESP HEADER"));
            return false;
        }
        _pingRespRetries = 0;
        return true;
    }

    return true;   
}

void MQTTSNCommon::pingResp(){
    uint8_t frameLength = mqttsnParser->pingRespFrame();
    uint8_t result = _sendPacket(frameLength);
}

void MQTTSNCommon::pubAck(uint16_t topicId, uint16_t msgId, uint8_t returnCode){
    uint8_t frameLength = mqttsnParser->pubAckFrame(topicId, msgId, returnCode);
    uint8_t result = _sendPacket(frameLength);
}

bool MQTTSNCommon::publish(uint16_t topic, bool predefined, bool retain, const char * data, uint8_t qos){
    if(_state != MQTTSN_ACTIVE){
        debugPrintLn(F("==PUBLISH (NO ACTIVE)=="));
        return false;
    }

    if(_state == MQTTSN_AWAKE) {
        if(!connect(_clientId)) {
            return false;
        }
    }    
    uint8_t frameLength = mqttsnParser->publishFrame(topic, predefined, retain, data, nextMsgId, qos);
    
    return _publishCommon(frameLength, qos);
}


bool MQTTSNCommon::publish(const char * topic, bool retain, const char * data, uint8_t qos){
    if(_state != MQTTSN_ACTIVE){
        debugPrintLn(F("==PUBLISH (NO ACTIVE)=="));
        return false;
    }

    if(strlen(topic) > 2) return false;

    if(_state == MQTTSN_AWAKE) {
        if(!connect(_clientId)) {
            return false;
        }
    }
    
    uint8_t frameLength = mqttsnParser->publishFrame(topic,retain, data, nextMsgId, qos);
    return _publishCommon(frameLength, qos);
}


bool MQTTSNCommon::_publishCommon(uint8_t frameLength, uint8_t qos){
    uint8_t result = _sendPacket(frameLength);

    debugPrintLn(F("==PUBLISH=="));

    uint16_t lastMsgId = nextMsgId;
    _incrementNextMsgId();
        
    if(qos == 0) return true;

    if(!_waitResponsePacket()){
        debugPrintLn(F(">NO RESPONSE"));
        return false;
    } 

    return _handlePuback(lastMsgId);
    
}

void MQTTSNCommon::sleep(uint16_t duration){
    disconnect(duration);
}

bool MQTTSNCommon::subscribe(const char * topic, uint16_t * topicIdOut){
    if(_state != MQTTSN_ACTIVE){
        debugPrintLn(F("==SUBSCRIBE (NO ACTIVE)=="));
        return false;
    }

    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, true);
    return _subscribeCommon(frameLength, topicIdOut);
}

bool MQTTSNCommon::subscribe(uint16_t topic, uint16_t * topicIdOut){
    if(_state != MQTTSN_ACTIVE){
        debugPrintLn(F("==SUBSCRIBE (NO ACTIVE)=="));
        return false;
    }
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, true);
    return _subscribeCommon(frameLength, topicIdOut);
}

bool MQTTSNCommon::_subscribeCommon(uint8_t frameLength, uint16_t * topicIdOut){
    
    uint8_t result = _sendPacket(frameLength);
    uint8_t lastMsgId = nextMsgId;
    _incrementNextMsgId();
    
    debugPrintLn(F("==SUBSCRIBE=="));
    if(!_waitResponsePacket()){
        debugPrintLn(F("> NO RESPONSE"));
        return false;
    } 
    
    mqttsn_msg_suback * msg = (mqttsn_msg_suback *) responseBuffer;

    //if(msg->type != SUBACK)
    if(msg->type != SUBACK){
        //debugPrintLn(msg->type);
        debugPrintLn(F("> NO SUBACK HEADER"));
        return false;
    } 

    //if(msg->returnCode != ACCEPTED)
    if(msg->returnCode != ACCEPTED){
        //debugPrintLn(msg->returnCode);
        debugPrintLn(F("> SUBSCRIBE REJECTED"));
        return false;
    } 

    if(mqttsnParser->_bswap(msg->messageId) != lastMsgId){
        debugPrintLn(F("> BAD MSG ID"));
        return false;
    } 

    *topicIdOut = mqttsnParser->_bswap( msg->topicId );
    return true;
}

bool MQTTSNCommon::unsubscribe(const char * topic){
    if(strlen(topic) > 2) return false;

    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, false);

    return _unsubscribeCommon(frameLength);
}

bool MQTTSNCommon::unsubscribe(uint16_t topic){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, false);

    return _unsubscribeCommon(frameLength);    
}

bool MQTTSNCommon::_unsubscribeCommon(uint8_t frameLength){
    debugPrintLn(F("==UNSUBSCRIBE=="));
    uint8_t result = _sendPacket(frameLength);
    uint8_t lastMsgId = nextMsgId;
    _incrementNextMsgId();
    
    if(!_waitResponsePacket()) return false;

    mqttsn_msg_unsuback * msg = (mqttsn_msg_unsuback *) responseBuffer;
    
    if(msg->type != UNSUBACK){
        debugPrintLn(msg->type);
        debugPrintLn(F("> NO UNSUBACK HEADER"));
        return false;
    } 

    if(mqttsnParser->_bswap(msg->messageId) != lastMsgId){
        debugPrintLn(F("> BAD MSG ID"));
        return false;
    }

    return true;
}

bool MQTTSNCommon::awake(){
    if(_state == MQTTSN_ASLEEP){
        _state = MQTTSN_AWAKE;
        _waitingPingResp = true;


        //I don't know how many time since lastReceived so ¡Reset It!
        _lastReceived = millis();
        return pingReq();
    }
    
    return false;
}

uint8_t MQTTSNCommon::getState(){
    return _state;
}

void MQTTSNCommon::setState(uint8_t state){
    _state = state;
}

unsigned long MQTTSNCommon::getLastReceived(){
    return _lastReceived;
}

bool MQTTSNCommon::isWaitingPingResp(){
    return _waitingPingResp;
}

bool MQTTSNCommon::searchGateway(){
    _retries = 0;

    uint8_t frameLength = mqttsnParser->searchGWFrame();

    do{        
        debugPrintLn(F("==SEARCHGW=="));
        uint8_t result = _sendBroadcastPacket(frameLength);
        
        if(_waitResponsePacket( T_RETRY * 1000)){
            mqttsn_msg_searchgw * msg = (mqttsn_msg_searchgw *) responseBuffer;
            
            if( msg->type == GWINFO){
                _saveGatewayAddress();
                return true;
            }  
        } else{
            debugPrintLn("> NO RESPONSE");
        }
        
        _retries ++ ;       
    }while(_retries < N_RETRY);

    _retries = 0;

    return false;
}

bool MQTTSNCommon::searchGwAndConnect(const char * clientId, uint16_t keepAlive){
    if(searchGateway()){
        return connect(clientId, keepAlive);
    }

    return false;
}

uint8_t MQTTSNCommon::searchGwAndConnectAsync(const char * clientId, uint16_t keepAlive){
    static uint8_t stablishConnectionState = SEARCHING_GW;
    uint8_t returnCode = MQTTSN_NO_ERROR;
    unsigned long now = millis();
    
    if(_retries == 0 || now > _lastSent + T_SEARCHGW * 1000){
        uint8_t frameLength;
        uint8_t result;
        if(stablishConnectionState == SEARCHING_GW){
            frameLength = mqttsnParser->searchGWFrame();
            debugPrintLn(F("==SEARCHGW=="));
            result = _sendBroadcastPacket(frameLength);
        } else if(stablishConnectionState == TRYING_CONNECT){
            frameLength = mqttsnParser->connectFrame(clientId, keepAlive);
            debugPrintLn(F("==CONNECT=="));
            result = _sendPacket(frameLength);
        }

        _retries ++;

        if(_retries > N_RETRY){
            returnCode = MQTTSN_MAX_RETRIES_REACH;
        }
    }
    
    if(_continuosWait()){
        mqttsn_msg_header * msgHeader = (mqttsn_msg_header *) responseBuffer;

        if(stablishConnectionState == SEARCHING_GW && msgHeader->type == GWINFO){
            _saveGatewayAddress();
            stablishConnectionState = TRYING_CONNECT;
            _retries = 0;
        } 
        else if(stablishConnectionState == TRYING_CONNECT && msgHeader->type == CONNACK){
            if(_handleConnack()){
                //Next time I want to connect, I should start from SearchGW
                setClientId(clientId);
                _keepAlive = keepAlive;
                stablishConnectionState = SEARCHING_GW;
            }
            _retries = 0;
        }
    }

    

    return returnCode;
}

uint8_t MQTTSNCommon::continuosAsyncTask(){
    if(_continuosWait()){
        mqttsn_msg_header * msgHeader = (mqttsn_msg_header *) responseBuffer;

        if(msgHeader->type == PUBLISH){
            mqttsn_msg_publish * msg = (mqttsn_msg_publish *) responseBuffer;
            //uint8_t flags = msg->flags;
            uint8_t topicIdType = msg->flags & 0x03;
            uint8_t qos = (msg->flags & 0x60);
            bool retain = (msg->flags & 0x10) >> 4;            
            uint8_t dataLength = msg->length - 7;
            uint16_t msgId = mqttsnParser->_bswap(msg->messageId);
            for(int i = 0; i < dataLength; i++){
               _onDataBuffer[i] = (char )responseBuffer[7 + i];
            }

            _onDataBuffer[dataLength] = '\0';
            debugPrintLn(msgId);
            if( (topicIdType == PREDEFINED_TOPIC_ID || topicIdType == NORMAL_TOPIC_ID) && _onTopicMsgCallback != nullptr){
                (*_onTopicMsgCallback)(mqttsnParser->_bswap(msg->topicId), topicIdType, _onDataBuffer, dataLength, retain);
                //pubAck(msg->topicId, msg->messageId);
            } else if(topicIdType == SHORT_TOPIC_NAME && _onShortTopicCallback != nullptr){
                (*_onShortTopicCallback)(msg->topicName, _onDataBuffer, dataLength, retain);
                //pubAck(0, msg->messageId);
            }

            debugPrint(F("QoS: "));
            debugPrintLn(qos);
            if(qos > 0) {
                pubAck( topicIdType == SHORT_TOPIC_NAME ? 0 : msg->topicId, msg->messageId);
            }
        }
        else if( msgHeader->type == PINGREQ){
            _lastSent = millis();
            pingResp();
        }
        else if ( msgHeader->type == DISCONNECT ){
            if(_onDisconnectCallback != nullptr){
                (*_onDisconnectCallback)();
            }
            _state = MQTTSN_DISCONNECTED;
        }
        else if( msgHeader->type == PINGRESP){
            if( _state == MQTTSN_AWAKE){
                _pingRespRetries = 0;
                _waitingPingResp = false;
            }
        }

        _lastReceived = millis();
    }

    unsigned long now = millis();

    if(_state == MQTTSN_ACTIVE && now > _lastSent + (T_RETRY * 1000)){
        pingReq();
        if(_pingRespRetries >= N_RETRY) {
            _setLost();
        }
    }

    if(_waitingPingResp && now > _lastReceived + (T_PINGRESP * 1000)){
        _setLost();
        return MQTTSN_PINGRESP_TIMEOUT;
    }

    return MQTTSN_NO_ERROR;
}

void MQTTSNCommon::setTopicMsgCallback(void(*f)(uint16_t topicId, uint8_t topicIdType, const char * data, uint16_t dataLength, bool retained)){
    _onTopicMsgCallback = f;
}

void MQTTSNCommon::setShortTopicCallback(void(*f)(const char * topicName, const char * data, uint16_t dataLength, bool retained)){
    _onShortTopicCallback = f;
}

void MQTTSNCommon::setOnDisconnectCallback(void (*f)()){
    _onDisconnectCallback = f;
}

void MQTTSNCommon::setClientId(const char * clientId){
    uint8_t maxLength = strlen(clientId) > MAX_LENGTH_CLIENT_ID ? MAX_LENGTH_CLIENT_ID : strlen(clientId);

    strncpy(_clientId, clientId, maxLength);
}

void MQTTSNCommon::setWdtChecker( void (*f)()) {
    _wdtChecker = f;
}


boolean MQTTSNCommon::_handleConnack(){
    mqttsn_msg_connack * msg = (mqttsn_msg_connack *) responseBuffer;

    //Packet received should be: LENGTH(always 3) | CONNACK(0x05) | RETURN_CODE
    if(msg->length != 3){
        debugPrintLn(F("\tBAD MSG LENGTH"));
        return false;
    } 
    
    if(msg->type != CONNACK) {
        debugPrintLn(F("\tNO CONNACK HEADER"));
        return false;
    }
    
    if(msg->returnCode != ACCEPTED){
        debugPrintLn(F("\tREJECTED CONNECT"));
        return false;
    } 

    nextMsgId = 1;
    _state = MQTTSN_ACTIVE;
    _waitingPingResp = false;
    _pingRespRetries = 0;
    return true;
}

boolean MQTTSNCommon::_handlePuback(uint16_t lastMsgId){
    mqttsn_msg_puback * msg = (mqttsn_msg_puback *) responseBuffer;
    
    if(msg->type != PUBACK){
        debugPrintLn(F("\tNO PUBACK HEADER"));
        return false;
    } 
    
    if(msg->returnCode != ACCEPTED) {
        debugPrintLn(F("> PUBLISH REJECTED"));
        return false;
    }
    
    if(mqttsnParser->_bswap(msg->messageId) != lastMsgId) return false;
    
    return true;
}

void MQTTSNCommon::_setKeepAlive(uint16_t keepAlive){
    _keepAlive = keepAlive;
}

void MQTTSNCommon::_setLost(){
    if(*_onDisconnectCallback != nullptr){
        (*_onDisconnectCallback)();
    }

    debugPrintLn(F("> GW LOST"));
    _state = MQTTSN_LOST;
}