/* mc_app_info_data.c */

#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

#include "core/midge_app.h"

static midge_app_info *__mc_midge_app_info;

void mc_init_midge_app_info()
{
  // Instance
  __mc_midge_app_info = malloc(sizeof(midge_app_info));
  __mc_midge_app_info->ROOT_UID = MIDGE_APP_INFO_ROOT_UID;
  __mc_midge_app_info->uid_index = 100U;

  mc_obtain_app_itp_data(&__mc_midge_app_info->itp_data);
  __mc_midge_app_info->global_node = __mc_midge_app_info->itp_data->global_node;
  __mc_midge_app_info->global_node->data = __mc_midge_app_info;

  // Hierarchy Mutex
  pthread_mutexattr_init(&__mc_midge_app_info->hierarchy_mutex_attr);
  pthread_mutexattr_settype(&__mc_midge_app_info->hierarchy_mutex_attr, PTHREAD_MUTEX_RECURSIVE_NP);

  pthread_mutex_init(&__mc_midge_app_info->hierarchy_mutex, &__mc_midge_app_info->hierarchy_mutex_attr);

  __mc_midge_app_info->_exit_requested = false;

  // Event Handlers
  // TODO -- if any non-MCApp Events are to be added or .. making an event handler array for each seems wrong
  // Maybe make a linked-list / binary or hash search system in the future. Not too high priority but not optimal...
  __mc_midge_app_info->event_handlers.alloc = (unsigned int)MC_APP_EXCLUSIVE_MAX;
  __mc_midge_app_info->event_handlers.count = 0;
  __mc_midge_app_info->event_handlers.items =
      (event_handler_array **)malloc(sizeof(event_handler_array *) * __mc_midge_app_info->event_handlers.alloc);
  __mc_midge_app_info->event_handlers.items[0] = NULL;

  event_handler_array *eha;
  for (int a = 1; a < MC_APP_EXCLUSIVE_MAX; ++a) {
    eha = (event_handler_array *)malloc(sizeof(event_handler_array));
    __mc_midge_app_info->event_handlers.items[a] = eha;

    eha->capacity = 0;
    eha->count = 0;
    eha->event_type = (mc_app_event_type)a;
    eha->handlers = NULL;
  }

  // Projects
  __mc_midge_app_info->projects.active = NULL;
  __mc_midge_app_info->projects.capacity = 0U;
  __mc_midge_app_info->projects.count = 0U;
}

void mc_destroy_midge_app_info()
{
  // Projects
  if (__mc_midge_app_info->projects.capacity) {
    free(__mc_midge_app_info->projects.items);

    // TODO -- free the projects also?? -- how to handle app close ...??
  }

  // Event Handlers
  event_handler_array *eha;
  for (int a = 1; a < MC_APP_EXCLUSIVE_MAX; ++a) {
    eha = __mc_midge_app_info->event_handlers.items[a];
    for (int b = 0; b < eha->count; ++b) {
      free(eha->handlers[b]);
    }
    free(eha);
  }
  free(__mc_midge_app_info->event_handlers.items);

  // Hierarchy Mutex
  pthread_mutexattr_destroy(&__mc_midge_app_info->hierarchy_mutex_attr);
  pthread_mutex_destroy(&__mc_midge_app_info->hierarchy_mutex);

  // Instance
  free(__mc_midge_app_info);
  __mc_midge_app_info = NULL;
}

// TODO -- make this return instead of set -- no need for error checking
void mc_obtain_midge_app_info(midge_app_info **p_app_info) { *p_app_info = __mc_midge_app_info; }