#include "MQTTSN.h"

uint8_t MQTTSN::buildSearchGWFrame(){
    this->buffer[0] = 0x03; //Length Byte
    this->buffer[1] = SEARCHGW;
    this->buffer[2] = 0x00; //Radius     
    
    return 3;
}
