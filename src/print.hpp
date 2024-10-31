#pragma once
#include "util.hpp"

#if IS_ATTINY13

    #define PRINT_BEGIN()
    #define PRINT(...)
    #define PRINTLN(...)

#else
    #include <Arduino.h>

    #define PRINT_BEGIN() Serial.begin(115200);
    #define PRINT(...) Serial.print(__VA_ARGS__)
    #define PRINTLN(...) Serial.println(__VA_ARGS__)

#endif

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)
#define PRINT_PREFIX() PRINT(F("[" __BASE_FILE__ "@" STRINGIFY(__LINE__)  "]"))
