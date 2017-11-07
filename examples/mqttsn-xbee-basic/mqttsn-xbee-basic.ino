#include "MQTTSNXbee.h"

#define xbeeSerial Serial1
#define debugSerial Serial

MQTTSNXbee mqttsnxbee(xbeeSerial);

void onTopicMsg(unsigned int topicId, unsigned int topicIdType, const char * data, unsigned int dataLength){
  Serial.println();
  Serial.print("TopicId: ");
  Serial.print(topicId);
  Serial.print("\t Data: ");
  for(int i = 0; i < dataLength; i++){
    Serial.write(data[i]);
  }
  Serial.println();
}

void onShortTopicMsg(const char * topicId, const char * data, unsigned int dataLength){
  Serial.println();
  Serial.print("TopicId: ");
  Serial.write(topicId);
  Serial.print("\t Data: ");
  for(int i = 0; i < dataLength; i++){
    Serial.write(data[i]);
  }
  Serial.println();
}


void setup() {
  
  // put your setup code here, to run once:
  xbeeSerial.begin(9600);
  debugSerial.begin(19200);
  mqttsnxbee.setDebugStream(debugSerial);
  Serial.println("Reset");

  mqttsnxbee.setTopicMsgCallback(&onTopicMsg);
  mqttsnxbee.setShortTopicCallback(&onShortTopicMsg);
  if(mqttsnxbee.searchGwAndConnect("Arduino1")){
    Serial.println("Connect OK");
  }else{
    Serial.println("Connect NOK");
  } 

  delay(1000);
  unsigned int out;
  if(mqttsnxbee.subscribe("val/1", &out)){
    Serial.print(out);
    Serial.println("Subscribe OK");
  } else{
    Serial.println("Subscribe NOK");
  }

  delay(1000);
}

void loop() {
  mqttsnxbee.loopTask();  
 

  /*if(mqttsnxbee.publish(0,false, true, "Hola", 1)){
    Serial.println("Publish OK");
  } else{
    Serial.println("Publish NOK");
  }

  delay(5000);*/
}
