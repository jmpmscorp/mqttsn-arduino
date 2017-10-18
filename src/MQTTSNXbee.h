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

        

        uint8_t sendPacket(boolean broadcast, uint8_t length);

        boolean publishCommon(const char * data, uint16_t frameLength);
        boolean subscribeCommon(uint16_t frameLength);
        boolean unsubscribeCommon(uint16_t frameLength);
};

#endif