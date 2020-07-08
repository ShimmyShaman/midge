#include "midge_core.h"

int special_update_v1(int argc, void **argv)
{
  /*mcfuncreplace*/
  mc_command_hub_v1 *command_hub;
  /*mcfuncreplace*/

  frame_time *elapsed = *(frame_time **)argv[0];

  printf("special_update! @ %li app secs\n", elapsed->app_secs * 5);

  return 0;
}
