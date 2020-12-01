
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
  MCcall(mcs_interpret_file("src/modules/ui_elements/button.h"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/textbox.h"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/ui_elements.h"));

  MCcall(mcs_interpret_file("src/modules/ui_elements/button.c"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/textbox.c"));

  // MCitpCall(mce_init_source_editor_pool, "mce_init_source_editor_pool");

  return 0;
}