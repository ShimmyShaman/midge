/* mc_file.c */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "mc_str.h"

#include "modules/mc_io/mc_io.h"

int mcf_concat_filepath(char *buf, int buf_size, const char *appendage)
{
  int len = strlen(buf);
  if (buf[len - 1] != '\\' && buf[len - 1] != '/') {
    strcat(buf, "/");
    ++len;
  }
  if (len + strlen(appendage) + 1 > buf_size) {
    MCerror(2242, "mcf_concat_filepath oversized appendage");
  }
  strcat(buf, appendage);

  return 0;
}

int mcf_concat_filepath_str(mc_str *path_prefix, const char *appendage)
{
  if (path_prefix->text[path_prefix->len - 1] != '\\' && path_prefix->text[path_prefix->len - 1] != '/') {
    MCcall(append_char_to_mc_str(path_prefix, '/'));
  }
  MCcall(append_to_mc_str(path_prefix, appendage));

  // if(appendage)
  // if (path_prefix->text[path_prefix->len - 1] != '\\' || path_prefix->text[path_prefix->len - 1] != '/') {
  //   MCcall(append_char_to_mc_str(path_prefix, '/'));
  // }

  return 0;
}

int mcf_restrict_parent_directory_str(mc_str *path)
{
  char *c = path->text + (path->len - 1);
  if (*c == '/' || *c == '\\')
    --c;
  for (;; --c) {
    if (c == path->text) {
      MCerror(5251, "mcf_restrict_parent_directory_str: Path cannot be reduced");
    }
    if (*c == '/' || *c == '\\') {
      break;
    }
  }

  MCcall(restrict_mc_str(path, (int)(c - path->text)));
  printf("mcf_restrict_parent_directory_str: '%s'\n", path);
  return 0;
}

int mcf_get_parent_directory(char *buf, int buf_size, const char *path)
{
  char *c = (char *)path;

  for (; *c != '\0'; ++c)
    ;
  --c;
  if (*c == '/' || *c == '\\')
    --c;
  for (;; --c) {
    if (c == path) {
      MCerror(5251, "mcf_get_parent_directory: Path cannot be reduced");
    }
    if (*c == '/' || *c == '\\') {
      break;
    }
  }

  int n = c - path;
  if (n + 1 > buf_size) {
    MCerror(2283, "mcf_get_parent_directory buffer size too small");
  }
  strncpy(buf, path, n);
  buf[n] = '\0';
  printf("mcf_get_parent_directory: '%s'\n", buf);
  return 0;
}

int mcf_directory_exists(const char *path, bool *exists)
{
  struct stat stats;

  puts(path);
  int res = stat(path, &stats);
  printf("res = %i\n", res);
  if (!res) {
    *exists = S_ISDIR(stats.st_mode) ? true : false;
    return 0;
  }
  else if (res == -1) {
    *exists = false;
    return 0;
  }

  fprintf(stderr, "stat failed!\n %s\n", strerror(errno));
  return 33;
}

// int mcf_create_directory(const char *path, const char *directory_name)
// {
//   struct stat stats;

//   MCcall(stat(path, &stats));

//   // Check for file existence
//   exists = S_ISDIR(stats.st_mode) ? true : false;

//   return 0;
// }
int mcf_obtain_file_extension(const char *path, char *buf, int max_len)
{
  int a = 0;
  char *c = (char *)&path[0];

  while (*c != '\0') {
    if (a) {
      if (a >= max_len) {
        MCerror(6825, "File extension was too large");
      }

      buf[a - 1] = *c;
      ++a;
    }
    else if (*c == '.') {
      a = 1;
    }
    ++c;
  }

  buf[a - 1] = '\0';
  // printf("mcf_obtain_file_extension Input:'%s' Output:'%s'\n", path, buf);
  return 0;
}