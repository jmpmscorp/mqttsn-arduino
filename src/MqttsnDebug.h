#pragma once

#define ENABLED_MQTTSN_DEBUG true

#if ENABLED_MQTTSN_DEBUG
    #if defined(CUSTOM_STREAM_DEBUG) && ( defined(ARDUINO_ARCH_AVR) || defined(ARDUINO_ARCH_SAMD))
        #define DEBUG(...) { if (_debugStream) _debugStream->print(__VA_ARGS__); }
        #define DEBUGHEX(...) { if (_debugStream) _debugStream->print(__VA_ARGS__, HEX); }
        #define DEBUGLN(...) { if (_debugStream) _debugStream->println(__VA_ARGS__); }
    #elif defined(ARDUINO_ARCH_AVR)
        #include <Arduino.h>
        #define DEBUG(...) { Serial->print(__VA_ARGS__); }
        #define DEBUGHEX(...) { Serial->print(__VA_ARGS__, HEX); }
        #define DEBUGLN(...) { Serial->println(__VA_ARGS__); }
    #elif defined(ARDUINO_ARCH_SAMD)
        #include <Arduino.h>
        #define DEBUG(...) { SerialUSB->print(__VA_ARGS__); }
        #define DEBUGHEX(...) { SerialUSB->print(__VA_ARGS__, HEX); }
        #define DEBUGLN(...) { SerialUSB->println(__VA_ARGS__); }
    #else
        #define DEBUG(...)
        #define DEBUGHEX(...)
        #define DEBUGLN(...)
    #endif
#else
    #pragma message ("NOT_ENABLED_MQTTSN_DEBUG")
    /// Debug print
    #define DEBUG(...)
    #define DEBUGHEX(...)
    #define DEBUGLN(...)
#endif