#include "midge_core.h"

void special_update(frame_time *elapsed) {
  printf("special_update! @ %li app secs\n", elapsed->app_secs * 5);
}