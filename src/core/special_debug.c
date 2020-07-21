/* special_debug.c */

#include "core/midge_core.h"

// [_mc_version=3]
struct special_data {
  int num;
  int add;
};

// [_mc_iteration=3]
void special_modification(special_data *data)
{
  // This stuff
  printf("special_modification() : number=%i\n", data->num);
  data->num += data->add;
}

// [_mc_iteration=12]
void special_update(frame_time *elapsed)
{
  // printf("special_update! @ %li appsecs\n", elapsed->app_secs);
  special_data s;
  s.num = 8;
  s.add = 14;
  s.num = 3;
  s.num = 1;
  // special_modification(&s);
  // printf("special_update! s.num:%i\n", s.num);

  // printf("global_node_name:%s\n", command_hub->global_node->name);
}