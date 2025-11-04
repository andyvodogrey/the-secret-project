#include <Arduino.h>

#include "encoder.h"
#include "led_feedback.h"
#include "lgfx.h"
#include "lvgl.h"
#include "pomodoro.h"
#include "ui.h"

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

lv_color_t *buf;
LGFX tft;

static uint32_t last = 0;

void displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void lvglInit();
static void lvgl_pomodoro_tick(lv_timer_t *t);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000)
    ;
  delay(2000);

  tft.init();
  tft.fillScreen(TFT_BLACK);

  // Load your saved durations and pass to Pomodoro
  int32_t fcsTime = loadValue(KEY_FOCUS_TIME);
  if (fcsTime > 0)
    focus_time = fcsTime; // UI mirror
  int32_t brkTime = loadValue(KEY_BREAK_TIME);
  if (brkTime > 0)
    break_time = brkTime; // UI mirror
  Serial.printf("fcsTime %d  brkTime %d\n", fcsTime, brkTime);
  encoder_init();
  led_feedback_init();

  lvglInit();

  last = millis();
}

void loop() {
  uint32_t ms = millis();
  uint32_t elapsed = ms - last;
  if (elapsed > 0) {
    lv_tick_inc(elapsed);
    last = ms;
  }

  lv_timer_handler();
  delay(5);
}

void lvglInit() {
  lv_init();

  // Allocate a draw buffer for LVGL.
  buf = (lv_color_t *)heap_caps_malloc(240 * 10 * sizeof(lv_color_t), MALLOC_CAP_DMA);
  lv_disp_draw_buf_init(&draw_buf, buf, NULL, 240 * 10);

  // Set up the LVGL display driver.
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = TFT_WIDTH;
  disp_drv.ver_res = TFT_HEIGHT;
  disp_drv.flush_cb = displayFlush;
  disp_drv.draw_buf = &draw_buf;
  lv_disp_drv_register(&disp_drv);

  ui_init();
  // #######################################################
  pomodoro_init(); // create esp_timer

  pomodoro_set_durations_minutes(focus_time > 0 ? focus_time : 25, break_time > 0 ? break_time : 5);

  // 1-second LVGL UI updater
  lv_timer_create(lvgl_pomodoro_tick, 1000, NULL);
  // #######################################################

  // Register the encoder driver
  lv_indev_drv_init(&enc_drv);
  enc_drv.type = LV_INDEV_TYPE_ENCODER;
  enc_drv.long_press_time = 500; // in milliseconds
  enc_drv.long_press_repeat_time = 0;
  // enc_drv.long_press_repeat_time = 0;
  // enc_drv.read_cb = lvEncoderRead;
  enc_drv.read_cb = encoder_lvgl_read;
  enc_indev = lv_indev_drv_register(&enc_drv);

  // Attach the group
  group_obj[0] = lv_group_create();
  group_obj[1] = lv_group_create();

  // Group Main Screen
  lv_group_add_obj(group_obj[0], ui_MainScreen);
  // lv_group_add_obj(group_obj[0], ui_labelMinuts);

  // Group Config Screen
  lv_group_add_obj(group_obj[1], ui_ConfScreen);
  lv_group_add_obj(group_obj[1], ui_labelMinConf);
  lv_group_add_obj(group_obj[1], ui_labelConfBreakTime);

  lv_group_focus_obj(ui_MainScreen);
  // Set default group
  lv_group_set_default(group_obj[0]);
  lv_indev_set_group(enc_indev, group_obj[0]);
}

void displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p) {
  int32_t w = (area->x2 - area->x1 + 1);
  int32_t h = (area->y2 - area->y1 + 1);

  tft.startWrite();
  tft.setAddrWindow(area->x1, area->y1, w, h);
  tft.pushPixels((uint16_t *)&color_p->full, w * h, true);
  tft.endWrite();

  lv_disp_flush_ready(disp);
}

static void lvgl_pomodoro_tick(lv_timer_t *t) {
  Serial.printf(" tick\n");
  (void)t;
  // 1) Read current Pomodoro state
  const bool focus_now = pomodoro_is_focus();
  const int sess_now = pomodoro_get_session();

  int min_left, sec_left;
  pomodoro_get_remaining(&min_left, &sec_left);

  // 2) Detect transitions (Focus↔Break OR Session increments)
  //    Keep previous snapshot across calls
  static bool focus_prev = true;
  static int sess_prev = 1;
  static bool first_run = true;

  if (first_run) {
    // Initialize snapshot on the very first tick (no transition yet)
    focus_prev = focus_now;
    sess_prev = sess_now;
    first_run = false;
  } else if (focus_now != focus_prev || sess_now != sess_prev) {
    // -> We just crossed a boundary (stage changed or next session)
    led_feedback_on_transition(focus_now); // flash + short buzz
    focus_prev = focus_now;
    sess_prev = sess_now;
  }

  // 3) Update your UI labels (minutes, seconds, session, title)
  //    (Assumes your helper that formats numbers with leading zero if needed)
  if (pomodoro_running()) {
    Serial.printf("min_left %d sec_left %d sess_now %d\n", min_left, sec_left, sess_now);
    set_custom_label_text(ui_labelMinuts, min_left);
    set_custom_label_text(ui_labelSeconds, sec_left);
    lv_label_set_text_fmt(ui_Label2, "%d", sess_now);
    lv_label_set_text(ui_labelFocus, focus_now ? "Focus" : "Break");
  }

  // 4) Drive LEDs every second (progress bar + color)
  //    Decide the full duration of the current stage in minutes
  const int total_minutes =
      focus_now ? (focus_time > 0 ? focus_time : 25) : (break_time > 0 ? break_time : 5);

  led_feedback_update(focus_now, sess_now, min_left, sec_left, total_minutes);
}