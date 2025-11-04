#include "encoder.h"

#include <Arduino.h>

#include "lvgl.h"

#ifndef GPIO_ROT_ENC_A
#  error "GPIO_ROT_ENC_A must be defined"
#endif
#ifndef GPIO_ROT_ENC_B
#  error "GPIO_ROT_ENC_B must be defined"
#endif
#ifndef GPIO_PUSH_BUTTON
#  error "GPIO_PUSH_BUTTON must be defined"
#endif

static uint32_t s_debounce_ticks = pdMS_TO_TICKS(40);

static volatile int32_t s_enc_diff = 0;
static volatile bool s_btn_pressed = false; // level: LOW = pressed (INPUT_PULLUP)
static volatile TickType_t s_last_btn_isr = 0;

void encoder_set_debounce_ms(int ms) {
  if (ms < 0)
    ms = 0;
  s_debounce_ticks = pdMS_TO_TICKS(ms);
}

void encoder_reset(void) {
  noInterrupts();
  s_enc_diff = 0;
  s_btn_pressed = false;
  interrupts();
}

static void IRAM_ATTR isr_rotary() {
  static uint8_t last_ab = 0;

  uint8_t a = digitalRead(GPIO_ROT_ENC_A);
  uint8_t b = digitalRead(GPIO_ROT_ENC_B);
  uint8_t ab = (a << 1) | b;
  uint8_t transition = (last_ab << 2) | ab;

  switch (transition) {
  case 0b0001:
  case 0b0111:
  case 0b1110:
  case 0b1000:
    s_enc_diff += 1; // CW
    break;
  case 0b0010:
  case 0b0100:
  case 0b1101:
  case 0b1011:
    s_enc_diff -= 1; // CCW
    break;
  default:
    break;
  }
  last_ab = ab;
}

static void IRAM_ATTR isr_button() {
  TickType_t now = xTaskGetTickCountFromISR();
  if ((now - s_last_btn_isr) < s_debounce_ticks)
    return;
  s_last_btn_isr = now;

  // With INPUT_PULLUP, pressed = LOW
  s_btn_pressed = (digitalRead(GPIO_PUSH_BUTTON) == LOW);
}

void encoder_init(void) {

  pinMode(GPIO_PUSH_BUTTON, INPUT_PULLUP);
  pinMode(GPIO_ROT_ENC_A, INPUT);
  pinMode(GPIO_ROT_ENC_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(GPIO_PUSH_BUTTON), isr_button, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_ROT_ENC_A), isr_rotary, CHANGE);
  attachInterrupt(digitalPinToInterrupt(GPIO_ROT_ENC_B), isr_rotary, CHANGE);
}

void encoder_lvgl_read(lv_indev_drv_t *drv, lv_indev_data_t *data) {
  (void)drv;

  int32_t diff;
  bool pressed;
  noInterrupts();
  diff = s_enc_diff;
  s_enc_diff = 0;
  pressed = s_btn_pressed;
  interrupts();

  if (diff >= 1) {
    data->enc_diff = +1;
  } else if (diff <= -1) {
    data->enc_diff = -1;
  } else {
    data->enc_diff = 0;
  }

  data->state = pressed ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
}
