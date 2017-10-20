#include "MQTTSNXbee.h"

MQTTSNXbee::MQTTSNXbee(Stream & xbeeStream){
    this->xbee = XBee();
    this->xbee.setSerial(xbeeStream);
}

boolean MQTTSNXbee::connect(const char * clientId){
    uint8_t frameLength = this->buildConnectFrame(clientId);
    uint8_t xbeeResponse = _sendPacket(frameLength);

    if(!_waitResponsePacket()) return false;

    uint8_t * mqttsnPacket = _rx.getData();

    //Packet received should be: LENGTH(always 3) | CONNACK(0x05) | RETURN_CODE
    if(mqttsnPacket[0] != 3) return false;

    if(mqttsnPacket[1] != CONNACK) return false;

    if(mqttsnPacket[2] != ACCEPTED) return false;

    return true;
}

void MQTTSNXbee::disconnect(uint16_t duration){
    uint8_t frameLength = this->buildDisconnectFrame(duration);
    uint8_t xbeeResponse = _sendPacket(true, frameLength);
}

boolean MQTTSNXbee::publish(const char * topic, const char * data){
    uint8_t frameLength = this->buildPublishFrame(topic, data);
    return publishCommon(data, frameLength);
}

boolean MQTTSNXbee::publish(uint16_t topic, const char * data){
    uint8_t frameLength = buildPublishFrame(topic, data);

    return publishCommon(data, frameLength);
}

boolean MQTTSNXbee::publishCommon(const char * data, uint16_t frameLength){
    uint8_t response = _sendPacket(true, frameLength);

    return true;
}

boolean MQTTSNXbee::searchGateway(){
    uint8_t frameLength = this->buildSearchGWFrame();
    uint8_t response = _sendBroadcastPacket(frameLength);

    if(!_waitResponsePacket()) return false;

    uint8_t * mqttsnPacket = _rx.getData();

    //Packet should be Length | GWINFO (0x02) | gatewayId
    uint8_t length = *mqttsnPacket++;
    
    if( *mqttsnPacket != GWINFO){
        return false;
    } else{
        gatewayAddr = _rx.getRemoteAddress64();
    }
        
       
    return true;
}

boolean MQTTSNXbee::subscribe(const char * topic){
    uint8_t frameLength = buildSubscribeOrUnsubscribeFrame(topic, true);
    return subscribeCommon(frameLength);
}


boolean MQTTSNXbee::subscribe(uint16_t topic){
    uint8_t frameLength = buildSubscribeOrUnsubscribeFrame(topic, true);
    return subscribeCommon(frameLength);
}

boolean MQTTSNXbee::subscribeCommon(uint16_t frameLength){
    uint8_t response = _sendPacket(true, frameLength);

    return true;
}

boolean MQTTSNXbee::unsubscribe(const char * topic){
    uint8_t frameLength = buildSubscribeOrUnsubscribeFrame(topic, false);

    return unsubscribeCommon(frameLength);
}

boolean MQTTSNXbee::unsubscribe(uint16_t topic){
    uint8_t frameLength = buildSubscribeOrUnsubscribeFrame(topic, false);
    
    return unsubscribeCommon(frameLength);
}

boolean MQTTSNXbee::unsubscribeCommon(uint16_t frameLength){
    uint8_t response = _sendPacket(true, frameLength);
    
    return true;
}

boolean MQTTSNXbee::pingReq(){
    uint8_t frameLength = buildPingReqFrame();
    uint8_t response = _sendPacket(true, frameLength);

    return true;
}

uint8_t MQTTSNXbee::_sendPacket(uint8_t length, boolean broadcast){
    
    if(broadcast)
        _tx = ZBTxRequest(XBeeAddress64(0x00,BROADCAST_ADDRESS), buffer, length);
    else
        _tx = ZBTxRequest(gatewayAddr, buffer, length);

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