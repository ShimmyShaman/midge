/* init_dialogs.c */

#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

int init_dialogs(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/dialogs/file_dialog.h"));
  MCcall(mcs_interpret_file("src/modules/dialogs/folder_dialog.h"));
  MCcall(mcs_interpret_file("src/modules/dialogs/text_input_dialog.h"));
  MCcall(mcs_interpret_file("src/modules/dialogs/options_dialog.h"));

  MCcall(mcs_interpret_file("src/modules/dialogs/file_dialog.c"));
  MCcall(mcs_interpret_file("src/modules/dialogs/folder_dialog.c"));
  MCcall(mcs_interpret_file("src/modules/dialogs/text_input_dialog.c"));
  MCcall(mcs_interpret_file("src/modules/dialogs/options_dialog.c"));

  // TODO make an anonymous data register system
  // function_debug_pool *pool = (function_debug_pool *)malloc(sizeof(function_debug_pool));
  // global_data->ui_state->function_debug_pool = (mce_function_debug_pool *)malloc(sizeof(mce_function_debug_pool));
  // mca_register_global_data_pointer("FUNCTION_DEBUG_POOL", pool);

  mc_app_itp_data *itp_data;
  mc_obtain_app_itp_data(&itp_data);

  int (*mc_fd_init_file_dialog)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "mc_fd_init_file_dialog");
  if (!mc_fd_init_file_dialog) {
    MCerror(3463, "mc_fd_init_file_dialog");
  }
  MCcall(mc_fd_init_file_dialog(app_root));

  int (*mc_fd_init_folder_dialog)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "mc_fd_init_folder_dialog");
  if (!mc_fd_init_folder_dialog) {
    MCerror(3463, "mc_fd_init_folder_dialog");
  }
  MCcall(mc_fd_init_folder_dialog(app_root));

  int (*mc_tid_init_text_input_dialog)(mc_node *) =
      tcci_get_symbol(itp_data->interpreter, "mc_tid_init_text_input_dialog");
  if (!mc_tid_init_text_input_dialog) {
    MCerror(3464, "mc_tid_init_text_input_dialog");
  }
  MCcall(mc_tid_init_text_input_dialog(app_root));

  int (*mc_obd_init_options_dialog)(mc_node *) = tcci_get_symbol(itp_data->interpreter, "mc_obd_init_options_dialog");
  if (!mc_obd_init_options_dialog) {
    MCerror(3463, "mc_obd_init_options_dialog");
  }
  MCcall(mc_obd_init_options_dialog(app_root));

  return 0;
}