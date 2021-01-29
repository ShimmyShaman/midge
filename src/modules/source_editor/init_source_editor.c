
#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

#include "modules/source_editor/source_editor.h"
#include "modules/source_editor/modification_watcher.h"

#define MCitpCall(itp_function, app_root, error_message)                                    \
  {                                                                                         \
    mc_app_itp_data *itp_data;                                                              \
    mc_obtain_app_itp_data(&itp_data);                                                      \
    int (*itp_function)(mc_node *) = tcci_get_symbol(itp_data->interpreter, #itp_function); \
    if (!itp_function) {                                                                    \
      MCerror(3456, #error_message);                                                        \
    }                                                                                       \
    MCcall(itp_function(app_root));                                                         \
  }

int init_source_editor(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/source_editor/source_editor.h"));
  MCcall(mcs_interpret_file("src/modules/source_editor/modification_watcher.h"));
  MCcall(mcs_interpret_file("src/modules/source_editor/source_editor.c"));
  MCcall(mcs_interpret_file("src/modules/source_editor/modification_watcher.c"));
  // MCcall(mcs_interpret_file("src/modules/source_editor/function_editor.c"));
  // MCcall(mcs_interpret_file("src/modules/source_editor/source_editor_pool.c"));

  MCitpCall(mc_se_init_source_editor, app_root, "mc_se_init_source_editor");
  MCitpCall(mc_se_init_modification_watcher, app_root, "mc_se_init_modification_watcher");

  return 0;
}