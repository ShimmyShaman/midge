
#include "m_threads.h"
#include "midge_common.h"

void *midge_render_thread(void *vargp)
{
  render_thread_info *render_thread = (render_thread_info *)vargp;

  // printf("~~midge_render_thread called!~~\n");

  mthread_info *thr = render_thread->thread_info;
  // printf("mrt-2: %p\n", thr);

  // -- States
  mxcb_window_info winfo;
  winfo.shouldExit = 0;

  vk_render_state vkrs = {};
  vkrs.window_width = APPLICATION_SET_WIDTH;
  vkrs.window_height = APPLICATION_SET_HEIGHT;
  vkrs.maximal_image_width = 2048;
  vkrs.maximal_image_height = 2048;
  vkrs.xcb_winfo = &winfo;

  // vkrs.textures.allocated = 0;
  VkResult res = mvk_init_vulkan(&vkrs);
  if (res) {
    printf("--ERR[%i] mvk_init_vulkan\n", res);
    return NULL;
  }
  res = mvk_cleanup_vulkan(&vkrs);
  if (res) {
    printf("--ERR[%i] mvk_cleanup_vulkan\n", res);
    return NULL;
  }

  // VkResult res;
  // res = mvk_init_global_layer_properties(vkrs);
  // if (res) {
  //   printf("--ERR[%i] mvk_init_global_layer_properties\n", res);
  //   return NULL;
  // }
  // res = mvk_init_device_extension_names(vkrs);
  // if (res) {
  //   printf("--ERR[%i] mvk_init_device_extension_names\n", res);
  //   return NULL;
  // }
  // res = mvk_init_instance(vkrs, "midge");
  // if (res) {
  //   printf("--ERR[%i] mvk_init_instance line:%i\n", res, __LINE__);
  //   return NULL;
  // }
  // res = mvk_init_enumerate_device(vkrs);
  // if (res) {
  //   printf("--ERR[%i] mvk_init_enumerate_device\n", res);
  //   return NULL;
  // }

  render_thread->thread_info->has_concluded = 1;
  return NULL;
}