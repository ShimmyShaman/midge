/* mc_file.c */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "mc_str.h"

#include "modules/mc_io/mc_file.h"

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
    MCcall(mc_append_char_to_str(path_prefix, '/'));
  }
  MCcall(mc_append_to_str(path_prefix, appendage));

  // if(appendage)
  // if (path_prefix->text[path_prefix->len - 1] != '\\' || path_prefix->text[path_prefix->len - 1] != '/') {
  //   MCcall(mc_append_char_to_str(path_prefix, '/'));
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

  MCcall(mc_restrict_str(path, (int)(c - path->text)));
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

int mcf_file_exists(const char *path, bool *exists)
{
  struct stat stats;

  // puts(path);
  int res = stat(path, &stats);
  // printf("res = %i\n", res);
  if (!res) {
    *exists = S_ISDIR(stats.st_mode) ? false : true;
    return 0;
  }
  else if (res == -1) {
    *exists = false;
    return 0;
  }

  fprintf(stderr, "stat failed!\n %s\n", strerror(errno));
  return 33;
}

int mcf_ensure_directory_exists(const char *path)
{
  const char *c, *s;
  bool exists;
  mc_str *str;
  MCcall(mc_alloc_str(&str));
  MCcall(mc_set_str(str, path));

  // Ensure each parent directory exists
  s = path;
  while (*s != '\0') {
    if (*s == '/' || *s == '\\')
      ++s;
    c = s;
    while (*c != '/' && *c != '\\' && *c != '\0')
      ++c;

    MCcall(mc_set_strn(str, path, c - path));

    MCcall(mcf_directory_exists(str->text, &exists));
    if (!exists) {
      mkdir(str->text, 0700);
    }
    s = c;
  }

  mc_release_str(str, true);

  return 0;
}

int mcf_obtain_full_path(const char *relative_path, char *dest, int dest_size)
{
  if (relative_path[0] == '/') {
    strcpy(dest, relative_path);
    return 0;
  }

  // Given filepath is relative -- convert to absolute
  if (!getcwd(dest, dest_size)) {
    MCerror(6120, "getcwd failed");
  }
  int n = strlen(dest);

  if (n + strlen(relative_path) >= dest_size) {
    MCerror(6133, "full path exceeds buffer bounds");
  }

  MCcall(mcf_concat_filepath(dest, dest_size, relative_path));

  return 0;
}

int mcf_obtain_path_relative(const char *path, const char *comparative_path, char *dest, int dest_size)
{
  const char *c = comparative_path, *f = path;
  char *d;
  int backtrack = 0;
  while (1) {
    if (*c == '\0') {
      if (*f == '\\' || *f == '/')
        ++f;

      if (!strlen(f)) {
        if (dest_size < 1 + 1) {
          MCerror(4788, "destination buffer is too small");
        }
        strcpy(dest, ".");
        printf("mcf_obtain_path_relative: '%s':'%s' => '%s'\n\n", path, comparative_path, dest);
        return 0;
      }
      else {
        if (strlen(f) + 1 > dest_size) {
          MCerror(4789, "destination buffer is too small");
        }
        strcpy(dest, f);
      }

      printf("mcf_obtain_path_relative: '%s':'%s' => '%s'\n\n", path, comparative_path, dest);

      return 0;
    }

    if (*f == '\0') {
      if (*(c + 1) == '\0') {
        if (dest_size < 1 + 1) {
          MCerror(4790, "destination buffer is too small");
        }
        strcpy(dest, ".");
        printf("mcf_obtain_path_relative: '%s':'%s' => '%s'\n\n", path, comparative_path, dest);
        return 0;
      }
      --backtrack;
      break;
    }

    if (*c != *f)
      break;

    ++c;
    ++f;
  }

  if (*c != '\0') {
    ++backtrack;
    while (*c != '\0') {
      if (*c == '\\' || *c == '/')
        ++backtrack;
      ++c;
    }
  }

  int req_len = (backtrack == 0 ? 0 : (1 + 2 * backtrack + backtrack - 1)) + strlen(f);
  if (dest_size < 1 + 1) {
    MCerror(4791, "destination buffer is too small");
  }

  d = dest;
  if (backtrack) {
    strcpy(d, "..");
    c += 2;
    while (backtrack > 1) {
      strcpy(d, "/..");
      c += 3;
      --backtrack;
    }
    if (*f != '\0') {
      strcpy(d, "/");
      ++c;
    }
  }
  strcpy(d, f);

  printf("mcf_obtain_path_relative: '%s':'%s' => '%s'\n\n", path, comparative_path, dest);

  return 0;
}

int mcf_obtain_path_relative_to_cwd(const char *full_path, char *dest, int dest_size)
{
  char buf[256];
  if (!getcwd(buf, 256)) {
    MCerror(6844, "getcwd failed");
  }

  return mcf_obtain_path_relative(full_path, buf, dest, dest_size);
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