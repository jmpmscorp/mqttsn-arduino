#include "MQTTSNXbee.h"

MQTTSNXbee mqttsnxbee(Serial1);

void setup() {
  // put your setup code here, to run once:
  Serial1.begin(9600);
  Serial.begin(19200);
  mqttsnxbee.setDebugStream(Serial);
}

void loop() {
  // put your main code here, to run repeatedly:
  //mqttsnxbee.searchGateway();
  mqttsnxbee.connect("Arduino1");
  //mqttsnxbee.publish("su","Esto es una prueba");
  //mqttsnxbee.subscribe("topicDeMierda");
  //mqttsnxbee.subscribe("su");
  //mqttsnxbee.subscribe(2500);
  //mqttsnxbee.unsubscribe("topicDeMierda");
  //mqttsnxbee.unsubscribe("su");
  //mqttsnxbee.unsubscribe(2500);

  //mqttsnxbee.disconnect();
  //mqttsnxbee.disconnect(1205);

  mqttsnxbee.pingReq();
  
  
  delay(5000);

}
