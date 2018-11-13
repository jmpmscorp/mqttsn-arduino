#include <Arduino.h>
#include "ZigbeeMqttsnClient.h"
#include "inttypes.h"

ZigbeeMqttsnClient::ZigbeeMqttsnClient(Stream & stream) {
    _serial = &stream;
}

void ZigbeeMqttsnClient::setSerial(Stream &stream){
    _serial = &stream;
}

bool ZigbeeMqttsnClient::packetAvailable() {
    return _receivePacketAvailable;
}

int ZigbeeMqttsnClient::readPacket() {
    uint8_t c;
    if( _packetAvailable || _error) {
        _receivePacketAvailable = false;
        _packetAvailable = false;
        _byteEscape = 0;
        _bytePos = 0;
        _frameLength = 0;
        _checksum = 0;
        _apiFrameType = 0;
    }

    while(_serial->available()) {
        c = _serial->read();
        DEBUG(c);
        DEBUG('-');
        if (_bytePos > 0 && c == START_BYTE) {
            _error = true;
            return 0;
        }

        if( _bytePos > 0 && c == ESCAPE ) {
            if(_serial->available()) {
                c = _serial->read();
                c = 0x20 ^ c;
            }
            else {
                _byteEscape = true;
                continue;
            }
        }

        if( _byteEscape == true ) {
            c ^= 0x20;
            _byteEscape = false; 
        }

        if( _bytePos >= 3) {
            if(_bytePos == 3) {
                _checksum = 0;
            }
            _checksum += c;
        }

        switch(_bytePos) {
            case 0:
                if ( c == START_BYTE) _bytePos ++;

                break;
            
            case 1:
                _frameLength = c * 256;
                _bytePos++;

                break;

            case 2:
                _frameLength += c;
                _bytePos++;

                break;

            case 3:
                _apiFrameType = c;
                DEBUG("Frame Type: ");
                DEBUGLN(_apiFrameType);
                _bytePos++;

                break;

            default:
                if(_bytePos == _frameLength + 3) {
                    if( (_checksum & 0xFF) == 0xFF ) {
                        DEBUGLN(F("No error"));
                        _packetAvailable = true;

                        if(_apiFrameType == RECEIVE_PACKET_FRAME_TYPE) {
                            _receivePacketAvailable = true;
                        }

                        _error = false;
                    }
                    else {
                        DEBUG(F("Error Checksum:"));
                        DEBUGHEX(_checksum);
                        
                        _error = true;
                    }

                    _bytePos = 0;
                    return 0;
                }
                else {
                    if(_apiFrameType == RECEIVE_PACKET_FRAME_TYPE && _bytePos >= RECEIVE_PACKET_FRAME_MQTTSN_DATA_OFFSET) {
                        _mqttsnFrameBuffer[_bytePos - RECEIVE_PACKET_FRAME_MQTTSN_DATA_OFFSET] = c;
                    }
                    else{
                        auxBuffer[_bytePos - FRAME_DATA_OFFSET] = c;
                    }

                    _bytePos++;
                }

                break;            

        }
    }
}


int ZigbeeMqttsnClient::sendPacket(bool broadcast) {
    //DEBUGLN("Send Packet");
    _frameLength = _mqttsnFrameBuffer[0] + 14;
    
    _sendByte(START_BYTE, false);    
    _sendByte( (uint8_t)((_frameLength >> 8) & 0xFF), true);     // MSB
    _sendByte( (uint8_t) (_frameLength & 0xFF), true);     // LSB
    
    _checksum = 0;
    _sendByte(TRANSMIT_REQUEST_FRAME_TYPE, true);
    _checksum += TRANSMIT_REQUEST_FRAME_TYPE; 
    _sendByte(1, true);      // Frame Number ID
    _checksum += _frameIdCounter++;
    //_frameIdCounter++;    
    
    if(broadcast) {
        _sendAddress(BROADCAST_ADDRESS_MSB, BROADCAST_ADDRESS_LSB);
    }
    else {
        _sendAddress(_gatewayAddressMsb, _gatewayAddressLsb);
    }

    _sendByte(0xFF, true);            // Reserved
    _checksum += 0xFF;
    _sendByte(0xFE, true);            
    _checksum += 0xFE;
    _sendByte(0x00, true);            // Broadcast Radius
    _checksum += 0x00;
    _sendByte(0x00, true);            // Transmit Options
    _checksum += 0x00;
    
    for(size_t i = 0; i < _mqttsnFrameBuffer[0]; i++) {
        _sendByte(_mqttsnFrameBuffer[i], true);
        _checksum += _mqttsnFrameBuffer[i];
    }

    _checksum = 0xFF - _checksum;
    _sendByte(_checksum, true);

    _serial->flush();

    unsigned long now = millis();

    do {
        delay(50);
        readPacket();
    }while(!_packetAvailable && (millis() - now < 500) );
    
    DEBUG("Frame Type: ");
    DEBUGLN(_apiFrameType);

    if(_packetAvailable && _apiFrameType == TRANSMIT_STATUS_FRAME_TYPE) {
        DEBUG(F("Delivery Status: "));
        DEBUGHEX(auxBuffer[4]);
        DEBUGLN();
        return auxBuffer[4];        // Delivery Status;
    }
}
        
void ZigbeeMqttsnClient::saveGatewayAddress() {

}

void ZigbeeMqttsnClient::_sendAddress(uint32_t msb, uint32_t lsb) {
    uint8_t b;
    
    for(int8_t i = 3; i >= 0 ; i-- ){
        b = static_cast<uint8_t> ((msb >> (8 * i)) & 0xFF);
        _sendByte(b, true);
        _checksum += b;
    }

    for(int8_t i = 3; i >= 0 ; i-- ){
        b = static_cast<uint8_t> ((lsb >> (8 * i)) & 0xFF);
        _sendByte(b, true);
        _checksum += b;
    }
}

void ZigbeeMqttsnClient::_sendByte(uint8_t b, bool escape) {
    if( escape && ( b == START_BYTE || b == ESCAPE || b == XON || b == XOFF )) {
        _serial->write(ESCAPE);
        _serial->write(b ^ 0x20);
    }
    else {
        _serial->write(b);
    }

    DEBUG('-')

}

