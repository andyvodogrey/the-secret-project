#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifndef NEOPIXEL_PIN
#  define NEOPIXEL_PIN 4
#endif

#ifndef MOTOR_PIN
#  define MOTOR_PIN 2
#endif

#ifndef LED_COUNT
#  define LED_COUNT 12
#endif

#ifdef __cplusplus
extern "C" {
#endif

void led_feedback_init(void);

/**
 * Call once per second LVGL timer.
 * @param is_focus   true=focus, false=break
 * @param session    1..4
 * @param min_left   minutes remaining in current stage
 * @param sec_left   seconds remaining in current stage
 * @param full_min   total minutes for the current stage
 */
void led_feedback_update(bool is_focus, int session, int min_left, int sec_left, int full_min);
void led_feedback_on_transition(bool is_focus);

#ifdef __cplusplus
}
#endif
