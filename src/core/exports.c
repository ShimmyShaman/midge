

#include "core/midge_core.h"

size_t save_text_to_file(char *filepath, char *text);

void _export_app_write_main_c(mc_node_v1 *node, mc_str *src, mc_str *path)
{
  console_app_info *app_info = (console_app_info *)node->extra;

  set_mc_str(src, "#include <stdio.h>\n"
                 "\n");

  // OTHER STUFF TEMPORARRY
  append_to_mc_str(src, "typedef struct node {\n"
                       "const char *name;\n"
                       "} node;\n"
                       "\n");

  if (app_info) {
    if (app_info->initialize_app) {
      append_to_mc_str(src, app_info->initialize_app->source->code);
      append_to_mc_str(src, "\n\n");
    }
  }
  // OTHER STUFF TEMPORARRY

  append_to_mc_str(src, "int main()\n"
                       "{\n");

  // -- Node
  append_to_mc_strf(src, "  node app_root;\n", node->name);
  append_to_mc_strf(src, "  app_root.name = \"%s\";\n", node->name);
  append_to_mc_str(src, "\n");

  // -- Initialize
  if (app_info) {
    if (app_info->initialize_app) {
      append_to_mc_strf(src, "  %s(&app_root);\n", app_info->initialize_app->name);
      append_to_mc_str(src, "\n");
    }
  }

  // Cleanup -- TODO

  // Exit
  append_to_mc_str(src, "  return 0;\n");
  append_to_mc_str(src, "}\n");

  // Write to file
  append_to_mc_str(path, "main.c");
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
  init_mc_str(&src);
  init_mc_str(&path);

  // FILE:[node.c]
  // append_to_mc_strf(src, "  node %s;\n", node->name);

  // // Write to file
  // set_mc_str(path, directory_path);
  // if (path->text[path->len - 1] != '/')
  //   append_to_mc_str(path, "/");
  // append_to_mc_str(path, "node.c");
  // save_text_to_file(path->text, src->text);

  // FILE:[main.c]
  set_mc_str(path, directory_path);
  if (path->text[path->len - 1] != '/') {
    append_to_mc_str(path, "/");
  }
  _export_app_write_main_c(node, src, path);

  // Compile
  mc_str *output_path;
  init_mc_str(&output_path);
  append_to_mc_str(output_path, directory_path);
  append_to_mc_str(output_path, "/");
  append_to_mc_str(output_path, node->name);

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

  release_mc_str(output_path, true);
  // return child_status;
}