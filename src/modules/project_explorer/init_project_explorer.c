/* init_project_explorer.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

int init_project_explorer(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/project_explorer/project_explorer_window.h"));
  MCcall(mcs_interpret_file("src/modules/project_explorer/project_explorer_window.c"));

  //   MCcall(mcs_interpret_file("src/modules/source_editor/source_line.c"));
  //   MCcall(mcs_interpret_file("src/modules/source_editor/function_editor.c"));
  //   MCcall(mcs_interpret_file("src/modules/source_editor/source_editor_pool.c"));

  //   MCitpCall(mce_init_source_editor_pool, "mce_init_source_editor_pool");

  mc_app_itp_data *itp_data;
  mc_obtain_app_itp_data(&itp_data);
  int (*mcm_init_project_explorer)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "mcm_init_project_explorer");
  if (!mcm_init_project_explorer) {
    MCerror(3456, "mcm_init_project_explorer");
  }
  MCcall(mcm_init_project_explorer(app_root));

  return 0;
}