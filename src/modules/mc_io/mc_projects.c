/* mc_projects.c */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>

#include <sys/stat.h>
#include <unistd.h>

#include "midge_error_handling.h"

#include "core/core_definitions.h"
#include "mc_str.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_projects.h"

int _mc_construct_project_initialize_header(const char *subdir, const char *name)
{
  mc_str *str;
  MCcall(init_mc_str(&str));

  // Preamble
  MCcall(append_to_mc_strf(str, "/* initialize_%s.h */\n", name));

  // Include Guard
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#ifndef INITIALIZE_"));
  char *c = (char *)&name[0];
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H\n"
                               "#define INITIALIZE_"));
  c = (char *)name;
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
                           name, name));

  // Function Declaration
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_strf(str, "/* %s-Initiation */\n", name));
  MCcall(append_to_mc_strf(str, "int initialize_%s(mc_node *app_root);\n", name));

  // #Endif
  MCcall(append_to_mc_str(str, "\n#endif // INITIALIZE_"));
  c = (char *)name;
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H"));

  // Write the file
  char path[256];
  sprintf(path, "%sinitialize_%s.h", subdir, name);
  MCcall(save_text_to_file(path, str->text));

  // Cleanup & return
  release_mc_str(str, true);
  return 0;
}

int _mc_construct_project_initialize_source(const char *subdir, const char *name)
{
  mc_str *str;
  MCcall(init_mc_str(&str));

  // Preamble
  MCcall(append_to_mc_strf(str, "/* initialize_%s.c */\n", name));

  // Base System Includes
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#include <stdlib.h>\n"));

  // Midge Includes
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_str(str, "#include \"core/midge_app.h\"\n"));
  MCcall(append_to_mc_str(str, "#include \"render/render_common.h\"\n"));

  // Initialize Header
  MCcall(append_to_mc_str(str, "\n"));
  MCcall(append_to_mc_strf(str, "#include \"../projects/%s/src/app/initialize_%s.h\"\n", name, name));

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
      name, name, name));

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
                        "  app_root->layout->preferred_width = 900;\n"
                        "  app_root->layout->preferred_height = 600;\n"
                        "\n"
                        "  app_root->layout->z_layer_index = 4U;\n"
                        "  app_root->layout->padding = (mc_paddingf){302, 2, 2, 2};\n"
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
                        name, name, name, name, name));

  // Write the file
  char path[256];
  sprintf(path, "%sinitialize_%s.c", subdir, name);
  MCcall(save_text_to_file(path, str->text));

  // Cleanup & return
  release_mc_str(str, true);
  return 0;
}

int mcf_create_project(const char *parent_directory, const char *name)
{
  // Determine valid directory for parent_directory
  bool exists;
  MCcall(mcf_directory_exists(parent_directory, &exists));
  if (!exists) {
    // Do nothing more
    MCerror(5420, "TODO - error handling");
  }

  // Check potential directory does not exist
  char fprdir[256];
  {
    strcpy(fprdir, parent_directory);
    char c = fprdir[strlen(fprdir) - 1];
    if (c != '\\' && c != '/')
      strcat(fprdir, "/");
  }

  char proj_dir[256];
  sprintf(proj_dir, "%s%s/", fprdir, name);
  MCcall(mcf_directory_exists(proj_dir, &exists));
  if (exists) {
    // Do nothing more
    MCerror(5436, "TODO - Project Directory Already Exists!");
  }

  // Make the directory
  if (mkdir(proj_dir, 0700)) {
    MCerror(5441, "TODO - error handling");
  }

  // Make the src directory
  char src_dir[256];
  sprintf(src_dir, "%ssrc/", proj_dir);
  if (mkdir(src_dir, 0700)) {
    MCerror(5444, "TODO - error handling");
  }

  // Make the app sub-src-directory
  char subdir[256];
  sprintf(subdir, "%sapp/", src_dir);
  if (mkdir(subdir, 0700)) {
    MCerror(5441, "TODO - error handling");
  }

  // Make the main.c file
  // _mc_construct_project_main_source_file(subdir, name);

  // Make the initialize.h file
  _mc_construct_project_initialize_header(subdir, name);

  // Make the initialize.c file
  _mc_construct_project_initialize_source(subdir, name);

  return 0;
}