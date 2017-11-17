#include "MQTTSNXbee.h"

#define xbeeSerial Serial1
#define debugSerial SerialUSB


#if defined (ARDUINO_SODAQ_AUTONOMO)
  #define xbeeSleepRQ BEEDTR
  #define SD_CS_PIN CS_SD
#endif

MQTTSNXbee mqttsnxbee(xbeeSerial);

void onTopicMsg(unsigned int topicId, unsigned int topicIdType, const char * data, unsigned int dataLength){
  debugSerial.println();
  debugSerial.print("TopicId: ");
  debugSerial.print(topicId);
  debugSerial.print("\t Data: ");
  for(int i = 0; i < dataLength; i++){
    debugSerial.write(data[i]);
  }
  debugSerial.println();
}

void onShortTopicMsg(const char * topicId, const char * data, unsigned int dataLength){
  debugSerial.println();
  debugSerial.print("TopicId: ");
  debugSerial.write(topicId);
  debugSerial.print("\t Data: ");
  for(int i = 0; i < dataLength; i++){
    debugSerial.write(data[i]);
  }
  debugSerial.println();
}


void setup() {
#if defined (ARDUINO_SODAQ_AUTONOMO)
  pinMode(BEE_VCC, OUTPUT);  
  digitalWrite(BEE_VCC, HIGH);
  pinMode(BEEDTR, OUTPUT);
  digitalWrite(BEEDTR, LOW);
  delay(5000);
  while(!SerialUSB);
#endif
  
  // put your setup code here, to run once:
  xbeeSerial.begin(4800);
  debugSerial.begin(19200);
  mqttsnxbee.setDebugStream(debugSerial);
  debugSerial.println("Reset");

  


  mqttsnxbee.setTopicMsgCallback(&onTopicMsg);
  mqttsnxbee.setShortTopicCallback(&onShortTopicMsg);
  if(mqttsnxbee.searchGwAndConnect("Arduino2")){
    debugSerial.println("Connect OK");
  }else{
    debugSerial.println("Connect NOK");
  }

  delay(1000);
  
  unsigned int out;
  /*if(mqttsnxbee.subscribe(11, &out)){
    debugSerial.print(out);
    debugSerial.println("Subscribe OK");
  } else{
    debugSerial.println("Subscribe NOK");
  }*/
}

void loop() {
  static unsigned long lastTime = millis();
  mqttsnxbee.continuosAsyncTask();  
 

  /*if(mqttsnxbee.publish(0,false, true, "Hola", 1)){
    Serial.println("Publish OK");
  } else{
    Serial.println("Publish NOK");
  }*/

  
  /*int topic = 0;
  if(mqttsnxbee.unsubscribe(topic)){
    Serial.println("Unsubscribe OK");
  } else{
    Serial.println("Unsubscribe NOK");
  }*/
  if(millis() > lastTime + 5000){
    unsigned int out;
    if(mqttsnxbee.subscribe(11, &out)){
      debugSerial.print(out);
      debugSerial.println("Subscribe OK");
    } else{
      debugSerial.println("Subscribe NOK");
    }
    lastTime = millis();
  }

}
