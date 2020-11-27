/* mc_file.c *


#include "mc_str.h"

#include "modules/mc_io/mc_io.h"

int mcf_ensure_directory_path(mc_str *path)
{
  if (path->text[pd->len - 1] != '\\' || path->text[pd->len - 1] != '/') {
    MCcall(append_char_to_mc_str(pd, '/'));
  }
  return 0;
}