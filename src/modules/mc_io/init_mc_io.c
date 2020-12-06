
#include <stdio.h>

#include "core/core_definitions.h"
#include "core/mc_source.h"

int init_mc_io(mc_node *app_root)
{
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_file.h"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_projects.h"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_io.h"));

  MCcall(mcs_interpret_file("src/modules/mc_io/mc_file.c"));
  MCcall(mcs_interpret_file("src/modules/mc_io/mc_projects.c"));

  return 0;
}