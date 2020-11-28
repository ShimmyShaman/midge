/* mc_projects.c */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_projects.h"

int mcf_create_project(const char *parent_directory, const char *name)
{
  // Determine valid directory for parent_directory
  bool exists;
  mcf_directory_exists(parent_directory, &exists);
  if (!exists) {
    // Do nothing more
    MCerror(5420, "TODO - error handling");
  }

  // Check potential directory does not exist
  char proj_dir[256];
  {
    strcpy(proj_dir, parent_directory);
    char c = proj_dir[strlen(proj_dir) - 1];
    if (c != '\\' && c != '/')
      strcat(proj_dir, "/");
    strcat(proj_dir, name);
    strcat(proj_dir, "/");
  }
  mcf_directory_exists(proj_dir, &exists);
  if (exists) {
    // Do nothing more
    MCerror(5436, "TODO - error handling");
  }

  // Make the directory
  if (mkdir(proj_dir, 0700)) {
    MCerror(5441, "TODO - error handling");
  }

  // Make the src directory
  char src_dir[256];
  strcpy(src_dir, proj_dir);
  strcat(src_dir, "src/");

  // Make the main.c file
  // EEEEEK TODO

  // Make the init.c file
  MCerror(5553, "Progress");

  return 0;
}