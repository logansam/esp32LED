#pragma once
#include <Arduino.h>

// LED strip configuration
#define NUM_GROUPS 6
#define GROUP_SIZE 20
#define NUM_LEDS (GROUP_SIZE * NUM_GROUPS)

// Target-specific pin definitions (can be overridden by build_flags in platformio.ini)
#ifndef LED_DATA_PIN
#define LED_DATA_PIN 18
#endif

#ifndef BUTTON_PIN
#define BUTTON_PIN 32
#endif
