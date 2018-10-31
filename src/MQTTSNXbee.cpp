#include "MQTTSNXbee.h"

MQTTSNXbee::MQTTSNXbee(Stream & xbeeStream){
    this->xbee = XBee();
    this->xbee.setSerial(xbeeStream);
    this->mqttsnParser = new MQTTSNParser();
}

void MQTTSNXbee::_incrementNextMsgId(){
    nextMsgId++;

    if(nextMsgId == 0x7E) nextMsgId = 0; // Skip 7E values because is start byte for API MODE
}

uint8_t MQTTSNXbee::_sendPacket(uint8_t length, bool broadcast){
    
    if(broadcast)
        _tx = ZBTxRequest(XBeeAddress64(0x00,BROADCAST_ADDRESS), mqttsnParser->buffer, length);
    else
        _tx = ZBTxRequest(gatewayAddr, mqttsnParser->buffer, length);

    xbee.send(_tx);
        
    if(xbee.readPacket(500)){
        if(xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE){
            xbee.getResponse().getZBTxStatusResponse(_txStatus);
            //debugPrintLn(_txStatus.getDeliveryStatus());
            _lastSent = millis();
            return _txStatus.getDeliveryStatus();
        }
    }
    else if (xbee.getResponse().isError()) {
        debugPrintLn(F("XBEE TRANSMISION ERROR"));
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

bool MQTTSNXbee::_waitResponsePacket(uint16_t timeout){
    uint8_t retries = 0;
    bool packetReceived;

    do{
        if(_wdtChecker != nullptr) {
            (*_wdtChecker)();
        }
        packetReceived = xbee.readPacket(timeout);
        
        if(packetReceived){
            if(xbee.getResponse().getApiId() == ZB_RX_RESPONSE){
                xbee.getResponse().getZBRxResponse(_rx);
        
                if(_rx.getOption() == ZB_PACKET_ACKNOWLEDGED){
                    debugPrintLn(F("ACK"));
                }
                
                for(size_t i = 0; i < _rx.getDataLength(); i++){
                    responseBuffer[i] = _rx.getData()[i];
                }
                _lastReceived = millis();
                return true;
            } else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE){
                debugPrintLn("MODEN STATUS RESPONSE");
                _handleModemStatusResponse();
                return false;
            } else {
                debugPrintLn(F("UKNOWN PACKET"));
                retries ++;
            }
            
        }else if(xbee.getResponse().isError()){
            debugPrintLn("XBEE RESPONSE ERROR");
            _handleResponseError();
            return false;
        }else {
            retries++;
        }
    }while(retries < 3);
    
    return false;
    //Skip any others packets that aren't RESPONSE
}

boolean MQTTSNXbee::_continuosWait(){
    xbee.readPacket();
    
    if (xbee.getResponse().isAvailable()) {
      // got something
      
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        // got a zb rx packet
        
        // now fill our zb rx class
        xbee.getResponse().getZBRxResponse(_rx);
            
        if (_rx.getOption() == ZB_PACKET_ACKNOWLEDGED) {
            // the sender got an ACK
            
        } else {
            // we got it (obviously) but sender didn't get an ACK
            
        }
        // set dataLed PWM to value of the first byte in the data
        for(size_t i = 0; i < _rx.getDataLength();i++){
             
            responseBuffer[i] = _rx.getData()[i];
        }

        /*for(int i = 0; i < _rx.getDataLength(); i++) {
            debugPrint(responseBuffer[i], HEX);
            debugPrint('-');
        }
        debugPrintLn();*/

        //responseBuffer = _rx.getData();
        _lastReceived = millis();
        return true;
      } else if (xbee.getResponse().getApiId() == MODEM_STATUS_RESPONSE) {
        xbee.getResponse().getModemStatusResponse(_msr);
        // the local XBee sends this response on certain events, like association/dissociation      
      } else {
      }
    } else if (xbee.getResponse().isError()) {
      //nss.print("Error reading packet.  Error code: ");  
      //nss.println(xbee.getResponse().getErrorCode());
    }
    return false;
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
    debugPrintLn("Error");
}