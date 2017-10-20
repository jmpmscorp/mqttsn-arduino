#ifndef MQTTSNXbee_H_
#define MQTTSNXbee_H_



#include "MQTTSN.h"
#include "XBee.h"

class MQTTSNXbee : public MQTTSN{  
    public:  
        MQTTSNXbee(Stream & xbeeStream);


        boolean connect(const char * clientId);
        void disconnect(uint16_t duration = 0);
        boolean publish(const char * topic, const char * data);
        boolean publish(uint16_t topic, const char * data);
        boolean searchGateway();
        boolean subscribe(const char * topic);
        boolean subscribe(uint16_t topic);
        boolean unsubscribe(const char * topic);
        boolean unsubscribe(uint16_t topic);    
        boolean pingReq();

    private:    
        
        XBee xbee;
        XBeeAddress64 gatewayAddr;
        
        ZBTxRequest _tx;
        XBeeResponse _response = XBeeResponse();
        ZBTxStatusResponse _txStatus = ZBTxStatusResponse();
        ZBRxResponse _rx = ZBRxResponse();
        ModemStatusResponse _msr = ModemStatusResponse();
        
        boolean publishCommon(const char * data, uint16_t frameLength);
        boolean subscribeCommon(uint16_t frameLength);
        boolean unsubscribeCommon(uint16_t frameLength);

        uint8_t _sendPacket(uint8_t length, boolean broadcast = false);
        uint8_t _sendBroadcastPacket(uint8_t length);
        boolean  _waitResponsePacket(int timeout = 2000);
        void _handleModemStatusResponse();
        void _handleResponseError();

        
};

#endif