#include "MQTTSNXbee.h"

MQTTSNXbee::MQTTSNXbee(Stream & xbeeStream){
    this->xbee = XBee();
    this->xbee.setSerial(xbeeStream);
    this->mqttsnParser = new MQTTSNParser();
}



void MQTTSNXbee::disconnect(uint16_t duration){
    uint8_t frameLength = mqttsnParser->disconnectFrame(duration);
    uint8_t xbeeResponse = _sendPacket(true, frameLength);
}

boolean MQTTSNXbee::publish(const char * topic, const char * data){
    uint8_t frameLength = mqttsnParser->publishFrame(topic, data, nextMsgId);
    return publishCommon(data, frameLength);
}

boolean MQTTSNXbee::publish(uint16_t topic, const char * data){
    uint8_t frameLength = mqttsnParser->publishFrame(topic, data, nextMsgId);

    return publishCommon(data, frameLength);
}

boolean MQTTSNXbee::publishCommon(const char * data, uint16_t frameLength){
    uint8_t response = _sendPacket(true, frameLength);

    return true;
}

boolean MQTTSNXbee::subscribe(const char * topic){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, true);
    return subscribeCommon(frameLength);
}


boolean MQTTSNXbee::subscribe(uint16_t topic){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, true);
    return subscribeCommon(frameLength);
}

boolean MQTTSNXbee::subscribeCommon(uint16_t frameLength){
    uint8_t response = _sendPacket(true, frameLength);

    return true;
}

boolean MQTTSNXbee::unsubscribe(const char * topic){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, false);

    return unsubscribeCommon(frameLength);
}

boolean MQTTSNXbee::unsubscribe(uint16_t topic){
    uint8_t frameLength = mqttsnParser->subscribeOrUnsubscribeFrame(topic, nextMsgId, false);
    
    return unsubscribeCommon(frameLength);
}

boolean MQTTSNXbee::unsubscribeCommon(uint16_t frameLength){
    uint8_t response = _sendPacket(true, frameLength);
    
    return true;
}

boolean MQTTSNXbee::pingReq(const char * clientId){
    uint8_t frameLength = mqttsnParser->pingReqFrame(clientId);
    uint8_t response = _sendPacket(true, frameLength);

    return true;
}

uint8_t MQTTSNXbee::_sendPacket(uint8_t length, boolean broadcast){
    
    if(broadcast)
        _tx = ZBTxRequest(XBeeAddress64(0x00,BROADCAST_ADDRESS), mqttsnParser->buffer, length);
    else
        _tx = ZBTxRequest(gatewayAddr, mqttsnParser->buffer, length);

    xbee.send(_tx);
        
    if(xbee.readPacket(500)){
        if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE){
            xbee.getResponse().getZBTxStatusResponse(_txStatus);
            //debugPrintLn(_txStatus.getDeliveryStatus());
            return _txStatus.getDeliveryStatus();
        }
        
    }
    else if (xbee.getResponse().isError()) {
        _handleResponseError();
        return 0xFF;
    } else{
        0xFE;
    }
}

void MQTTSNXbee::_saveGatewayAddress(){
    gatewayAddr = _rx.getRemoteAddress64();
}

uint8_t MQTTSNXbee::_sendBroadcastPacket(uint8_t length){
    _sendPacket(length, true);
}

boolean MQTTSNXbee::_waitResponsePacket(int timeout){
    uint8_t retry = 0;

    boolean packetReceived;
    do{
        packetReceived = xbee.readPacket(timeout);

        if(packetReceived){
            if(xbee.getResponse().getApiId() == ZB_RX_RESPONSE){
                xbee.getResponse().getZBRxResponse(_rx);
        
                if(_rx.getOption() == ZB_PACKET_ACKNOWLEDGED){
                    debugPrintLn("ACK");
                }
                else{
                    for(int i = 0; i < _rx.getDataLength(); i++){
                        Serial.print(_rx.getData()[i],HEX);
                        Serial.print('-');
                    }
                    Serial.println();
                    responseBuffer = _rx.getData();
                    return true;
                }
            } else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE){
                _handleModemStatusResponse();
            }
        }else if(xbee.getResponse().isError()){
            _handleResponseError();
            return false;
        }
    } while(packetReceived); //Skip any others packets that aren't RESPONSE
}

void MQTTSNXbee::_handleModemStatusResponse(){
    xbee.getResponse().getModemStatusResponse(_msr);

    if(_msr.getStatus() == ASSOCIATED){
        debugPrintLn("Xbee Associated");
    } else if( _msr.getStatus() == DISASSOCIATED){
        debugPrintLn("Xbee Disassociated");
    } else{
        debugPrintLn("MSR uknown status");
    }
}

void MQTTSNXbee::_handleResponseError(){

}