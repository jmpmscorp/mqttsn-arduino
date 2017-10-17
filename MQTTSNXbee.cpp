#include "MQTTSNXbee.h"

MQTTSNXbee::MQTTSNXbee(Stream & xbeeStream){
    this->xbee = XBee();
    this->xbee.setSerial(xbeeStream);
}

boolean MQTTSNXbee::connect(const char * id){
    return true;
}

void MQTTSNXbee::disconnect(){

}

boolean MQTTSNXbee::publish(const char * topic, const char * payload){
    return true;
}

boolean MQTTSNXbee::searchGateway(){
    uint8_t frameLenght = this->buildSearchGWFrame();
    uint8_t response = sendPacket(true,frameLenght);
       
    return true;
}

boolean MQTTSNXbee::subscribe(const char * topic){
    return true;
}

boolean MQTTSNXbee::unsubscribe(const char * topic){
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