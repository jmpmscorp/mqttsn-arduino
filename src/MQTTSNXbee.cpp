#include "MQTTSNXbee.h"

MQTTSNXbee::MQTTSNXbee(Stream & xbeeStream){
    this->xbee = XBee();
    this->xbee.setSerial(xbeeStream);
}

boolean MQTTSNXbee::connect(const char * clientId){
    uint8_t frameLength = this->buildConnectFrame(clientId);
    uint8_t xbeeResponse = sendPacket(true, frameLength);

    return true;
}

void MQTTSNXbee::disconnect(uint16_t duration){
    uint8_t frameLength = this->buildDisconnectFrame(duration);
    uint8_t xbeeResponse = sendPacket(true, frameLength);
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
    uint8_t response = sendPacket(true, frameLength);

    return true;
}

boolean MQTTSNXbee::searchGateway(){
    uint8_t frameLength = this->buildSearchGWFrame();
    uint8_t response = sendPacket(true,frameLength);
       
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
    uint8_t response = sendPacket(true, frameLength);

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
    uint8_t response = sendPacket(true, frameLength);
    
    return true;
}

boolean MQTTSNXbee::pingReq(){
    uint8_t frameLength = buildPingReqFrame();
    uint8_t response = sendPacket(true, frameLength);

    return true;
}

uint8_t MQTTSNXbee::sendPacket(boolean broadcast, uint8_t length){
    XBeeAddress64 addr = broadcast ? XBeeAddress64(0x00000000, 0x0000FFFF) : gatewayAddr;

    ZBTxRequest zbTx = ZBTxRequest(addr, this->buffer, length);

    ZBTxStatusResponse txStatus = ZBTxStatusResponse();
    
    this->xbee.send(zbTx);
        
    if(xbee.readPacket(500)){
        if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE){
            xbee.getResponse().getZBTxStatusResponse(txStatus);
            debugPrintLn(txStatus.getDeliveryStatus());
            return txStatus.getDeliveryStatus();
        }
        
    }
    else if (xbee.getResponse().isError()) {
        return xbee.getResponse().getErrorCode();
    }
}