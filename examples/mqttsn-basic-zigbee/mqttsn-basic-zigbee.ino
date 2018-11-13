#define ZIGBEE_MQTTSN_CLIENT
#include "MQTTSN.h"

#if defined(ARDUINO_AVR_SODAQ_MBILI)
  #define debugSerial Serial
  #define zigbeeSerial Serial1
  #define zigbeeSleepRQ BEEDTR
#elif defined (ARDUINO_SODAQ_AUTONOMO)
  #define debugSerial SerialUSB
  #define zigbeeSerial Serial1
  #define zigbeeSleepRQ BEEDTR
#endif

ZigbeeMqttsnClient client(zigbeeSerial);
MQTTSN mqttsn(client);

void setup() {
    #if defined (ARDUINO_SODAQ_AUTONOMO)
        pinMode(BEE_VCC, OUTPUT);  
        digitalWrite(BEE_VCC, HIGH);
        pinMode(BEEDTR, OUTPUT);
        digitalWrite(BEEDTR, LOW);
        delay(5000);
        while(!SerialUSB);
    #endif

    
    zigbeeSerial.begin(9600);
    debugSerial.begin(19200);
    
    mqttsn.setDebugStream(debugSerial);
    debugSerial.println("Start");

    delay(1000);
    mqttsn.searchGateway();
}

void loop() {
  //client.readPacket();
}
