
#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

#ifndef MCitpCall
#define MCitpCall(itp_function, error_message)                                         \
  {                                                                                    \
    mc_app_itp_data *itp_data;                                                         \
    mc_obtain_app_itp_data(&itp_data);                                                 \
    int (*itp_function)(void) = tcci_get_symbol(itp_data->interpreter, #itp_function); \
    if (!itp_function) {                                                               \
      MCerror(3456, #error_message);                                                   \
    }                                                                                  \
    MCcall(itp_function());                                                            \
  }
#endif

int init_ui_elements(mc_node *app_root)
{
  // MCcall(mcs_interpret_file("src/modules/source_editor/source_editor.h"));
  // MCcall(mcs_interpret_file("src/modules/source_editor/source_line.c"));
  // MCcall(mcs_interpret_file("src/modules/source_editor/function_editor.c"));
  // MCcall(mcs_interpret_file("src/modules/source_editor/source_editor_pool.c"));

  // MCitpCall(mce_init_source_editor_pool, "mce_init_source_editor_pool");

  // MCcall(mce_init_source_editor_pool());

  return 0;
}