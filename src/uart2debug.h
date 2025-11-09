#pragma once
#include <Arduino.h>

void initUart2debug(uint32_t baud = 115200, int8_t rxPin = 16, int8_t txPin = 17);

void debugPrintf(const char *fmt, ...);
