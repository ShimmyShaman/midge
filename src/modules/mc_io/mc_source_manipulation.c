/* mc_source_manipulation.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "core/midge_app.h"
#include "midge_error_handling.h"

#include "modules/mc_io/mc_file.h"
#include "modules/mc_io/mc_info_transcription.h"

#include "modules/mc_io/mc_source_manipulation.h"

int mcs_obtain_source_file_info(const char *path, bool create_if_not_exists, mc_source_file_info **source_file)
{
  char full_path[256];
  mcf_obtain_full_path(path, full_path, 256);

  mc_app_itp_data *app_itp_data;
  mc_obtain_app_itp_data(&app_itp_data);

  // Determine if the header already exists
  mc_source_file_info *sf;
  for (int a = 0; a < app_itp_data->source_files.count; ++a) {
    sf = app_itp_data->source_files.items[a];

    if (strcmp(sf->filepath, full_path)) {
      sf = NULL;
      continue;
    }

    // Found it
    break;
  }
  if (!sf) {
    if (!create_if_not_exists) {
      *source_file = NULL;
      return 0;
    }

    // Create it
    if (access(full_path, F_OK) != -1) {
      MCerror(7348, "File already exists! but...");
    }

    sf = (mc_source_file_info *)malloc(sizeof(mc_source_file_info));
    sf->filepath = strdup(path);
    sf->segments.capacity = sf->segments.count = 0U;

    // Register & persist
    MCcall(append_to_collection((void ***)&app_itp_data->source_files.items, &app_itp_data->source_files.alloc,
                                &app_itp_data->source_files.count, sf));
    MCcall(mc_save_source_file_from_updated_info(sf));
  }

  if (source_file) {
    *source_file = sf;
  }
  return 0;
}