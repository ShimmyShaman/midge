/* mc_file.c */

#include <stdio.h>

#include <sys/stat.h>
#include <unistd.h>

#include "mc_str.h"

#include "modules/mc_io/mc_io.h"

int mcf_concat_filepath(mc_str *path_prefix, const char *appendage)
{
  if (path_prefix->text[path_prefix->len - 1] != '\\' || path_prefix->text[path_prefix->len - 1] != '/') {
    MCcall(append_char_to_mc_str(path_prefix, '/'));
  }
  MCcall(append_to_mc_str(path_prefix, appendage));

  // if(appendage)
  // if (path_prefix->text[path_prefix->len - 1] != '\\' || path_prefix->text[path_prefix->len - 1] != '/') {
  //   MCcall(append_char_to_mc_str(path_prefix, '/'));
  // }

  return 0;
}

int mcf_directory_exists(const char *path, bool *exists)
{
  struct stat stats;

  MCcall(stat(path, &stats));

  // Check for file existence
  *exists = S_ISDIR(stats.st_mode) ? true : false;

  return 0;
}

// int mcf_create_directory(const char *path, const char *directory_name)
// {
//   struct stat stats;

//   MCcall(stat(path, &stats));

//   // Check for file existence
//   exists = S_ISDIR(stats.st_mode) ? true : false;

//   return 0;
// }