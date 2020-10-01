#include "core/core_definitions.h"
#include "env/environment_definitions.h"

void init_obj_loader(mc_node *app_root)
{
    // instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/hash_table.h", NULL);
    // instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/hash_table.h", NULL);
    // instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/obj_loader.h", NULL);
    // instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/obj_loader.c", NULL);
    // instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/model_loading.c", NULL);

  instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/wvf_obj_loader.h", NULL);
  instantiate_all_definitions_from_file(app_root, "src/modules/obj_loader/wvf_obj_loader.c", NULL);
}