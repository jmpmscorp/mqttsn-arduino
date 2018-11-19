#include "Mqttsn.h"

//#include "MQTTSN_Msg.h"

Mqttsn::Mqttsn(MqttsnClient &mqttsnClient) : _mqttsnClient(&mqttsnClient) {
    _mqttsnMsg.setInputBuffer(_mqttsnClient->getMqttsnMsgPtr());
}

void Mqttsn::_sendSearchGw() {
    SearchGwMqttsnMsg &msg = static_cast<SearchGwMqttsnMsg &>(_mqttsnMsg);    
    msg.build(0);

    _sendMsg(msg.getOutputBuffer(), msg.getOutputBuffer()[0], true);
}

int Mqttsn::searchGateway() {
    _sendSearchGw();

    if(_waitMsg(T_SEARCH_GW * 1000)) {
        return _handleGwInfo();
    }

    return static_cast<int>(MqttsnOperationError::TIMEOUT);    
}


int Mqttsn::_handleGwInfo() {
    GwInfoMqttsnMsg &msg = static_cast<GwInfoMqttsnMsg &>(_mqttsnMsg);
    if(msg.getMsgType() == GWINFO){
        _mqttsnClient->saveGatewayAddress();
        return static_cast<int>(MqttsnOperationError::NO_ERROR);
    }

 
    return static_cast<int>(MqttsnOperationError::NO_EXPECTED_MSG_TYPE);
}

void Mqttsn::_sendConnect(const char * clientId, bool cleanSession, uint16_t keepAlive) {
    ConnectMqttsnMsg &msg = static_cast<ConnectMqttsnMsg &>(_mqttsnMsg);
    uint8_t fixedLength = 6;
    msg.build(fixedLength + strlen(clientId), 
                    _computeFlags(false, MQTTSN_QoS::QoS0, false, _willTopic && _willMsg ? true : false, cleanSession, TOPIC_ID_TYPE::NORMAL_TOPIC ), 
                    keepAlive);
   
    _sendMsg(msg.getOutputBuffer(), fixedLength, reinterpret_cast<uint8_t *>(const_cast<char *>(clientId)), strlen(clientId));
    
}

int Mqttsn::connect(const char * clientId, bool cleanSession, uint16_t keepAlive) {
    
    if(_autoreconnect) {
        strncpy(_clientId, clientId, strlen(clientId) <= MAX_LENGTH_CLIENT_ID ? strlen(clientId) : MAX_LENGTH_CLIENT_ID );
        DEBUGLN(_clientId);
        _keepAlive = keepAlive;
    }   
    
    if(!_mqttsnClient->hasGatewayAddress()) {
        uint8_t result = searchGateway();
        if(result > 0) return result;
    }

    _sendConnect(clientId, cleanSession, keepAlive);
    
    

    if(_waitMsg(2000)) {
        return _handleConnack();
    }

    return static_cast<int>(MqttsnOperationError::TIMEOUT);  
}

void Mqttsn::setAutoreconnect(bool autoreconnect) {
    _autoreconnect = autoreconnect;
} 

int Mqttsn::_handleConnack() {
    ConnackMqttsnMsg &msg = static_cast<ConnackMqttsnMsg &>(_mqttsnMsg);

    if(msg.getMsgType() != CONNACK) {
        return static_cast<int>(MqttsnOperationError::NO_EXPECTED_MSG_TYPE);
    }

    if(msg.getReturnCode() == ReturnCode::ACCEPTED) {
        _state = MqttsnState::ACTIVE;
    }

    return static_cast<int>(msg.getReturnCode());
}

void Mqttsn::setWillOptions(const char * willTopic, const char * willMsg, bool retain , MQTTSN_QoS qos) {
    _willTopic = willTopic;
    _willMsg = willMsg;
    _willFlags = _computeFlags(0, qos, retain, 0, 0, static_cast<TOPIC_ID_TYPE>(0));
}

int Mqttsn::_handleWillTopicReq() {

}

int Mqttsn::_handleWillMsgReq() {

}



uint8_t Mqttsn::_computeFlags(bool dup, MQTTSN_QoS qos, bool retain, bool will, bool cleanSession, TOPIC_ID_TYPE topicIdType) {
    return (dup << 7) + (static_cast<uint8_t>(qos) << 5) + (retain << 4) + (will << 3) + (cleanSession << 2) + (static_cast<uint8_t>(topicIdType));
}

void Mqttsn::_sendMsg(uint8_t * fixedMsgSection, uint8_t fixedMsgSectionLength, bool broadcast ) {
    _lastMsgSentMillis = millis();
    _mqttsnClient->sendPacket(fixedMsgSection, fixedMsgSectionLength, NULL, 0, broadcast);
}

void Mqttsn::_sendMsg(uint8_t * fixedMsgSection, uint8_t fixedMsgSectionLength, 
                uint8_t * variableMsgSection, uint8_t variableMsgSectionLength, 
                bool broadcast) {
    
    _lastMsgSentMillis = millis();
    _mqttsnClient->sendPacket(fixedMsgSection, fixedMsgSectionLength, variableMsgSection, variableMsgSectionLength, broadcast);
}
    

bool Mqttsn::_waitMsg(int milliseconds) {
    unsigned long now = millis();

    do {
        _mqttsnClient->readPacket();
    }while(!_mqttsnClient->packetAvailable() && millis() - now < milliseconds);

    if(_mqttsnClient->packetAvailable()) {
        _lastMsgReceiveMillis = millis();
        return true;
    } else {
        return false;
    }
}

void Mqttsn::loop() {
    if ( _state == MqttsnState::ACTIVE) {
        _activeLoop();
    }
    else if(_autoreconnect && (_state == MqttsnState::DISCONNECTED || _state == MqttsnState::LOST)) {
        _disconnectedLoop();
    }
    else if ( _state == MqttsnState::AWAKE) {
        _awakeLoop();
    }    
}

void Mqttsn::_activeLoop() {
    // DEBUGLN("DISCONNECTED LOOP");
}


void Mqttsn::_disconnectedLoop() {
    // DEBUGLN("DISCONNECTED LOOP");
    if( !_mqttsnClient->hasGatewayAddress() && millis() - _lastMsgSentMillis > T_SEARCH_GW * 1000) {
        _sendSearchGw();
    } else if ( _mqttsnClient->hasGatewayAddress() && millis() - _lastMsgSentMillis > T_RETRY * 1000){
        
        DEBUGLN("SEND CONNECT");
        _sendConnect(_clientId, true, _keepAlive);
        
        if( ++_retries > N_RETRY) {
            _retries = 0;
            _mqttsnClient->clearGatewayAddress();
        }
    }
    
    if(_waitMsg(0)) {
        switch(_mqttsnMsg.getMsgType()) {
            case GWINFO:
                _handleGwInfo();
                break;

            case CONNACK:
                _handleConnack();
                break;

            case WILLTOPICREQ:
                _handleWillTopicReq();
                break;
            
            case WILLMSGREQ:
                _handleWillMsgReq();
                break;

            default:
                break;
        }
    }
}


void Mqttsn::_awakeLoop() {

}


void Mqttsn::setDebugStream(Stream & stream) {
    _debugStream = &stream;

    if(_mqttsnClient != nullptr) {
        _mqttsnClient->setDebugStream(stream);
    }
}