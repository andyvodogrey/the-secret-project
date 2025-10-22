#include <Arduino.h>

#include "lgfx.h"
#include "lvgl.h"
#include "ui.h"

static lv_disp_draw_buf_t draw_buf;
static lv_disp_drv_t disp_drv;

lv_color_t *buf;
LGFX tft;

#define BLK_GPIO PIN_LCD_BACKLIGHT

void displayFlush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p);
void lvglInit();

uint32_t last_tick = xTaskGetTickCount();

void setup() {
  tft.init();
  tft.fillScreen(TFT_BLACK);

  lvglInit();

  pinMode(BLK_GPIO, OUTPUT);
  digitalWrite(BLK_GPIO, HIGH);
}

void loop() {
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