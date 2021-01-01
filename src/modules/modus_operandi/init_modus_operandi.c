/* init_modus_operandi.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

// TODO -- rename these initialize module methods to init_module and load & delete them after each module

int init_modus_operandi(mc_node *app_root)
{
  // Initialize the process step dialog
  MCcall(mcs_interpret_file("src/modules/modus_operandi/mo_types.h"));
  MCcall(mcs_interpret_file("src/modules/modus_operandi/mo_util.h"));
  MCcall(mcs_interpret_file("src/modules/modus_operandi/process_step_dialog.h"));
  MCcall(mcs_interpret_file("src/modules/modus_operandi/mo_serialization.h"));

  MCcall(mcs_interpret_file("src/modules/modus_operandi/mo_util.c"));
  MCcall(mcs_interpret_file("src/modules/modus_operandi/process_step_dialog.c"));
  MCcall(mcs_interpret_file("src/modules/modus_operandi/mo_serialization.c"));
  MCcall(mcs_interpret_file("src/modules/modus_operandi/modus_operandi.c"));

  mc_app_itp_data *itp_data;
  mc_obtain_app_itp_data(&itp_data);

  int (*init_modus_operandi_system)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "init_modus_operandi_system");
  if (!init_modus_operandi_system) {
    MCerror(5838, "init_modus_operandi_system");
  }
  MCcall(init_modus_operandi_system(app_root));

  return 0;
}