
#include "core/midge_core.h"

int register_update_timer(int (**fnptr_update_callback)(int, void **), uint usecs_period, bool reset_timer_on_update,
                          void *state)
{
  //   register_midge_error_tag("register_update_timer()");
  // printf("register_update_timer0\n");

  update_callback_timer *callback_timer = (update_callback_timer *)malloc(sizeof(update_callback_timer));
  MCcall(append_to_collection((void ***)&command_hub->update_timers.callbacks, &command_hub->update_timers.allocated,
                              &command_hub->update_timers.count, callback_timer));

  clock_gettime(CLOCK_REALTIME, &callback_timer->next_update);
  callback_timer->period = (struct timespec){usecs_period / 1000000, (usecs_period % 1000000) * 1000};
  increment_time_spec(&callback_timer->next_update, &callback_timer->period, &callback_timer->next_update);
  // printf("register_update_timer2\n");
  callback_timer->reset_timer_on_update = true;
  callback_timer->update_delegate = fnptr_update_callback;
  callback_timer->state = state;
  // printf("register_update_timer3\n");

  printf("callback_timer=%p tv-sec=%li\n", callback_timer, callback_timer->next_update.tv_sec);
  printf("callback_timer ic=%p\n", command_hub->update_timers.callbacks[0]);

  //   register_midge_error_tag("register_update_timer(~)");
  return 0;
}