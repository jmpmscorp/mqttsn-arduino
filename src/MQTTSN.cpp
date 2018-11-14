#include "Mqttsn.h"

//#include "MQTTSN_Msg.h"

Mqttsn::Mqttsn(MqttsnClient &mqttsnClient) : _mqttsnClient(&mqttsnClient){

}

bool Mqttsn::searchGateway() {
    uint8_t buf[3];  
    _mqttsnMsg.setParseBuffer(buf);    
    SearchGwMqttsnMsg &searchGw = static_cast<SearchGwMqttsnMsg &>(_mqttsnMsg);    
    searchGw.build(0);

    _mqttsnClient->sendPacket(buf, buf[0], true);

    if(_waitResponse(T_SEARCH_GW * 1000) && _mqttsnClient->getMqttsnMsgPtr()) {
        _mqttsnMsg.setParseBuffer(_mqttsnClient->getMqttsnMsgPtr());
        GwInfoMqttsnMsg &gwMsg = static_cast<GwInfoMqttsnMsg &>(_mqttsnMsg);
                
        if(gwMsg.getLength() == 3 && gwMsg.getMsgType() == GWINFO){
            _mqttsnClient->saveGatewayAddress();
            return true;
        }
    }

    return false;    
}

bool Mqttsn::connect(const char * clientId, uint16_t keepAlive){
    connect(clientId, false);
}

bool Mqttsn::connect(const char * clientId, bool willFlag, bool cleanSession, uint16_t keepAlive) {
    uint8_t buf[6];
    _mqttsnMsg.setParseBuffer(buf);
    ConnectMqttsnMsg &connectMsg = static_cast<ConnectMqttsnMsg &>(_mqttsnMsg);
    connectMsg.build(sizeof(buf) + strlen(clientId), 0, keepAlive);
    
    _mqttsnClient->sendPacket(buf, sizeof(buf), reinterpret_cast<uint8_t *>(const_cast<char *>(clientId)), strlen(clientId));
    
    if(_waitResponse(2000) && _mqttsnClient->getMqttsnMsgPtr()) {
        _mqttsnMsg.setParseBuffer(_mqttsnClient->getMqttsnMsgPtr());
        ConnackMqttsnMsg &connackMsg = static_cast<ConnackMqttsnMsg &>(_mqttsnMsg);

        if(connackMsg.getMsgType() == CONNACK && connackMsg.getReturnCode() == ReturnCode::ACCEPTED) {
            return true;
        } else {
            return false;
        }
    }
}

void Mqttsn::setDebugStream(Stream & stream) {
    _debugStream = &stream;

    if(_mqttsnClient != nullptr) {
        _mqttsnClient->setDebugStream(stream);
    }
}

bool Mqttsn::_waitResponse(int milliseconds) {
    unsigned long now = millis();

    do {
        _mqttsnClient->readPacket();
    }while(!_mqttsnClient->packetAvailable() && millis() - now < milliseconds);

    if(_mqttsnClient->packetAvailable()) {
        return true;
    } else {
        return false;
    }
}