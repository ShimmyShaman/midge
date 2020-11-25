/* mc_app_info_data.c */

#include <stdio.h>
#include <stdlib.h>

#include "core/midge_app.h"

static midge_app_info *__mc_midge_app_info;

void mc_init_midge_app_info()
{
  __mc_midge_app_info = malloc(sizeof(midge_app_info));

  mc_obtain_app_itp_data(&__mc_midge_app_info->itp_data);
  __mc_midge_app_info->global_node = __mc_midge_app_info->itp_data->global_node;

  __mc_midge_app_info->_exit_requested = false;
}

void mc_destroy_midge_app_info() { pthread_mutex_destroy(&__mc_midge_app_info->hierarchy_mutex); }

// TODO -- make this return instead of set -- no need for error checking
void mc_obtain_midge_app_info(midge_app_info **p_app_info) { *p_app_info = __mc_midge_app_info; }