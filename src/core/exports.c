

#include "core/midge_core.h"

size_t save_text_to_file(char *filepath, char *text);

void _export_app_write_main_c(mc_node_v1 *node, mc_str *src, mc_str *path)
{
  console_app_info *app_info = (console_app_info *)node->extra;

  mc_set_str(src, "#include <stdio.h>\n"
                 "\n");

  // OTHER STUFF TEMPORARRY
  mc_append_to_str(src, "typedef struct node {\n"
                       "const char *name;\n"
                       "} node;\n"
                       "\n");

  if (app_info) {
    if (app_info->initialize_app) {
      mc_append_to_str(src, app_info->initialize_app->source->code);
      mc_append_to_str(src, "\n\n");
    }
  }
  // OTHER STUFF TEMPORARRY

  mc_append_to_str(src, "int main()\n"
                       "{\n");

  // -- Node
  mc_append_to_strf(src, "  node app_root;\n", node->name);
  mc_append_to_strf(src, "  app_root.name = \"%s\";\n", node->name);
  mc_append_to_str(src, "\n");

  // -- Initialize
  if (app_info) {
    if (app_info->initialize_app) {
      mc_append_to_strf(src, "  %s(&app_root);\n", app_info->initialize_app->name);
      mc_append_to_str(src, "\n");
    }
  }

  // Cleanup -- TODO

  // Exit
  mc_append_to_str(src, "  return 0;\n");
  mc_append_to_str(src, "}\n");

  // Write to file
  mc_append_to_str(path, "main.c");
  save_text_to_file(path->text, src->text);
}

void export_node_to_application(mc_node_v1 *node, char *directory_path)
{
  if (node->type != NODE_TYPE_CONSOLE_APP) {
    printf("ERROR: NODE TYPE NOT SUPPORTED\n");
    return;
  }

  // console_app_info *app_info = (console_app_info *)node->extra;
  if (!node->extra) {
    printf("ERROR: ARGUMENT EXPECTED 14\n");
    return;
  }

  // Generate the source
  mc_str *src, *path;
  mc_alloc_str(&src);
  mc_alloc_str(&path);

  // FILE:[node.c]
  // mc_append_to_strf(src, "  node %s;\n", node->name);

  // // Write to file
  // mc_set_str(path, directory_path);
  // if (path->text[path->len - 1] != '/')
  //   mc_append_to_str(path, "/");
  // mc_append_to_str(path, "node.c");
  // save_text_to_file(path->text, src->text);

  // FILE:[main.c]
  mc_set_str(path, directory_path);
  if (path->text[path->len - 1] != '/') {
    mc_append_to_str(path, "/");
  }
  _export_app_write_main_c(node, src, path);

  // Compile
  mc_str *output_path;
  mc_alloc_str(&output_path);
  mc_append_to_str(output_path, directory_path);
  mc_append_to_str(output_path, "/");
  mc_append_to_str(output_path, node->name);

  char *clargs[5];
  const char *command = "/home/jason/cling/inst/bin/clang";
  allocate_and_copy_cstr(clargs[0], "clang");
  allocate_and_copy_cstr(clargs[1], "test/main.c");
  allocate_and_copy_cstr(clargs[2], "-o");
  clargs[3] = output_path->text;
  clargs[4] = NULL;

  pid_t child_pid;
  int child_status;

  child_pid = fork();
  if (child_pid == 0) {

    // This is done by the child process
    int result = execvp(command, clargs);

    // If execvp returns, it must have failed.
    printf("clang failure:%i\n", result);

    exit(0);
  }
  else {
    // Run by main thread
    pid_t tpid = -999;
    while (tpid != child_pid) {
      tpid = wait(&child_status);
      if (tpid != child_pid) {
        // process_terminated(tpid);
      }
    }
  }

  printf("%s compiled!\n", output_path->text);

  mc_release_str(output_path, true);
  // return child_status;
}