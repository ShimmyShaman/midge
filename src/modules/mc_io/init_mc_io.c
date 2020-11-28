
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

int init_mc_io(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_file.h"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_io.h"));

  MCcall(mcs_interpret_file("src/modules/mc_io/mc_file.c"));

  // MCitpCall(mce_init_source_editor_pool, "mce_init_source_editor_pool");

  return 0;
}