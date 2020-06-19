/* renderer.cpp */

#include "rendering/renderer.h"
#include "rendering/cube_data.h"
#include "rendering/node_render.h"

#define MRT_RUN(CALL)         \
  result = CALL;              \
  if (result != VK_SUCCESS) { \
    thr->has_concluded = 1;   \
    return NULL;              \
  }

#define MRT_RUN2(CALL)        \
  CALL;                       \
  if (result != VK_SUCCESS) { \
    thr->has_concluded = 1;   \
    return NULL;              \
  }

static glsl_shader vertex_shader = {
    .text = "#version 400\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding = 0) uniform UBO0 {\n"
            "    mat4 mvp;\n"
            "} globalUI;\n"
            "layout (binding = 1) uniform UBO1 {\n"
            "    vec2 drawOffset;\n"
            "} element;\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 1) in vec4 inColor;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = globalUI.mvp * pos;\n"
            "   gl_Position.xy += element.drawOffset.xy;"
            "}\n",
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
};

static glsl_shader fragment_shader = {
    .text = "#version 400\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = color;\n"
            "}\n",
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
};

VkResult draw_cube(vk_render_state *p_vkrs);
VkResult render_through_queue(vk_render_state *p_vkrs, renderer_queue *render_queue);

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
extern "C" void *midge_render_thread(void *vargp)
{
  // -- Arguments
  render_thread_info *render_thread = (render_thread_info *)vargp;
  mthread_info *thr = render_thread->thread_info;
  // printf("mrt-2: %p\n", thr);

  // -- States
  mxcb_window_info winfo = {
      .shouldExit = 0,
  };
  vk_render_state vkrs;
  vkrs.window_width = 1024;
  vkrs.window_height = 640;
  vkrs.maximal_image_width = 1024;
  vkrs.maximal_image_height = 1024;
  vkrs.depth.format = VK_FORMAT_UNDEFINED;
  vkrs.xcb_winfo = &winfo;

  vkrs.render_offset[0] = 0.4f;
  vkrs.render_offset[1] = 0;

  bool depth_present = false;

  // -- Initialization
  VkResult result;
  MRT_RUN(mvk_init_global_layer_properties(&vkrs));
  mvk_init_device_extension_names(&vkrs);

  // -- Renderer
  MRT_RUN2(mvk_init_instance(&result, &vkrs, "midge"));
  MRT_RUN(mvk_init_enumerate_device(&vkrs, 1));
  mxcb_init_window(&winfo, vkrs.window_width, vkrs.window_height);
  MRT_RUN(mvk_init_swapchain_extension(&vkrs));
  MRT_RUN(mvk_init_device(&vkrs));

  MRT_RUN(mvk_init_command_pool(&vkrs));
  MRT_RUN(mvk_init_command_buffer(&vkrs));
  MRT_RUN(mvk_execute_begin_command_buffer(&vkrs));
  mvk_init_device_queue(&vkrs);
  MRT_RUN(mvk_init_swapchain(&vkrs));
  MRT_RUN(mvk_init_depth_buffer(&vkrs));
  MRT_RUN(mvk_init_headless_image(&vkrs));
  MRT_RUN(mvk_init_uniform_buffer(&vkrs));
  MRT_RUN(mvk_init_descriptor_and_pipeline_layouts(&vkrs, false, 0));
  MRT_RUN(mvk_init_renderpass(&vkrs));
  MRT_RUN(mvk_init_shader(&vkrs, &vertex_shader, 0));
  MRT_RUN(mvk_init_shader(&vkrs, &fragment_shader, 1));
  MRT_RUN(mvk_init_framebuffers(&vkrs, depth_present));
  MRT_RUN(mvk_init_cube_vertices(&vkrs, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
                                 sizeof(g_vb_solid_face_colors_Data[0]), false));
  MRT_RUN(mvk_init_shape_vertices(&vkrs));
  MRT_RUN(mvk_init_descriptor_pool(&vkrs, false));
  MRT_RUN(mvk_init_descriptor_set(&vkrs, false));
  MRT_RUN(mvk_init_pipeline_cache(&vkrs));
  MRT_RUN(mvk_init_pipeline(&vkrs, depth_present, true)); // Maybe false?

  // MRT_RUN(draw_cube(&vkrs));

  // -- Update
  printf("mvkInitSuccess!\n");
  // printf("mrt-2: %p\n", thr);
  // printf("mrt-2: %p\n", &winfo);
  while (!thr->should_exit && !winfo.shouldExit) {
    // printf("mrt-3\n");
    usleep(1);
    mxcb_update_window(&winfo);

    render_thread->render_thread_initialized = true;

    if (!render_thread->renderer_queue->in_use && render_thread->renderer_queue->count) {
      printf("received render queue in renderer : %i items\n", render_thread->renderer_queue->count);
      render_thread->renderer_queue->in_use = true;

      render_through_queue(&vkrs, render_thread->renderer_queue);
      render_thread->renderer_queue->count = 0;
      render_thread->renderer_queue->items = NULL;
      render_thread->renderer_queue->in_use = false;
    }
  }
  printf("AfterUpdate!\n");

  // -- Cleanup
  mvk_destroy_pipeline(&vkrs);
  mvk_destroy_pipeline_cache(&vkrs);
  mvk_destroy_descriptor_pool(&vkrs);
  // printf("mrt-4\n");
  mvk_destroy_shape_vertices(&vkrs);
  mvk_destroy_cube_vertices(&vkrs);
  mvk_destroy_framebuffers(&vkrs);
  mvk_destroy_shaders(&vkrs);
  mvk_destroy_renderpass(&vkrs);
  // printf("mrt-5\n");
  mvk_destroy_descriptor_and_pipeline_layouts(&vkrs);
  mvk_destroy_uniform_buffer(&vkrs);
  mvk_destroy_headless_image(&vkrs);
  mvk_destroy_depth_buffer(&vkrs);
  mvk_destroy_swap_chain(&vkrs);
  mvk_destroy_command_buffer(&vkrs);
  // printf("mrt-6\n");
  mvk_destroy_command_pool(&vkrs);
  mxcb_destroy_window(&winfo);
  mvk_destroy_device(&vkrs);
  mvk_destroy_instance(&vkrs);
  printf("mvkConcluded(SUCCESS)\n");
  thr->has_concluded = 1;
  return 0;
}

VkResult render_sequence(renderer_queue *render_queue) { return VK_SUCCESS; }

VkResult render_through_queue(vk_render_state *p_vkrs, renderer_queue *render_queue)
{
  for (int i = 0; i < render_queue->count; ++i) {

    node_render_sequence *sequence = render_queue->items[i];

    if (sequence->render_target != NODE_RENDER_TARGET_PRESENT)
      return VK_ERROR_UNKNOWN;

    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.19f;
    clear_values[0].color.float32[1] = 0.34f;
    clear_values[0].color.float32[2] = 0.83f;
    clear_values[0].color.float32[3] = 1.f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    VkSemaphore imageAcquiredSemaphore;
    VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
    imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    imageAcquiredSemaphoreCreateInfo.pNext = NULL;
    imageAcquiredSemaphoreCreateInfo.flags = 0;

    VkResult res = vkCreateSemaphore(p_vkrs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
    assert(res == VK_SUCCESS);

    // Get the index of the next available swapchain image:
    res = vkAcquireNextImageKHR(p_vkrs->device, p_vkrs->swap_chain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
                                &p_vkrs->current_buffer);
    // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
    // return codes
    assert(res == VK_SUCCESS);

    VkRenderPassBeginInfo rp_begin;
    rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rp_begin.pNext = NULL;
    rp_begin.renderPass = p_vkrs->render_pass;
    rp_begin.framebuffer = p_vkrs->framebuffers[p_vkrs->current_buffer];
    rp_begin.renderArea.offset.x = 0;
    rp_begin.renderArea.offset.y = 0;
    rp_begin.renderArea.extent.width = sequence->extent_width;
    rp_begin.renderArea.extent.height = sequence->extent_height;
    rp_begin.clearValueCount = 2;
    rp_begin.pClearValues = clear_values;

    vkCmdBeginRenderPass(p_vkrs->cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
    mvk_init_viewports(p_vkrs, sequence->extent_width, sequence->extent_height);
    mvk_init_scissors(p_vkrs, sequence->extent_width, sequence->extent_height);

    // const unsigned int MAX_DESC_SET_WRITES = 4;
    // VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
    // VkDescriptorBufferInfo bufferInfos[MAX_DESC_SET_WRITES];

    for (int j = 0; j < sequence->render_command_count; ++j) {

      vkCmdBindPipeline(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline);

      // // Binding Descriptor Sets
      // int wdsIndex = 0;
      // VkWriteDescriptorSet *write = &writes[wdsIndex];
      // writes->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      // writes->pNext = NULL;
      // writes->dstSet = p_vkrs->desc_set[0];
      // writes->descriptorCount = 1;
      // writes->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      // writes->pBufferInfo = &p_vkrs->uniform_data.buffer_info;
      // writes->dstArrayElement = 0;
      // writes->dstBinding = 0;

      vkCmdBindDescriptorSets(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline_layout, 0, NUM_DESCRIPTOR_SETS,
                              p_vkrs->desc_set.data(), 0, NULL);

      render_command *cmd = &sequence->render_commands[j];
      switch (cmd->type) {
      case RENDER_COMMAND_COLORED_RECTANGLE: {
        const VkDeviceSize offsets[1] = {0};

        vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->shape_vertices.buf, offsets);
        vkCmdDraw(p_vkrs->cmd, 2 * 3, 1, 0, 0);

        // vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->cube_vertices.buf, offsets);
        // vkCmdDraw(p_vkrs->cmd, 12 * 3, 1, 0, 0);
      } break;

      default:
        printf("COULD NOT HANDLE RENDER COMMAND TYPE=%i\n", cmd->type);
        continue;
      }
    }

    vkCmdEndRenderPass(p_vkrs->cmd);
    res = vkEndCommandBuffer(p_vkrs->cmd);
    assert(res == VK_SUCCESS);

    VkFenceCreateInfo fenceInfo;
    VkFence drawFence;
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.pNext = NULL;
    fenceInfo.flags = 0;
    vkCreateFence(p_vkrs->device, &fenceInfo, NULL, &drawFence);
    assert(res == VK_SUCCESS);

    VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo submit_info[1] = {};
    submit_info[0].pNext = NULL;
    submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info[0].waitSemaphoreCount = 1;
    submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
    submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
    submit_info[0].commandBufferCount = 1;
    const VkCommandBuffer cmd_bufs[] = {p_vkrs->cmd};
    submit_info[0].pCommandBuffers = cmd_bufs;
    submit_info[0].signalSemaphoreCount = 0;
    submit_info[0].pSignalSemaphores = NULL;

    /* Queue the command buffer for execution */
    res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
    assert(res == VK_SUCCESS);

    /* Now present the image in the window */
    VkPresentInfoKHR present;
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.pNext = NULL;
    present.swapchainCount = 1;
    present.pSwapchains = &p_vkrs->swap_chain;
    present.pImageIndices = &p_vkrs->current_buffer;
    present.pWaitSemaphores = NULL;
    present.waitSemaphoreCount = 0;
    present.pResults = NULL;

    /* Make sure command buffer is finished before presenting */
    do {
      res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
    } while (res == VK_TIMEOUT);
    assert(res == VK_SUCCESS);

    res = vkQueuePresentKHR(p_vkrs->present_queue, &present);
    assert(res == VK_SUCCESS);

    vkDestroySemaphore(p_vkrs->device, imageAcquiredSemaphore, NULL);
    vkDestroyFence(p_vkrs->device, drawFence, NULL);
  }

  return VK_SUCCESS;
}

VkResult draw_cube(vk_render_state *p_vkrs)
{
  VkClearValue clear_values[2];
  clear_values[0].color.float32[0] = 0.19f;
  clear_values[0].color.float32[1] = 0.34f;
  clear_values[0].color.float32[2] = 0.83f;
  clear_values[0].color.float32[3] = 1.f;
  clear_values[1].depthStencil.depth = 1.0f;
  clear_values[1].depthStencil.stencil = 0;

  VkSemaphore imageAcquiredSemaphore;
  VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
  imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  imageAcquiredSemaphoreCreateInfo.pNext = NULL;
  imageAcquiredSemaphoreCreateInfo.flags = 0;

  VkResult res = vkCreateSemaphore(p_vkrs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
  assert(res == VK_SUCCESS);

  // Get the index of the next available swapchain image:
  res = vkAcquireNextImageKHR(p_vkrs->device, p_vkrs->swap_chain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
                              &p_vkrs->current_buffer);
  // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
  // return codes
  assert(res == VK_SUCCESS);

  VkRenderPassBeginInfo rp_begin;
  rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  rp_begin.pNext = NULL;
  rp_begin.renderPass = p_vkrs->render_pass;
  rp_begin.framebuffer = p_vkrs->framebuffers[p_vkrs->current_buffer];
  rp_begin.renderArea.offset.x = 0;
  rp_begin.renderArea.offset.y = 0;
  rp_begin.renderArea.extent.width = p_vkrs->window_width;
  rp_begin.renderArea.extent.height = p_vkrs->window_height;
  rp_begin.clearValueCount = 2;
  rp_begin.pClearValues = clear_values;

  vkCmdBeginRenderPass(p_vkrs->cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline);
  vkCmdBindDescriptorSets(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline_layout, 0, NUM_DESCRIPTOR_SETS,
                          p_vkrs->desc_set.data(), 0, NULL);

  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->cube_vertices.buf, offsets);

  mvk_init_viewports(p_vkrs, p_vkrs->window_width, p_vkrs->window_height);
  mvk_init_scissors(p_vkrs, p_vkrs->window_width, p_vkrs->window_height);

  vkCmdDraw(p_vkrs->cmd, 12 * 3, 1, 0, 0);
  vkCmdEndRenderPass(p_vkrs->cmd);
  res = vkEndCommandBuffer(p_vkrs->cmd);
  const VkCommandBuffer cmd_bufs[] = {p_vkrs->cmd};
  VkFenceCreateInfo fenceInfo;
  VkFence drawFence;
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.pNext = NULL;
  fenceInfo.flags = 0;
  vkCreateFence(p_vkrs->device, &fenceInfo, NULL, &drawFence);

  VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  VkSubmitInfo submit_info[1] = {};
  submit_info[0].pNext = NULL;
  submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submit_info[0].waitSemaphoreCount = 1;
  submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
  submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
  submit_info[0].commandBufferCount = 1;
  submit_info[0].pCommandBuffers = cmd_bufs;
  submit_info[0].signalSemaphoreCount = 0;
  submit_info[0].pSignalSemaphores = NULL;

  /* Queue the command buffer for execution */
  res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
  assert(res == VK_SUCCESS);

  /* Now present the image in the window */

  VkPresentInfoKHR present;
  present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  present.pNext = NULL;
  present.swapchainCount = 1;
  present.pSwapchains = &p_vkrs->swap_chain;
  present.pImageIndices = &p_vkrs->current_buffer;
  present.pWaitSemaphores = NULL;
  present.waitSemaphoreCount = 0;
  present.pResults = NULL;

  /* Make sure command buffer is finished before presenting */
  do {
    res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);

  assert(res == VK_SUCCESS);
  res = vkQueuePresentKHR(p_vkrs->present_queue, &present);
  assert(res == VK_SUCCESS);

  const struct timespec ts = {1L, 0L};
  nanosleep(&ts, NULL);

  /* VULKAN_KEY_END */
  // if (p_vkrs->save_images)
  //   write_ppm(p_vkrs, "15-draw_cube");

  vkDestroySemaphore(p_vkrs->device, imageAcquiredSemaphore, NULL);
  vkDestroyFence(p_vkrs->device, drawFence, NULL);

  return res;
}

// VkResult vk_init_layers_extensions(std::vector<const char *> *instanceLayers, std::vector<const char *>
// *instanceExtensions)
// {
//   // Set up extensions
//   instanceExtensions->push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
//   instanceExtensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);

//   return VK_SUCCESS;
// }

// VkResult initVulkan(vk_render_state *p_vkrs, mxcb_window_info *p_wnfo)
// {
//   VkResult res;

//   // -- Application Info --
//   VkApplicationInfo application_info;
//   application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//   // application_info.apiVersion = VK_MAKE_VERSION(1, 0, 2); // 1.0.2 should work on all vulkan enabled drivers.
//   // application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
//   application_info.pApplicationName = "Vulkan API Tutorial Series";

//   // -- Layers & Extensions --
//   std::vector<const char *> instanceLayers;
//   std::vector<const char *> instanceExtensions;
//   // vk_init_layers_extensions(&instanceLayers, &instanceExtensions);
//   instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
//   instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

//   // -- Debug --
//   VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
//   VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
//   // setupDebug(&debugReport, &debugCallbackCreateInfo, &instanceLayers, &instanceExtensions);

//   // -- VK Instance --
//   VkInstanceCreateInfo instance_create_info;
//   instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//   instance_create_info.pApplicationInfo = &application_info;
//   instance_create_info.enabledLayerCount = instanceLayers.size();
//   instance_create_info.ppEnabledLayerNames = instanceLayers.data();
//   instance_create_info.enabledExtensionCount = instanceExtensions.size();
//   instance_create_info.ppEnabledExtensionNames = instanceExtensions.data();
// instance_create_info.pNext = debugCallbackCreateInfo;

//   // printf("vkinst=%p\n", p_vkstate->instance);
//   printf("aboutToVkCreateInstance()\n");
//   res = vkCreateInstance(&instance_create_info, NULL, &p_vkrs->instance);
//   if (res != VK_SUCCESS)
//   {
//     printf("Failed to create vulkan instance!\n");
//     return res;
//   }
//   printf("vkCreateInstance(SUCCESS)\n");

//   // initDebug();
//   res = initDevice(p_vkrs);
//   if (res != VK_SUCCESS)
//   {
//     printf("Failed to create vulkan instance!\n");
//     return res;
//   }

//   // Window
//   // printf("initOSWindow\n");
//   initOSWindow(p_wnfo, 800, 480);
//   initOSSurface(p_wnfo, p_vkrs->instance, &p_vkrs->surface);
//   // init_swapchain_extension(p_vkrs, p_wnfo);
//   return VK_SUCCESS;
// }

// VkResult initDevice(vk_render_state *p_vkstate)
// {
//   VkResult res;
//   std::vector<const char *> device_extensions; // TODO -- does this get filled with values anywhere?

//   {
//     uint32_t gpu_count = 0;
//     vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, NULL);
//     std::vector<VkPhysicalDevice> gpu_list(gpu_count);
//     vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, gpu_list.data());
//     p_vkstate->gpu = gpu_list[0];
//     vkGetPhysicalDeviceProperties(p_vkstate->gpu, &p_vkstate->gpu_properties);
//   }
//   {
//     uint32_t family_count = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(p_vkstate->gpu, &family_count, nullptr);
//     std::vector<VkQueueFamilyProperties> family_property_list(family_count);
//     vkGetPhysicalDeviceQueueFamilyProperties(p_vkstate->gpu, &family_count, family_property_list.data());

//     bool found = false;
//     for (uint32_t i = 0; i < family_count; ++i)
//     {
//       if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//       {
//         found = true;
//         p_vkstate->graphics_family_index = i;
//       }
//     }
//     if (!found)
//     {
//       printf("Vulkan ERROR: Queue family supporting graphics not found.\n");
//       return VK_NOT_READY;
//     }
//   }

//   float queue_priorities[]{1.0f};
//   VkDeviceQueueCreateInfo device_queue_create_info{};
//   device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//   device_queue_create_info.queueFamilyIndex = p_vkstate->graphics_family_index;
//   device_queue_create_info.queueCount = 1;
//   device_queue_create_info.pQueuePriorities = queue_priorities;

//   VkDeviceCreateInfo device_create_info{};
//   device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//   device_create_info.queueCreateInfoCount = 1;
//   device_create_info.pQueueCreateInfos = &device_queue_create_info;
//   //	device_create_info.enabledLayerCount		= _device_layers.size();				//
//   depricated
//   //	device_create_info.ppEnabledLayerNames		= _device_layers.data();				//
//   depricated device_create_info.enabledExtensionCount = device_extensions.size();
//   device_create_info.ppEnabledExtensionNames = device_extensions.data();

//   res = vkCreateDevice(p_vkstate->gpu, &device_create_info, nullptr, &p_vkstate->device);
//   if (res != VK_SUCCESS)
//   {
//     printf("unhandled error 8258528");
//     return res;
//   }

//   vkGetDeviceQueue(p_vkstate->device, p_vkstate->graphics_family_index, 0, &p_vkstate->queue);
//   return VK_SUCCESS;
// }

// void setupDebug(VkDebugReportCallbackEXT *debugReport, VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo,
// std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions)
// {
//   debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
//   debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
//   debugCallbackCreateInfo.flags =
//       //		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
//       VK_DEBUG_REPORT_WARNING_BIT_EXT |
//       VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
//       VK_DEBUG_REPORT_ERROR_BIT_EXT |
//       //		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
//       0;

//   instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
//   /*
// //	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_threading" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_GOOGLE_threading" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_draw_state" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_image" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_object_tracker" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_param_checker" );
// 	*/
//   instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

//   //	_device_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );				// depricated
//   /*
// //	_device_layers.push_back( "VK_LAYER_LUNARG_threading" );
// 	_device_layers.push_back( "VK_LAYER_GOOGLE_threading" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_image" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
// 	*/
// }

// VKAPI_ATTR VkBool32 VKAPI_CALL
// VulkanDebugCallback(
//     VkDebugReportFlagsEXT flags,
//     VkDebugReportObjectTypeEXT obj_type,
//     uint64_t src_obj,
//     size_t location,
//     int32_t msg_code,
//     const char *layerPrefix,
//     const char *msg,
//     void *user_data)
// {
//   printf("VKDBG: ");
//   if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
//   {
//     printf("INFO: ");
//   }
//   if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
//   {
//     printf("WARNING: ");
//   }
//   if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//   {
//     printf("PERFORMANCE: ");
//   }
//   if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
//   {
//     printf("ERROR: ");
//   }
//   if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
//   {
//     printf("DEBUG: ");
//   }
//   printf("@[%s]: ", layerPrefix);
//   printf("%s\n", msg);

//   return false;
// }

// void deInitVulkan(vk_render_state *p_vkrs)
// {
//   vkDestroyDevice(p_vkrs->device, NULL);
//   p_vkrs->device = NULL;

//   // deInitDebug() TODO

//   vkDestroyInstance(p_vkrs->instance, NULL);
//   p_vkrs->instance = NULL;
// }