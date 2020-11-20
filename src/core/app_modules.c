/* app_modules.c */

#include <stdio.h>
#include <string.h>

#include "core/core_definitions.h"
#include "mc_str.h"

void _mca_load_module(char *base_path, char *module_name)
{
  puts("MODULE SKIP TODO _mca_load_module");
  return;
  // mc_global_data *global_data;
  // obtain_midge_global_root(&global_data);

  // char buf[512];
  // sprintf(buf, "%s/%s/init_%s.c", base_path, module_name, module_name);

  // // instantiate_all_definitions_from_file(global_data->global_node, buf, NULL);

  // // Initialize the module
  // int mc_res;
  // sprintf(buf,
  //         "{\n"
  //         "  void *mc_vargs[1];\n"
  //         "  mc_vargs[0] = (void *)%p;\n"
  //         "  (*(int *)%p) = init_%s(1, mc_vargs);\n"
  //         "}\n",
  //         &global_data->global_node, &mc_res, module_name);
  // clint_process(buf);
  // if (mc_res) {
  //   MCerror(8974, "--init_%s() |line~ :??? ERR:%i\n", module_name, mc_res);
  // }
}

void mca_load_modules()
{
  puts("MODULE SKIP TODO mca_load_modules");
  // // Get all directories in folder
  // // TODO
  // const char *module_directories[] = {
  //     "modus_operandi",
  //     "source_editor",
  //     "function_debug",
  //     "obj_loader",
  //     NULL,
  // };

  // for (int d = 0; module_directories[d]; ++d) {
  //   _mca_load_module("src/modules", module_directories[d]);
  // }
}

void _mca_set_project_state(char *base_path, char *module_name)
{
  // mc_global_data *global_data;
  // obtain_midge_global_root(&global_data);

  // char buf[512];
  // // Initialize the module
  // int mc_res;
  // sprintf(buf,
  //         "{\n"
  //         "  void *mc_vargs[1];\n"
  //         "  mc_vargs[0] = (void *)%p;\n"
  //         "  (*(int *)%p) = set_%s_project_state(1, mc_vargs);\n"
  //         "}\n",
  //         &global_data->global_node, &mc_res, module_name);
  // clint_process(buf);
  // if (mc_res) {
  //   MCerror(8974, "--init_%s() |line~ :??? ERR:%i\n", module_name, mc_res);
  // }
}

void mca_load_open_projects()
{
  return;
  char *open_list_text;
  read_file_text("projects/open_project_list", &open_list_text);

  printf("open_list_text:'%s'\n", open_list_text);

  char buf[256];
  mc_str *str;
  init_mc_str(&str);

  int i = 0, s = 0;
  bool eof = false;
  while (!eof) {
    s = i;

    for (; open_list_text[i] != '|'; ++i)
      if (open_list_text[i] == '\0') {
        eof = true;
        break;
      }

    if (i > s) {
      strncpy(buf, open_list_text + s, i - s);
      buf[i - s] = '\0';

      _mca_load_module("projects", buf);
      _mca_set_project_state("projects", buf);
    }
  }
}