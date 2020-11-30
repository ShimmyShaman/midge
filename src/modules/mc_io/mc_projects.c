/* mc_projects.c */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <ctype.h>

#include <sys/stat.h>
#include <unistd.h>

#include "midge_error_handling.h"

#include "mc_str.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_projects.h"

int _mc_construct_project_initialize_header(const char *subdir, const char *name)
{
  mc_str *str;
  MCcall(init_mc_str(&str));

  // Include Guard
  MCcall(append_to_mc_str(str, "#ifndef INITIALIZE_"));
  char *c = &name[0];
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H\n"
                               "#define INITIALIZE_"));
  c = name;
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H\n\n"));

  MCcall(append_to_mc_str(str, "#endif // INITIALIZE_"));
  c = name;
  while (*c) {
    MCcall(append_char_to_mc_str(str, toupper(*c)));
    ++c;
  }
  MCcall(append_to_mc_str(str, "_H"));

  puts(str->text);
  return 0;
}

int _mc_construct_project_initialize_source(const char *subdir, const char *name) { return 0; }

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