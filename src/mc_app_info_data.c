/* mc_app_info_data.c */

#include "core/midge_app.h"

static midge_app_info *__mc_midge_app_info;

void mc_init_midge_app_info()
{
  __mc_midge_app_info = malloc(sizeof(midge_app_info));

  MCcall(mc_obtain_app_itp_data(&__mc_midge_app_info->itp_data));
}

void mc_obtain_midge_app_info(midge_app_info **p_app_info) { return __mc_midge_app_info; }