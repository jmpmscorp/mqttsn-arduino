#define ZIGBEE_MQTTSN_CLIENT
#include "Mqttsn.h"

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
Mqttsn mqttsn(client);

void setup() {
    #if defined (ARDUINO_SODAQ_AUTONOMO)
        pinMode(BEE_VCC, OUTPUT);  
        digitalWrite(BEE_VCC, HIGH);
        pinMode(BEEDTR, OUTPUT);
        digitalWrite(BEEDTR, LOW);
        unsigned long now = millis();
        while (!SerialUSB && millis() - now < 5000) {
          delay(100);
        }
    #endif

    
    zigbeeSerial.begin(9600);
    debugSerial.begin(19200);
    
    mqttsn.setDebugStream(debugSerial);
    debugSerial.println("Start");

    delay(1000);
    
    if(mqttsn.searchGateway()) {
      debugSerial.println(F("Search GW OK"));
      delay(1000);
      if(mqttsn.connect("Autonomo")) {
        debugSerial.println(F("Connect OK"));
      } else {
        debugSerial.println(F("Connect NOK"));
      }
    } else {
      debugSerial.println(F("Search GW NOK"));
    }
}

void loop() {
  client.readPacket();
}
