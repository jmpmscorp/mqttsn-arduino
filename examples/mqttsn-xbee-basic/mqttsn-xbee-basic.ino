#include "MQTTSNXbee.h"

#define xbeeSerial Serial1
#define debugSerial Serial

MQTTSNXbee mqttsnxbee(xbeeSerial);

void setup() {
  // put your setup code here, to run once:
  xbeeSerial.begin(9600);
  debugSerial.begin(19200);
  mqttsnxbee.setDebugStream(debugSerial);

  if(mqttsnxbee.searchGateway()){
    Serial.println("Search OK");
  }else{
    Serial.println("Search NOK");
  }

  if(mqttsnxbee.connect("Arduino1")){
    Serial.println("Connect OK");
  }else{
    Serial.println("Connect NOK");
  }

  delay(2000);
  if(mqttsnxbee.publish(2, "Hola")){
    Serial.println("Publish OK");
  } else{
    Serial.println("Publish NOK");
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  /*mqttsnxbee.searchGateway();
  mqttsnxbee.connect("Arduino1");
  mqttsnxbee.publish("su","Esto es una prueba");
  mqttsnxbee.subscribe("topicDeMierda");
  mqttsnxbee.subscribe("su");
  mqttsnxbee.subscribe(2500);
  mqttsnxbee.unsubscribe("topicDeMierda");
  mqttsnxbee.unsubscribe("su");
  mqttsnxbee.unsubscribe(2500);

  mqttsnxbee.disconnect();
  mqttsnxbee.disconnect(1205);

  mqttsnxbee.pingReq();*/
  
  
  delay(5000);

}
