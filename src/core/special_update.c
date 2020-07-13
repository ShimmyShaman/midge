/* special_update.c */

#include "core/midge_core.h"

// [_mc_iteration=5]
void special_update(frame_time *elapsed) {
// printf("special_update! @ %li appsecs\n", elapsed->app_secs);
special_data s;
s.num = 8;
s.add = 14;
//special_modification(&s);
//printf("special_update! s.num:%i\n", s.num);
}