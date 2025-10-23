#include <Arduino.h>

#include "lgfx.h"
#include "lvgl.h"
#include "ui.h"

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

lv_color_t *buf;
LGFX tft;

static hw_timer_t *timer = NULL;
static const uint16_t timerFreqHz = 10000; // timer clock = 10 kHz
bool tick = false;
int32_t seconds = 59;
int32_t minuts = 30;
static uint32_t last = 0;

void displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void lvglInit();

void IRAM_ATTR isrTimer() {
  tick = true;
}

void setup() {
  Serial.begin(9600);
  tft.init();
  tft.fillScreen(TFT_BLACK);

  lvglInit();

  pinMode(BLK_GPIO, OUTPUT);
  digitalWrite(BLK_GPIO, HIGH);
  timer = timerBegin(timerFreqHz);
  timerAttachInterrupt(timer, &isrTimer);
  timerAlarm(timer, timerFreqHz, true, 0);
  timerStart(timer);
}

void loop() {
  uint32_t now = millis() - last;

  if (now >= 5) {
    lv_tick_inc(now);
    lv_timer_handler();
    last = now;
  }

  if (tick) {
    Serial.print("I'm ticking ");
    Serial.println(seconds);

    tick = false;

    if (seconds <= 0) {
      seconds = 59;
      minuts--;
      lv_label_set_text_fmt(ui_labelMinuts, "%d", minuts);

    } else {
      lv_label_set_text_fmt(ui_labelSeconds, "%d", seconds);
    }

    seconds--;
  }
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