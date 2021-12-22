/* app_modules.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "tinycc/libtccinterp.h"

#include "core/core_definitions.h"
#include "core/mc_source.h"
#include "core/midge_app.h"
#include "m_threads.h"
#include "mc_error_handling.h"
#include "mc_str.h"

#include "core/app_modules.h"

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
    // Framework Components
    "mc_io",
    "collections",
    "render_utilities",
    "ui_elements",
    "dialogs",
    "obj_loader",

    // Active Modules
      // "welcome_window",
      // "modus_operandi",
      // "hierarchy_viewer",
      // "project_explorer",
      // And all before...
      "source_editor",
      "commander",
      
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
  printf("Loading Project '%s'...\n", project_name);

  midge_app_info *app_info;
  mc_obtain_midge_app_info(&app_info);

  char buf[512];
  mc_project_info *project = malloc(sizeof(mc_project_info));
  project->source_files.capacity = project->source_files.count = 0U;

  project->name = strdup(project_name);
  sprintf(buf, "%s/%s", base_path, project_name);
  project->path = strdup(buf);

  // Sub-Folders
  strcat(buf, "/src");
  project->path_src = strdup(buf);
  sprintf(buf, "%s/%s", project->path, ".mprj");
  project->path_mprj_data = strdup(buf);

  // Check
  if (access(project->path, F_OK) == -1) {
    MCerror(2135, "Cannot find folder '%s' for project '%s'", project->path, project_name);
  }
  if (access(project->path_mprj_data, F_OK) == -1) {
    // puts("Cannot find midge-config for project '%s'. Creating it...", project_name);
    mkdir(project->path_mprj_data, 0700);
    // MCerror(2137,
    //         "Cannot find midge-config-folder '%s' for project '%s'\n TODO find a way to create it, atm requires "
    //         "build_list to be appropriately set also TODO",
    //         project->path_mprj_data, project_name);
  }

  // Load the source
  // Find the build list
  char *bltxt;
  sprintf(buf, "%s/build_list", project->path_mprj_data);
  MCcall(read_file_text(buf, &bltxt));
  // puts(bltxt);

  // Parse each file in the build list
  char *c = &bltxt[0], *s;
  int wd;
  for (; *c != '\0'; ++c) {
    sprintf(buf, "%s/", project->path_src);
    s = &buf[0] + strlen(buf);
    for (; *c != '\0' && *c != '\n'; ++c) {
      *s = *c;
      ++s;
    }
    *s = '\0';
    // if (access(buf, F_OK) == -1) {
    //   MCerror(1998,
    //           "Loading project='%s'. Could not find This could not be accessed for project_name='%s'",
    //           project_name);
    // }

    mc_source_file_info *sf;
    MCcall(mcs_interpret_source_file(buf, &sf));

    MCcall(append_to_collection((void ***)&project->source_files.items, &project->source_files.capacity,
                                &project->source_files.count, sf));

    wd = inotify_add_watch(app_info->inotify_fd, sf->filepath, IN_MODIFY);
    if (wd < 0) {
      MCerror(8472, "Error adding watch descriptor for:'%s'", sf->filepath);
    }
    if (wd >= app_info->wds_size) {
      if (wd > app_info->wds_size + 5) {
        MCerror(8527, "Need to change the way this is allocated if this is the case wd=%i", wd);
      }

      // Realloc
      app_info->wds =
          (mc_source_file_info **)realloc(app_info->wds, app_info->wds_size * 2 * sizeof(mc_source_file_info *));
      if (app_info->wds) {
        MCerror(7709, "TODO");
      }
      for (int a = app_info->wds_size; a < app_info->wds_size * 2; ++a)
        app_info->wds[a] = NULL;
      app_info->wds_size *= 2;
    }
    app_info->wds[wd] = sf;

    if (*c == '\0')
      break;
  }
  free(bltxt);

  // MCerror(5252, "TODO");

  // Temporary fixup - source load
  // Load main_init.h && main_init.c
  // sprintf(buf, "%s/app/initialize_%s.h", project->path_src, project_name);
  // if (access(buf, F_OK) == -1) {
  //   MCerror(1998,
  //           "Within each projects src folder there must be a file in a folder named 'app' named "
  //           "'{project_name}.h' : This could not be accessed for project_name='%s'",
  //           project_name);
  // }

  // Check
  sprintf(buf, "%s/app/%s.c", project->path_src, project_name);
  if (access(buf, F_OK) == -1) {
    MCerror(1999,
            "Within each projects src folder there must be a file in a folder named 'app' named "
            "'{project_name}.c' : This could not be accessed for project_name='%s'",
            project_name);
  }

  sprintf(buf, "initialize_%s", project_name);
  int (*initialize_project)(mc_node *) = tcci_get_symbol(app_info->itp_data->interpreter, buf);
  if (!initialize_project) {
    MCerror(1999,
            "Within the projects src/app/{project_name}.c file there must be a function "
            "with the signature 'int %s(mc_node *)' : This could not be accessed for project_name='%s'",
            buf, project_name);
  }

  mc_node *project_root;
  sprintf(buf, "%s_root", project_name);
  MCcall(mca_init_mc_node(NODE_TYPE_VISUAL_PROJECT, buf, &project_root));
  MCcall(initialize_project(project_root));

  MCcall(mca_attach_node_to_hierarchy(app_info->global_node, project_root));

  project->root_node = project_root;
  MCcall(mca_register_loaded_project(project));

  app_info->projects.active = project;
  MCcall(mca_focus_node(project->root_node));

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

void *_mca_load_project_async_thread(void *state)
{
  midge_error_set_thread_name("load-project-async-thread");

  char *project_parent_dir = (char *)((void **)state)[0];
  char *project_name = (char *)((void **)state)[1];

  int res = _mca_load_project(project_parent_dir, project_name);
  if (res) {
    printf("--"
           "_mca_load_project"
           "line:%i:ERR:%i\n",
           __LINE__ - 5, res);
    printf("--"
           "_mca_load_project_async_thread"
           "line:??:ERR:%i\n",
           res);
  }

  // At least cleanup state
  free(project_parent_dir);
  free(project_name);
  free((void **)state);

  return NULL;
}

int mca_load_project_async(const char *project_parent_dir, char *project_name)
{
  // Have to just create a thread and use it soley for the project atm TODO
  // Probably best to just have a loader thread on standby -- Who knows, not me, not yet.
  void **state = (void **)malloc(sizeof(void *) * 2);
  state[0] = (void *)strdup(project_parent_dir);
  state[1] = (void *)strdup(project_name);

  mthread_info *thread;
  MCcall(begin_mthread(&_mca_load_project_async_thread, &thread, state));

  // TODO -- once thread ends there is no cleanup of thread info!!! bad bad bad issue#11

  return 0;
}

int mca_load_previously_open_projects()
{
  const char *opl_filepath = "projects/open_project_list";
  if (access(opl_filepath, F_OK) == -1) {
    // Create it
    MCcall(save_text_to_file(opl_filepath, ""));
  }

  char *open_list_text;
  MCcall(read_file_text(opl_filepath, &open_list_text));

  // printf("open_list_text:'%s'\n", open_list_text);

  // The current working directory
  char projects_dir[256];
  char *cdr = getcwd(projects_dir, 256); // TODO sizeof pointer instead of type in mc
  if (!cdr) {
    puts("5260 - TODO - error handling");
    return 5260;
  }

  char c = projects_dir[strlen(projects_dir) - 1];
  if (c != '\\' && c != '/')
    strcat(projects_dir, "/");
  strcat(projects_dir, "projects");

  char project_name[256];
  mc_str *str;
  mc_alloc_str(&str);

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
      strncpy(project_name, open_list_text + s, i - s);
      project_name[i - s] = '\0';

      MCcall(_mca_load_project("projects", project_name));
      // _mca_set_project_state("projects", buf); TODO
    }
  }

  return 0;
}