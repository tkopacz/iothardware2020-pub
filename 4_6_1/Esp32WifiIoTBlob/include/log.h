#pragma once
#include <Arduino.h>

#ifdef DEBUG_ESP_PORT
#define DEBUG_MSG(...) { DEBUG_ESP_PORT.printf( __VA_ARGS__ ); DEBUG_ESP_PORT.println();}
#else
#define DEBUG_MSG(...)
#endif

#ifdef ERROR_ESP_PORT
#define ERROR_MSG(...) ERROR_ESP_PORT.printf( __VA_ARGS__ )
#else
#define ERROR_MSG(...)
#endif