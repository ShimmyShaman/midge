/* mc_projects.c */

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>

#include <sys/stat.h>
#include <unistd.h>

#include "midge_error_handling.h"

#include "core/core_definitions.h"
#include "env/environment_definitions.h"
#include "mc_str.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_projects.h"

int _mc_construct_project_file_header(const char *subdir, const char *project_name)
{
  mc_str *str;
  MCcall(init_mc_str(&str));

  // Preamble
  MCcall(append_to_mc_strf(str, "/* %s.h */\n", project_name));

  // Include Guard
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#ifndef "));
  char *c = (char *)&project_name[0];
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H\n"
                               "#define "));
  c = (char *)project_name;
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H\n"));

  // Includes
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#include \"core/core_definitions.h\"\n"));

  // Basic App-Data Structure
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_strf(str,
                           "typedef struct %s_data {\n"
                           "  mc_node *app_root;\n"
                           "} %s_data;\n",
                           project_name, project_name));

  // Function Declaration
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_strf(str, "/* %s-Initialization */\n", project_name));
  MCcall(append_to_mc_strf(str, "int initialize_%s(mc_node *app_root);\n", project_name));

  // #Endif
  MCcall(append_to_mc_str(str, "\n#endif // "));
  c = (char *)project_name;
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H"));

  // Write the file
  char path[256];
  sprintf(path, "%s/%s.h", subdir, project_name);
  MCcall(save_text_to_file(path, str->text));

  // Cleanup & return
  release_mc_str(str, true);
  return 0;
}

int _mc_construct_project_file_source(const char *subdir, const char *project_name)
{
  mc_str *str;
  MCcall(init_mc_str(&str));

  // Preamble
  MCcall(append_to_mc_strf(str, "/* %s.c */\n", project_name));

  // Base System Includes
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#include <stdlib.h>\n"));

  // Midge Includes
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#include \"core/midge_app.h\"\n"));
  MCcall(append_to_mc_str(str, "#include \"render/render_common.h\"\n"));

  // Initialize Header
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_strf(str, "#include \"../projects/%s/src/app/%s.h\"\n", project_name, project_name));

  // Basic Render Function
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_strf(
      str,
      "void _%s_render_present(image_render_details *image_render_queue, mc_node *node)\n"
      "{\n"
      "  %s_data *data = (%s_data *)node->data;\n"
      "\n"
      "  mcr_issue_render_command_colored_quad(\n"
      "    image_render_queue, (unsigned int)node->layout->__bounds.x, (unsigned int)node->layout->__bounds.y,\n"
      "    (unsigned int)node->layout->__bounds.width, (unsigned int)node->layout->__bounds.height, \n"
      "    (render_color){0.f, 0.15f, 0.17f, 1.f});\n"
      "}\n",
      project_name, project_name, project_name));

  // Initialize Definition
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(
      append_to_mc_strf(str,
                        "int initialize_%s(mc_node *app_root)\n"
                        "{\n"
                        "  mca_init_node_layout(&app_root->layout);\n"
                        "  app_root->children = (mc_node_list *)malloc(sizeof(mc_node_list));\n"
                        "  app_root->children->count = 0;\n"
                        "  app_root->children->alloc = 0;\n"
                        "\n"
                        "  app_root->layout->z_layer_index = 4U;\n"
                        "  app_root->layout->padding = (mc_paddingf){301, 2, 2, 2};\n"
                        "\n"
                        "  app_root->layout->determine_layout_extents = (void *)&mca_determine_typical_node_extents;\n"
                        "  app_root->layout->update_layout = (void *)&mca_update_typical_node_layout;\n"
                        "  app_root->layout->render_headless = NULL;\n"
                        "  app_root->layout->render_present = (void *)&_%s_render_present;\n"
                        "\n"
                        "  // Global Data\n"
                        "  %s_data *data = (%s_data *)malloc(sizeof(%s_data));\n"
                        "  app_root->data = data;\n"
                        "  data->app_root = app_root;\n"
                        "\n"
                        "  return 0;\n"
                        "}",
                        project_name, project_name, project_name, project_name, project_name));

  // Write the file
  char path[256];
  sprintf(path, "%s/%s.c", subdir, project_name);
  MCcall(save_text_to_file(path, str->text));

  // Cleanup & return
  release_mc_str(str, true);
  return 0;
}

int _mc_construct_mprj_directory(const char *project_directory, const char *project_name)
{
  char path[256], buf[256];

  // Make the mcfg directory
  strcpy(path, project_directory);
  MCcall(mcf_concat_filepath(path, 256, ".mprj"));
  if (mkdir(path, 0700)) {
    MCerror(5442, "TODO - error handling");
  }

  sprintf(buf,
          "app/%s.h\n"
          "app/%s.c\n",
          project_name, project_name);
  MCcall(mcf_concat_filepath(path, 256, "build_list"));
  MCcall(save_text_to_file(path, buf));

  void **vargs = (void *)malloc(sizeof(void *) * 2);
  vargs[0] = strdup(project_directory);
  vargs[1] = strdup(project_name);
  MCcall(mca_fire_event_and_release_data(MC_APP_EVENT_PROJECT_STRUCTURE_CREATION, (void *)vargs, 3, vargs[0], vargs[1],
                                         vargs));

  return 0;
}

int mcf_create_project_file_structure(const char *parent_directory, const char *project_name)
{
  // Determine valid directory for parent_directory
  bool exists;
  MCcall(mcf_directory_exists(parent_directory, &exists));
  if (!exists) {
    // Do nothing more
    MCerror(5420, "TODO - error handling");
  }

  char path[256];

  strcpy(path, parent_directory);
  MCcall(mcf_concat_filepath(path, 256, project_name));
  MCcall(mcf_directory_exists(path, &exists));
  if (exists) {
    // Do nothing more
    MCerror(5436, "TODO - Project Directory Already Exists!");
  }

  // Make the directory
  if (mkdir(path, 0700)) {
    MCerror(5441, "TODO - error handling");
  }

  // -- Make the midge-project config folder
  MCcall(_mc_construct_mprj_directory(path, project_name));

  // Make the src directory
  MCcall(mcf_concat_filepath(path, 256, "src"));
  if (mkdir(path, 0700)) {
    MCerror(5444, "TODO - error handling");
  }

  // Make the app sub-src-directory
  MCcall(mcf_concat_filepath(path, 256, "app"));
  if (mkdir(path, 0700)) {
    MCerror(5441, "TODO - error handling");
  }

  // Make the main.c file
  // _mc_construct_project_main_source_file(subdir, project_name);

  // Make the initialize.h file
  MCcall(_mc_construct_project_file_header(path, project_name));

  // Make the initialize.c file
  MCcall(_mc_construct_project_file_source(path, project_name));

  return 0;
}