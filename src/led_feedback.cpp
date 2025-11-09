#include "led_feedback.h"

#include <Adafruit_NeoPixel.h>
#include <Arduino.h>

#include "driver/ledc.h"

static Adafruit_NeoPixel strip(LED_COUNT, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

static const ledc_mode_t MOTOR_SPEED_MODE = LEDC_LOW_SPEED_MODE;
static const ledc_timer_t MOTOR_TIMER = LEDC_TIMER_0;
static const ledc_channel_t MOTOR_CHANNEL = LEDC_CHANNEL_0;
static const uint32_t MOTOR_FREQ = 2000;   // Hz
static const uint32_t MOTOR_RES_BITS = 10; // 10-bit
static const uint32_t MOTOR_DUTY_ON = 600; // 0..(2^RES-1) = 0..1023

static uint32_t flash_until_ms = 0;
static uint32_t motor_off_deadline_ms = 0;

static inline void motor_write(uint32_t duty) {
  ledc_set_duty(MOTOR_SPEED_MODE, MOTOR_CHANNEL, duty);
  ledc_update_duty(MOTOR_SPEED_MODE, MOTOR_CHANNEL);
}
static inline void motor_on() {
  motor_write(MOTOR_DUTY_ON);
}
static inline void motor_off() {
  motor_write(0);
}

static void fill_all(uint32_t color) {
  for (int i = 0; i < LED_COUNT; ++i)
    strip.setPixelColor(i, color);
  strip.show();
}

static uint32_t color_focus() {
  return strip.Color(255, 40, 0);
} // orange/red
static uint32_t color_break() {
  return strip.Color(0, 160, 255);
} // cyan/blue
static uint32_t color_flash() {
  return strip.Color(255, 255, 255);
} // white

extern "C" void led_feedback_init(void) {

  strip.begin();
  strip.clear();
  strip.show();

  // 1) Timer IDF LEDC
  ledc_timer_config_t tcfg = {};
  tcfg.speed_mode = MOTOR_SPEED_MODE;
  tcfg.timer_num = MOTOR_TIMER;
  tcfg.duty_resolution = (ledc_timer_bit_t)MOTOR_RES_BITS;
  tcfg.freq_hz = MOTOR_FREQ;
  tcfg.clk_cfg = LEDC_AUTO_CLK;
  ledc_timer_config(&tcfg);

  // 2) Channel
  ledc_channel_config_t ccfg = {};
  ccfg.gpio_num = MOTOR_PIN;
  ccfg.speed_mode = MOTOR_SPEED_MODE;
  ccfg.channel = MOTOR_CHANNEL;
  ccfg.intr_type = LEDC_INTR_DISABLE;
  ccfg.timer_sel = MOTOR_TIMER;
  ccfg.duty = 0;
  ccfg.hpoint = 0;
  ledc_channel_config(&ccfg);

  motor_off();
}

extern "C" void
led_feedback_update(bool is_focus, int /*session*/, int min_left, int sec_left, int full_min) {
  const uint32_t now = millis();

  if (motor_off_deadline_ms && now >= motor_off_deadline_ms) {
    motor_off();
    motor_off_deadline_ms = 0;
  }

  if (now < flash_until_ms) {
    fill_all(color_flash());
    return;
  }

  // Progress within stage
  const int total_sec_stage = max(1, full_min * 60);
  const int left_sec = max(0, min_left * 60 + sec_left);
  const int elapsed_sec = total_sec_stage - left_sec;
  float frac = (float)elapsed_sec / (float)total_sec_stage;
  if (frac < 0.f)
    frac = 0.f;
  if (frac > 1.f)
    frac = 1.f;

  // LEDs ON
  int on = (int)lroundf(frac * LED_COUNT);
  if (on < 0)
    on = 0;
  if (on > LED_COUNT)
    on = LED_COUNT;

  // Render
  const uint32_t col = is_focus ? color_focus() : color_break();
  strip.clear();
  for (int i = 0; i < on; ++i)
    strip.setPixelColor(i, col);
  strip.show();
}

extern "C" void led_feedback_on_transition(bool /*is_focus*/) {
  flash_until_ms = millis() + 600; // white flash
  motor_on();
  motor_off_deadline_ms = millis() + 200;
}
