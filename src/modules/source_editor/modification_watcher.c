#include <stdio.h>

#include "core/core_definitions.h"
#include "env/environment_definitions.h"

int _mc_se_source_file_modified(void *handler_state, void *event_args)
{
  mc_source_file_info *sf = (mc_source_file_info *)event_args;

  printf("sf-modified:%s\n", sf->filepath);

  return 0;
}

int mc_se_init_modification_watcher(void)
{
  MCcall(mca_register_event_handler(MC_APP_EVENT_SOURCE_FILE_MODIFIED_EXTERNALLY, &_mc_se_source_file_modified, NULL));

  return 0;
}