/* app_modules.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <unistd.h>

#include "tinycc/libtccinterp.h"

#include "core/core_definitions.h"
#include "core/mc_source.h"
#include "core/midge_app.h"
#include "mc_str.h"
#include "midge_error_handling.h"

int _mca_load_module(const char *base_path, const char *module_name)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  char buf[512];
  sprintf(buf, "%s/%s/init_%s.c", base_path, module_name, module_name);
  if (access(buf, F_OK) == -1) {
    MCerror(1999,
            "Within each module there must be a file named 'init_{%%module_name%%}.c' : This could not be accessed for "
            "module_name='%s'",
            module_name);
  }

  MCcall(mcs_interpret_file(buf));

  // Initialize the module
  sprintf(buf, "init_%s", module_name);
  int (*initialize_module)(mc_node *) = tcci_get_symbol(app_info->itp_data->interpreter, buf);
  if (!initialize_module) {
    MCerror(2000,
            "within each 'init_{%%module_name%%}.c' file there must be a method with signature 'int "
            "init_{%%module_name%%}(mc_node *)' : This was not found for module_name='%s'",
            module_name);
  }

  // TODO -- for some reason interpreting from this function loads the functions to address ranges I normally see
  // reserved for stack variables. Find out whats happenning -- that can't be good
  // printf("interpreter=%p\n",app_info->itp_data->interpreter);
  // printf("%s=%p\n", buf, initialize_module);
  MCcall(initialize_module(app_info->global_node));

  return 0;
}

int mca_load_modules()
{
  // puts("MODULE SKIP TODO mca_load_modules");
  // // Get all directories in folder
  // // TODO
  const char *module_directories[] = {
      "mc_io",
      "ui_elements",
      "welcome_window",
      "modus_operandi",
      "source_editor",
      // "function_debug",
      "obj_loader",
      NULL,
  };

  for (int d = 0; module_directories[d]; ++d) {
    MCcall(_mca_load_module("src/modules", module_directories[d]));
  }

  return 0;
}

int _mca_set_project_state(char *base_path, char *module_name)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  char buf[512];
  sprintf(buf, "set_%s_project_state", module_name);
  int (*set_project_state)(mc_node *) = tcci_get_symbol(app_info->itp_data->interpreter, buf);
  if (!set_project_state) {
    MCerror(2000,
            "within each '{%%project_name%%}' project there must be a method with signature 'int "
            "set_{%%project_name%%}(mc_node *)' : This was not found for project_name='%s'",
            module_name);
  }

  MCcall(set_project_state(app_info->global_node));

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

  return 0;
}

int _mca_load_project(const char *base_path, const char *project_name)
{
  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  char buf[512];
  sprintf(buf, "%s/%s/src", base_path, project_name);

  // Load the source
  // TODO - hard problem ( have to load headers than .c files but do it in order)
  // DIR *dir;
  // struct dirent *ent;
  // if ((dir = opendir(buf)) != NULL) {
  //   /* print all the files and directories within directory */
  //   while ((ent = readdir(dir)) != NULL) {
  //     printf("%s\n", ent->d_name);
  //   }
  //   closedir(dir);
  // }
  // else {
  //   /* could not open directory */
  //   perror("");
  //   // return EXIT_FAILURE;
  // }

  // TEMP
  mc_project_info *project = malloc(sizeof(mc_project_info));
  project->path_src = strdup(buf);

  // Temporary fixup - source load
  // Load main_init.h && main_init.c
  sprintf(buf, "%s/app/initialize_%s.h", project->path_src, project_name);
  if (access(buf, F_OK) == -1) {
    MCerror(1998,
            "Within each projects src folder there must be a file in a folder named 'app' named "
            "'initialize_{project_name}.h' : This could not be accessed for project_name='%s'",
            project_name);
  }
  MCcall(mcs_interpret_file(buf));

  sprintf(buf, "%s/app/initialize_%s.c", project->path_src, project_name);
  if (access(buf, F_OK) == -1) {
    MCerror(1999,
            "Within each projects src folder there must be a file in a folder named 'app' named "
            "'initialize_{project_name}.c' : This could not be accessed for project_name='%s'",
            project_name);
  }
  MCcall(mcs_interpret_file(buf));

  sprintf(buf, "initialize_%s", project_name);
  int (*initialize_project)(void) = tcci_get_symbol(app_info->itp_data->interpreter, buf);
  if (!initialize_project) {
    MCerror(1999,
            "Within the projects src/app/initialize_{project_name}.c file there must be a function "
            "with the signature 'int %s(void)' : This could not be accessed for project_name='%s'",
            buf, project_name);
  }

  MCcall(initialize_project());

  // MCcall(mcs_interpret_file(buf));

  // // Initialize the module
  // sprintf(buf, "main", project_name);
  // int (*initialize_module)(mc_node *) = tcci_get_symbol(app_info->itp_data->interpreter, buf);
  // if (!initialize_module) {
  //   MCerror(2000,
  //           "within each 'init_{%%project_name%%}.c' file there must be a method with signature 'int "
  //           "init_{%%project_name%%}(mc_node *)' : This was not found for project_name='%s'",
  //           project_name);
  // }

  // // TODO -- for some reason interpreting from this function loads the functions to address ranges I normally see
  // // reserved for stack variables. Find out whats happenning -- that can't be good
  // // printf("interpreter=%p\n",app_info->itp_data->interpreter);
  // // printf("%s=%p\n", buf, initialize_module);
  // MCcall(initialize_module(app_info->global_node));

  return 0;
}

int mca_load_open_projects()
{
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

      MCcall(_mca_load_project("projects", buf));
      // _mca_set_project_state("projects", buf);
    }
  }

  return 0;
}