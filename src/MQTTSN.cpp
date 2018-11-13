#include "MQTTSN.h"
#include "MQTTSNMsg.h"

MQTTSN::MQTTSN(MqttsnClient &mqttsnClient) : _mqttsnClient(&mqttsnClient){

}

bool MQTTSN::searchGateway() {
    mqttsn_msg_searchgw * msg = reinterpret_cast<mqttsn_msg_searchgw *> (_mqttsnClient->getMqttsnFrameBuffer());

    msg->length = 0x03;
    msg->type = SEARCHGW;
    msg->radius = 0;

    _mqttsnClient->sendPacket(true);
}

void MQTTSN::setDebugStream(Stream & stream) {
    _debugStream = &stream;

    if(_mqttsnClient != nullptr) {
        _mqttsnClient->setDebugStream(stream);
    }
}