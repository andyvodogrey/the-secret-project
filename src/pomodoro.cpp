#include "pomodoro.h"

#include <Arduino.h>

#include "esp_timer.h"
#include "uart2debug.h"

static esp_timer_handle_t s_timer = nullptr;
static bool s_running = false;
static bool s_focus = true;
static int s_session = 1;
static int s_focus_min = 25;
static int s_break_min = 5;

static int64_t s_stage_us = 0;
static int64_t s_stage_start_us = 0;

#define MINUTE_US (60LL * 1000000LL)

static void start_stage(bool focus) {
  s_focus = focus;
  s_stage_us = (focus ? s_focus_min : s_break_min) * MINUTE_US;
  s_stage_start_us = esp_timer_get_time();
  esp_timer_stop(s_timer);
  esp_timer_start_once(s_timer, s_stage_us);
}

static void timer_cb(void *) {
  if (!s_running)
    return;

  if (s_focus) {

    start_stage(false);
  } else {

    s_session++;
    if (s_session > 4) {
      s_running = false;
      esp_timer_stop(s_timer);
      return;
    }
    start_stage(true);
  }
}

void pomodoro_init(void) {
  if (s_timer)
    return;
  const esp_timer_create_args_t args = {.callback = timer_cb, .arg = nullptr, .name = "pomodoro"};
  esp_timer_create(&args, &s_timer);
}

void pomodoro_set_durations_minutes(int focus_min, int break_min) {
  if (focus_min > 0)
    s_focus_min = focus_min;
  if (break_min > 0)
    s_break_min = break_min;
}

void pomodoro_start(void) {
  Serial.printf("Start called\n");
  debugPrintf("Start called\n");
  pomodoro_init();
  s_running = true;
  s_session = 1;
  start_stage(true);
}

void pomodoro_stop(void) {
  Serial.printf("Stop called\n");
  debugPrintf("Stop called\n");

  if (!s_timer)
    return;

  esp_timer_stop(s_timer);
  s_running = false;

  s_stage_us = 0;
  s_stage_start_us = esp_timer_get_time();
}

bool pomodoro_running(void) {
  return s_running;
}
bool pomodoro_is_focus(void) {
  return s_focus;
}
int pomodoro_get_session(void) {
  return s_session;
}

void pomodoro_get_remaining(int *min_out, int *sec_out) {
  int64_t now = esp_timer_get_time();
  int64_t elapsed = now - s_stage_start_us;
  int64_t left = (elapsed >= 0 && elapsed < s_stage_us) ? (s_stage_us - elapsed) : 0;

  int total_sec = (int)(left / 1000000LL);
  if (min_out)
    *min_out = total_sec / 60;
  if (sec_out)
    *sec_out = total_sec % 60;
}
