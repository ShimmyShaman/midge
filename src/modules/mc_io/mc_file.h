#ifndef MC_FILE_H
#define MC_FILE_H

#include "mc_str.h"

int mcf_concat_filepath(mc_str *path_prefix, const char *appendage);
int mcf_directory_exists(const char *path, bool *exists);

int mcf_obtain_file_extension(const char *path, char *buf, int max_len);

#endif // MC_FILE_H