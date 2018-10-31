#include "MQTTSNXbee.h"

#define xbeeSerial Serial1
#define debugSerial Serial

MQTTSNXbee mqttsnxbee(xbeeSerial);

void onPredefinedTopicMsg(unsigned int topicId, const char * data, unsigned int dataLength){
  Serial.println();
  Serial.print("TopicId: ");
  Serial.print(topicId);
  Serial.print("\t Data: ");
  for(int i = 0; i < dataLength; i++){
    Serial.write(data[i]);
  }
}

void onShortTopicMsg(const char * topicId, const char * data, unsigned int dataLength){
  Serial.println("SHORT TOPIC");
  Serial.println();
  Serial.print("TopicId: ");
  Serial.write(topicId);
  Serial.print("\t Data: ");
  Serial.println(data);
  //Serial.println();
}


void setup() {
  
  // put your setup code here, to run once:
  xbeeSerial.begin(9600);
  debugSerial.begin(19200);
  mqttsnxbee.setDebugStream(debugSerial);
  Serial.println("Reset");

  // mqttsnxbee.setPredefinedTopicCallback(&onPredefinedTopicMsg);
  // mqttsnxbee.setShortTopicCallback(&onShortTopicMsg);
  /*if(mqttsnxbee.searchGwAndConnect("Arduino1")){
    Serial.println("Connect OK");
  }else{
    Serial.println("Connect NOK");
  } 

  delay(1000);*/
}

void loop() {
  /*byte receivedMsg;
  static unsigned long now;
  
  receivedMsg = mqttsnxbee.continuosAsyncTask();

  //Serial.println(receivedMsg);
  if(mqttsnxbee.getState() == MQTTSN_AWAKE){
    if(receivedMsg == PINGRESP)
      sleep();
    /*else{
      if(receivedMsg != 0xFF){
        now = millis();
      } else{
        if(millis() > now + 10000) sleep();
      }
    }
  } else{
    if(receivedMsg != 0xFF){
      now = millis();
    } else{
      if(millis() > now + 5000) sleep();
    }
  }*/

  if (mqttsnxbee.getState() == MQTTSN_DISCONNECTED || mqttsnxbee.getState() == MQTTSN_LOST ) {
      if(mqttsnxbee.searchGwAndConnectAsync("Arduino1") == MQTTSN_MAX_RETRIES_REACH)
      {
        mqttsnxbee.disconnect();
        debugSerial.println("MAX REACH");
      }
    }
    else {
      byte result = mqttsnxbee.continuosAsyncTask();

      if (millis() > mqttsnxbee.getLastReceived() + 6000  || result == MQTTSN_PINGRESP_TIMEOUT) {
        debugSerial.println("Sleep");
        enterSleep();
      }
    }
}

void enterSleep(){
  mqttsnxbee.sleep(10000);
  delay(10000);
  mqttsnxbee.awake();
  Serial.println("Awake");
}
