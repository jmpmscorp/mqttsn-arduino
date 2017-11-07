#ifndef MQTTSNXbee_H_
#define MQTTSNXbee_H_



#include "MQTTSNCommon.h"
#include "XBee.h"

class MQTTSNXbee : public MQTTSNCommon{  
    public:  
        MQTTSNXbee(Stream & xbeeStream);

    private:    
               
        XBee xbee;
        XBeeAddress64 gatewayAddr;
        
        ZBTxRequest _tx;
        XBeeResponse _response = XBeeResponse();
        ZBTxStatusResponse _txStatus = ZBTxStatusResponse();
        ZBRxResponse _rx = ZBRxResponse();
        ModemStatusResponse _msr = ModemStatusResponse();        
        
        boolean subscribeCommon(uint16_t frameLength);
        boolean unsubscribeCommon(uint16_t frameLength);

        void _incrementNextMsgId();
        void _saveGatewayAddress();
        uint8_t _sendPacket(uint8_t length, boolean broadcast = false);
        uint8_t _sendBroadcastPacket(uint8_t length);
        boolean  _waitResponsePacket(int timeout = 2000);
        boolean _continuosWait();
        void _handleModemStatusResponse();
        void _handleResponseError();

        
};

#endif