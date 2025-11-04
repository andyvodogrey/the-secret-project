#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

void encoder_init(void);
void encoder_lvgl_read(lv_indev_drv_t *drv, lv_indev_data_t *data);

void encoder_set_debounce_ms(int ms);
void encoder_reset(void);

#ifdef __cplusplus
}
#endif
