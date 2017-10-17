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
  mqttsnxbee.searchGateway();

  delay(5000);
}
