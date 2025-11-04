#pragma once
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void pomodoro_init(void);
void pomodoro_start(void);
void pomodoro_stop(void);
bool pomodoro_running(void);

bool pomodoro_is_focus(void);
int pomodoro_get_session(void);
void pomodoro_get_remaining(int *min_out, int *sec_out);

void pomodoro_set_durations_minutes(int focus_min, int break_min);

#ifdef __cplusplus
}
#endif
