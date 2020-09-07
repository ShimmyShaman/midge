
#include "m_threads.h"
#include "midge_common.h"
#include <vulkan/vulkan.h>

VkResult mrt_run_update_loop(render_thread_info *render_thread, vk_render_state *vkrs)
{
  mthread_info *thr = render_thread->thread_info;

  // -- Update
  mxcb_update_window(&vkrs->xcb_winfo, &render_thread->input_buffer);
  render_thread->render_thread_initialized = true;
  // printf("mrt-2: %p\n", thr);
  // printf("mrt-2: %p\n", &winfo);
  uint frame_updates = 0;
  // while (!thr->should_exit && !vkrs->xcb_winfo.shouldExit) {
  //   // Resource Commands
  //   pthread_mutex_lock(&render_thread->resource_queue.mutex);
  //   if (render_thread->resource_queue.count) {
  //     // printf("Vulkan entered resources!\n");
  //     handle_resource_commands(&vkrs, &render_thread->resource_queue);
  //     render_thread->resource_queue.count = 0;
  //     printf("Vulkan loaded resources!\n");
  //   }
  //   pthread_mutex_unlock(&render_thread->resource_queue.mutex);

  //   // Render Commands
  //   pthread_mutex_lock(&render_thread->render_queue.mutex);
  //   if (render_thread->render_queue.count) {
  //     // {
  //     //   // DEBUG
  //     //   uint cmd_count = 0;
  //     //   for (int r = 0; r < render_thread->render_queue.count; ++r) {
  //     //     cmd_count += render_thread->render_queue.image_renders[r].command_count;
  //     //   }
  //     //   printf("Vulkan entered render_queue! %u sequences using %u draw-calls\n",
  //     render_thread->render_queue.count,
  //     //   cmd_count);
  //     // }
  //     render_through_queue(&vkrs, &render_thread->render_queue);
  //     render_thread->render_queue.count = 0;

  //     // printf("Vulkan rendered render_queue!\n");
  //     ++frame_updates;
  //   }
  //   pthread_mutex_unlock(&render_thread->render_queue.mutex);

  //   mxcb_update_window(&vkrs->xcb_winfo, &render_thread->input_buffer);
  // }
  printf("AfterUpdate! frame_updates = %i\n", frame_updates);
  return VK_SUCCESS;
}

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

  // Vulkan Initialize
  VkResult res = mvk_init_vulkan(&vkrs);
  if (res) {
    printf("--ERR[%i] mvk_init_vulkan\n", res);
    return NULL;
  }
  printf("Vulkan Initialized!\n");

  // Update Loop
  res = mrt_run_update_loop(render_thread, &vkrs);

  // Vulkan Cleanup
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