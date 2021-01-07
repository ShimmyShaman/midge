#ifndef MC_FILE_H
#define MC_FILE_H

#include "mc_str.h"

int mcf_concat_filepath(char *buf, int buf_size, const char *appendage);
int mcf_concat_filepath_str(mc_str *path_prefix, const char *appendage);
/*
 * Replaces the path in the argument with the path to its parent directory.
 */
int mcf_restrict_parent_directory_str(mc_str *path);
/*
 * Obtains from path the parent directory and sets to buf.
 */
int mcf_get_parent_directory(char *buf, int buf_size, const char *path);
int mcf_directory_exists(const char *path, bool *exists);
int mcf_file_exists(const char *path, bool *exists);

/*
 * Ensures a directory (and all its parent directories) exists, creating them if necessary.
 */
int mcf_ensure_directory_exists(const char *path);

int mcf_obtain_full_path(const char *relative_path, char *dest, int max_len);

int mcf_obtain_file_extension(const char *path, char *buf, int max_len);
int mcf_obtain_filename(const char *path, char *buf, int max_len);
int mcf_obtain_filename_with_extension(const char *path, char *buf, int max_len);

#endif // MC_FILE_H