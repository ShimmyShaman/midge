
#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

int init_mc_io(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_file.h"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_projects.h"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_info_transcription.h"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_source_extensions.h"));

  MCcall(mcs_interpret_file("src/modules/mc_io/mc_file.c"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_projects.c"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_info_transcription.c"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_source_extensions.c"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_function_manipulation.c"));

  // // DEBUG
  // mc_app_itp_data *itp_data;
  // mc_obtain_app_itp_data(&itp_data);

  // int (*debug_test_code_cursor)() = tcci_get_symbol(itp_data->interpreter, "debug_test_code_cursor");
  // if (!debug_test_code_cursor) {
  //   MCerror(4245, "debug_test_code_cursor");
  // }
  // MCcall(debug_test_code_cursor());
  // // DEBUG

  return 0;
}