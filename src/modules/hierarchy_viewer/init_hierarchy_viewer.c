/* init_hierarchy_viewer.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

int init_hierarchy_viewer(mc_node *app_root)
{
  // MCcall(mcs_interpret_file("src/modules/hierarchy_viewer/hierarchy_viewer_window.h"));
  MCcall(mcs_interpret_file("src/modules/hierarchy_viewer/hierarchy_viewer.c"));

  //   MCcall(mcs_interpret_file("src/modules/source_editor/source_line.c"));
  //   MCcall(mcs_interpret_file("src/modules/source_editor/function_editor.c"));
  //   MCcall(mcs_interpret_file("src/modules/source_editor/source_editor_pool.c"));

  mc_app_itp_data *itp_data;
  mc_obtain_app_itp_data(&itp_data);
  int (*mcm_init_hierarchy_viewer)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "mcm_init_hierarchy_viewer");
  if (!mcm_init_hierarchy_viewer) {
    MCerror(3456, "mcm_init_hierarchy_viewer");
  }
  MCcall(mcm_init_hierarchy_viewer(app_root));

  return 0;
}