#include "uart2debug.h"
#include <stdarg.h>

HardwareSerial DebugUART(UART2_NUM);

static bool uart_started = false;

void initUart2debug(uint32_t baud, int8_t rxPin, int8_t txPin) {
  if (!uart_started) {
    DebugUART.begin(baud, SERIAL_8N1, rxPin, txPin);
    uart_started = true;
  }
}

void debugPrintf(const char *fmt, ...) {
  if (!uart_started)
    return;

  char buf[256];
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, sizeof(buf), fmt, args);
  va_end(args);

  DebugUART.print(buf);
}
