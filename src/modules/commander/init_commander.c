/* init_collections.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

// #include "modules/function_debug/function_debug.h"

// extern "C" {
// }

int init_commander(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/commander/commander.c"));

  mc_app_itp_data *itp_data;
  mc_obtain_app_itp_data(&itp_data);

  int (*init_commander_system)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "init_commander_system");
  if (!init_commander_system) {
    MCerror(5839, "init_commander_system");
  }
  MCcall(init_commander_system(app_root));

  return 0;
}