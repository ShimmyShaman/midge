/* mc_file.c */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "mc_str.h"


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
  // printf("mcf_get_parent_directory: '%s'\n", buf);
  return 0;
}

int mcf_directory_exists(const char *path, bool *exists)
{
  struct stat stats;

  // puts(path);
  int res = stat(path, &stats);
  // printf("res = %i\n", res);
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

int mcf_obtain_full_path(const char *relative_path, char *dest, int max_len)
{
  if (relative_path[0] == '/') {
    strcpy(dest, relative_path);
    return 0;
  }

  // Given filepath is relative -- convert to absolute
  if (!getcwd(dest, max_len)) {
    MCerror(6120, "getcwd failed");
  }
  int n = strlen(dest);

  if (n + strlen(relative_path) >= max_len) {
    MCerror(6133, "full path exceeds buffer bounds");
  }

  MCcall(mcf_concat_filepath(dest, max_len, relative_path));

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

/* TODO -- this just won't work with linux - wont get no extension and will have trouble with '.' in anything but
 * extension use **think I fixed this but forgot to remove the TODO?
 */
int mcf_obtain_file_extension(const char *path, char *buf, int max_len)
{
  int a = 0;
  char *c = (char *)path - 1, *s = NULL;

  // Move to the end of the string
  while (*(++c) != '\0')
    ;

  // Go back to the first decimal or the start or the first slash bracket
  while (c != path) {
    --c;
    --max_len;
    if (*c == '.') {
      ++c;
      if (max_len < 0) {
        // TODO -- this max_len test might be off by 1
        MCerror(6825, "File extension was too large for provided buffer");
      }
      strcpy(buf, c);
      return 0;
    }
    else if (*c == '/' || *c == '\\') {
      break;
    }
  }

  // There is no extension
  if (max_len < 1) {
    MCerror(6826, "File extension was too large for provided buffer");
  }
  buf[0] = '\0';
  return 0;
}

int mcf_obtain_filename(const char *path, char *buf, int max_len)
{
  char *c = (char *)path - 1, *s;

  // Move to the end of the string
  while (*(++c) != '\0')
    ;
  s = c;

  // Go back to the start or the first slash bracket
  bool ext_found = false;
  while (c != path) {
    --c;
    --max_len;
    if (!ext_found && *c == '.') {
      ext_found = true;
      s = c;
    }
    else if (*c == '/' || *c == '\\') {
      ++c;
      break;
    }
  }

  // Copy
  if (max_len < s - c + 1) {
    MCerror(6826, "File extension was too large for provided buffer");
  }
  strncpy(buf, c, s - c);
  buf[s - c] = '\0';
  return 0;
}

int mcf_obtain_filename_with_extension(const char *path, char *buf, int max_len)
{
  char *c = (char *)path - 1, *s;

  // Move to the end of the string
  while (*(++c) != '\0')
    ;
  s = c;

  // Go back to the start or the first slash bracket
  while (c != path) {
    --c;
    --max_len;
    if (*c == '/' || *c == '\\') {
      ++c;
      break;
    }
  }

  // Copy
  if (max_len < s - c + 1) {
    MCerror(6826, "File extension was too large for provided buffer");
  }
  strcpy(buf, c);
  return 0;
}