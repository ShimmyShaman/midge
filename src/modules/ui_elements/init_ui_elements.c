
#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

int init_ui_elements(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/ui_elements/button.h"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/panel.h"));
  // MCcall(mcs_interpret_file("src/modules/ui_elements/stack_container.h"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/textblock.h"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/textbox.h"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/ui_elements.h"));

  MCcall(mcs_interpret_file("src/modules/ui_elements/button.c"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/panel.c"));
  // MCcall(mcs_interpret_file("src/modules/ui_elements/stack_container.c"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/textblock.c"));
  MCcall(mcs_interpret_file("src/modules/ui_elements/textbox.c"));

  return 0;
}