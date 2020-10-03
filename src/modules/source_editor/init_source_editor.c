#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "render/render_common.h"

extern "C" {
void mca_init_source_editor_pool();
}

void init_source_editor(mc_node *app_root)
{
  instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_editor.h", NULL);
  instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_editor_pool.c", NULL);
  instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/source_line.c", NULL);
  instantiate_all_definitions_from_file(app_root, "src/modules/source_editor/function_editor.c", NULL);

  mca_init_source_editor_pool();
}