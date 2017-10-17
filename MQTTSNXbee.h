#ifndef MQTTSNXbee_H_
#define MQTTSNXbee_H_

#define DEBUG

#ifdef DEBUG
#define debugPrintLn(...) { if (this->_debugStream) this->_debugStream->println(__VA_ARGS__); }
#define debugPrint(...) { if (this->_debugStream) this->_debugStream->print(__VA_ARGS__); }
#warning "Debug mode is ON"
#else
#define debugPrintLn(...)
#define debugPrint(...)
#endif

#include "MQTTSN.h"
#include "XBee.h"

class MQTTSNXbee : public MQTTSN{  
    public:  
        MQTTSNXbee(Stream & xbeeStream);


        boolean connect(const char * id);
        void disconnect();
        boolean publish(const char * topic, const char * payload);
        boolean searchGateway();
        boolean subscribe(const char * topic);
        boolean unsubscribe(const char * topic);
        
        
        // Sets the optional "Diagnostics and Debug" stream.
        void setDebugStream(Stream &stream) { _debugStream = &stream; }
        void setDebugStream(Stream *stream) { _debugStream = stream; }

    private:    
        
        XBee xbee;
        XBeeAddress64 gatewayAddr;

        Stream * _debugStream;

        uint8_t sendPacket(boolean broadcast, uint8_t lenght);

    

};

#endif