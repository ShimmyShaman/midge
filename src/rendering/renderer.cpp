/* renderer.cpp */

#include "rendering/renderer.h"
#include "rendering/cube_data.h"
#include "rendering/node_render.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

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
            "    vec2 offset;\n"
            "    vec2 scale;\n"
            "} element;\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 1) in vec4 inColor;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = globalUI.mvp * pos;\n"
            "   gl_Position.xy *= element.scale.xy;\n"
            "   gl_Position.xy += element.offset.xy;\n"
            "}\n",
    .stage = VK_SHADER_STAGE_VERTEX_BIT,
};

static glsl_shader fragment_shader = {
    .text = "#version 400\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (binding = 2) uniform UBO2 {\n"
            "    vec4 tint;\n"
            "} element;\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = element.tint * color;\n"
            "}\n",
    .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
};

VkResult draw_cube(vk_render_state *p_vkrs);
VkResult render_through_queue(vk_render_state *p_vkrs, renderer_queue *render_queue);
VkResult create_texture_image(vk_render_state *p_vkrs);

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

  // glm_vec2_copy((vec2){-0.2, 0.3}, vkrs.ui_element.offset);
  // glm_vec2_copy((vec2){1.f, 1.f}, vkrs.ui_element.scale);
  // glm_vec4_copy((vec4){0.f, 1.0f, 1.f, 1.f}, vkrs.ui_element_f.fragment.tint_color);

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
  MRT_RUN(create_texture_image(&vkrs));
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
  uint frame_updates = 0;
  while (!thr->should_exit && !winfo.shouldExit) {
    // printf("mrt-3\n");
    // usleep(1);
    mxcb_update_window(&winfo);

    render_thread->render_thread_initialized = true;

    if (!render_thread->renderer_queue->in_use && render_thread->renderer_queue->count) {
      // printf("received render queue in renderer : %i items\n", render_thread->renderer_queue->count);
      render_thread->renderer_queue->in_use = true;

      render_through_queue(&vkrs, render_thread->renderer_queue);
      render_thread->renderer_queue->count = 0;
      render_thread->renderer_queue->items = NULL;
      render_thread->renderer_queue->in_use = false;
      ++frame_updates;
    }
  }
  printf("AfterUpdate! frame_updates = %i\n", frame_updates);

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

VkResult render_sequence(vk_render_state *p_vkrs, node_render_sequence *sequence)
{
  // Descriptor Writes
  VkResult res;

  coloured_rect_draw_data rect_draws[128];
  int rect_draws_index = 0;

  p_vkrs->render_data_buffer.frame_utilized_amount = 0;
  p_vkrs->render_data_buffer.queued_copies_count = 0U;

  for (int j = 0; j < sequence->render_command_count; ++j) {

    render_command *cmd = &sequence->render_commands[j];
    switch (cmd->type) {
    case RENDER_COMMAND_COLORED_RECTANGLE: {
      // Set
      coloured_rect_draw_data &rect_draw_data = rect_draws[rect_draws_index++];

      // Vertex Data
      rect_draw_data.vert.scale[0] = 2.f * cmd->width / (float)p_vkrs->window_height;
      rect_draw_data.vert.scale[1] = 2.f * cmd->height / (float)p_vkrs->window_height;
      rect_draw_data.vert.offset[0] =
          -1.0f + 2.0f * (float)cmd->x / (float)(p_vkrs->window_width) + 1.0f * (float)cmd->width / (float)(p_vkrs->window_width);
      rect_draw_data.vert.offset[1] = -1.0f + 2.0f * (float)cmd->y / (float)(p_vkrs->window_height) +
                                      1.0f * (float)cmd->height / (float)(p_vkrs->window_height);

      // Fragment Data
      glm_vec4_copy((float *)cmd->data, rect_draw_data.frag.tint_color);

      // Queue Buffer Write
      const unsigned int MAX_DESC_SET_WRITES = 8;
      VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
      VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
      int buffer_info_index = 0;
      int write_index = 0;

      // TODO -- refactor this
      VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
      vert_ubo_info->buffer = p_vkrs->render_data_buffer.buffer;
      vert_ubo_info->offset = p_vkrs->render_data_buffer.frame_utilized_amount;
      vert_ubo_info->range = sizeof(rect_draw_data.vert);

      p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].p_source = &rect_draw_data.vert;
      p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].dest_offset =
          p_vkrs->render_data_buffer.frame_utilized_amount;
      p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count++].size_in_bytes =
          sizeof(rect_draw_data.vert);
      p_vkrs->render_data_buffer.frame_utilized_amount +=
          ((vert_ubo_info->range / p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment) + 1UL) *
          p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment;

      VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
      frag_ubo_info->buffer = p_vkrs->render_data_buffer.buffer;
      frag_ubo_info->offset = p_vkrs->render_data_buffer.frame_utilized_amount;
      frag_ubo_info->range = sizeof(rect_draw_data.frag);

      p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].p_source = &rect_draw_data.frag;
      p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].dest_offset =
          p_vkrs->render_data_buffer.frame_utilized_amount;
      p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count++].size_in_bytes =
          sizeof(rect_draw_data.frag);
      p_vkrs->render_data_buffer.frame_utilized_amount +=
          ((frag_ubo_info->range / p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment) + 1UL) *
          p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment;

      // Setup viewport:
      {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)sequence->extent_width;
        viewport.height = (float)sequence->extent_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(p_vkrs->cmd, 0, 1, &viewport);
      }

      // Apply scissor/clipping rectangle
      {
        VkRect2D scissor;
        scissor.offset.x = (int32_t)(cmd->x);
        scissor.offset.y = (int32_t)(cmd->y);
        scissor.extent.width = (uint32_t)(cmd->width);
        scissor.extent.height = (uint32_t)(cmd->height);
        vkCmdSetScissor(p_vkrs->cmd, 0, 1, &scissor);
      }

      // Allocate the descriptor set from the pool.
      VkDescriptorSetAllocateInfo setAllocInfo = {};
      setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
      setAllocInfo.pNext = NULL;
      // Use the pool we created earlier ( the one dedicated to this frame )
      setAllocInfo.descriptorPool = p_vkrs->desc_pool;
      // We only need to allocate one
      setAllocInfo.descriptorSetCount = 1;
      setAllocInfo.pSetLayouts = p_vkrs->desc_layout.data();

      unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
      res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
      assert(res == VK_SUCCESS);

      VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
      p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

      // Global Vertex Shader Uniform Buffer
      VkWriteDescriptorSet *write = &writes[write_index++];
      write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write->pNext = NULL;
      write->dstSet = desc_set;
      write->descriptorCount = 1;
      write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write->pBufferInfo = &p_vkrs->global_vert_uniform_buffer.buffer_info;
      write->dstArrayElement = 0;
      write->dstBinding = 0;

      // Element Vertex Shader Uniform Buffer
      write = &writes[write_index++];
      write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write->pNext = NULL;
      write->dstSet = desc_set;
      write->descriptorCount = 1;
      write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write->pBufferInfo = vert_ubo_info;
      write->dstArrayElement = 0;
      write->dstBinding = 1;

      // Element Fragment Shader Uniform Buffer
      write = &writes[write_index++];
      write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write->pNext = NULL;
      write->dstSet = desc_set;
      write->descriptorCount = 1;
      write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write->pBufferInfo = frag_ubo_info;
      write->dstArrayElement = 0;
      write->dstBinding = 2;

      vkUpdateDescriptorSets(p_vkrs->device, write_index, writes, 0, NULL);

      vkCmdBindDescriptorSets(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline_layout, 0, 1, &desc_set, 0, NULL);

      vkCmdBindPipeline(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline);

      const VkDeviceSize offsets[1] = {0};
      vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->shape_vertices.buf, offsets);

      vkCmdDraw(p_vkrs->cmd, 2 * 3, 1, 0, 0);
    } break;

    default:
      printf("COULD NOT HANDLE RENDER COMMAND TYPE=%i\n", cmd->type);
      continue;
    }
  }

  if (p_vkrs->render_data_buffer.queued_copies_count) {
    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->render_data_buffer.memory, 0, p_vkrs->render_data_buffer.frame_utilized_amount, 0,
                      (void **)&pData);
    assert(res == VK_SUCCESS);

    // Buffer Copies
    for (int k = 0; k < p_vkrs->render_data_buffer.queued_copies_count; ++k) {
      // printf("BufferCopy: %p (+%lu) bytes:%lu\n", pData + p_vkrs->render_data_buffer.queued_copies[k].dest_offset,
      //        p_vkrs->render_data_buffer.queued_copies[k].dest_offset,
      //        p_vkrs->render_data_buffer.queued_copies[k].size_in_bytes);
      memcpy(pData + p_vkrs->render_data_buffer.queued_copies[k].dest_offset,
             p_vkrs->render_data_buffer.queued_copies[k].p_source, p_vkrs->render_data_buffer.queued_copies[k].size_in_bytes);
    }

    vkUnmapMemory(p_vkrs->device, p_vkrs->render_data_buffer.memory);
  }
  return VK_SUCCESS;
}

VkResult render_through_queue(vk_render_state *p_vkrs, renderer_queue *render_queue)
{
  for (int i = 0; i < render_queue->count; ++i) {

    node_render_sequence *sequence = render_queue->items[i];

    if (sequence->render_command_count < 1) {
      return VK_ERROR_UNKNOWN;
    }
    if (sequence->render_target != NODE_RENDER_TARGET_PRESENT)
      return VK_ERROR_UNKNOWN;

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

    res = vkResetDescriptorPool(p_vkrs->device, p_vkrs->desc_pool, 0);
    assert(res == VK_SUCCESS);
    p_vkrs->descriptor_sets_count = 0U;

    // Begin Command Buffer Recording
    res = vkResetCommandPool(p_vkrs->device, p_vkrs->cmd_pool, 0);
    assert(res == VK_SUCCESS);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
    beginInfo.pInheritanceInfo = nullptr;                          // Optional
    vkBeginCommandBuffer(p_vkrs->cmd, &beginInfo);

    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.19f;
    clear_values[0].color.float32[1] = 0.34f;
    clear_values[0].color.float32[2] = 0.83f;
    clear_values[0].color.float32[3] = 1.f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

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

    render_sequence(p_vkrs, sequence);

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
    res = vkResetFences(p_vkrs->device, 1, &drawFence);
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
  VkResult res = VK_SUCCESS;
  // VkClearValue clear_values[2];
  // clear_values[0].color.float32[0] = 0.19f;
  // clear_values[0].color.float32[1] = 0.34f;
  // clear_values[0].color.float32[2] = 0.83f;
  // clear_values[0].color.float32[3] = 1.f;
  // clear_values[1].depthStencil.depth = 1.0f;
  // clear_values[1].depthStencil.stencil = 0;

  // VkSemaphore imageAcquiredSemaphore;
  // VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
  // imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
  // imageAcquiredSemaphoreCreateInfo.pNext = NULL;
  // imageAcquiredSemaphoreCreateInfo.flags = 0;

  // VkResult res = vkCreateSemaphore(p_vkrs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
  // assert(res == VK_SUCCESS);

  // // Get the index of the next available swapchain image:
  // res = vkAcquireNextImageKHR(p_vkrs->device, p_vkrs->swap_chain, UINT64_MAX, imageAcquiredSemaphore, VK_NULL_HANDLE,
  //                             &p_vkrs->current_buffer);
  // // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
  // // return codes
  // assert(res == VK_SUCCESS);

  // VkRenderPassBeginInfo rp_begin;
  // rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  // rp_begin.pNext = NULL;
  // rp_begin.renderPass = p_vkrs->render_pass;
  // rp_begin.framebuffer = p_vkrs->framebuffers[p_vkrs->current_buffer];
  // rp_begin.renderArea.offset.x = 0;
  // rp_begin.renderArea.offset.y = 0;
  // rp_begin.renderArea.extent.width = p_vkrs->window_width;
  // rp_begin.renderArea.extent.height = p_vkrs->window_height;
  // rp_begin.clearValueCount = 2;
  // rp_begin.pClearValues = clear_values;

  // vkCmdBeginRenderPass(p_vkrs->cmd, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

  // vkCmdBindPipeline(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline);
  // vkCmdBindDescriptorSets(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline_layout, 0, NUM_DESCRIPTOR_SETS,
  //                         p_vkrs->desc_set.data(), 0, NULL);

  // const VkDeviceSize offsets[1] = {0};
  // vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->cube_vertices.buf, offsets);

  // mvk_init_viewports(p_vkrs, p_vkrs->window_width, p_vkrs->window_height);
  // mvk_init_scissors(p_vkrs, p_vkrs->window_width, p_vkrs->window_height);

  // vkCmdDraw(p_vkrs->cmd, 12 * 3, 1, 0, 0);
  // vkCmdEndRenderPass(p_vkrs->cmd);
  // res = vkEndCommandBuffer(p_vkrs->cmd);
  // const VkCommandBuffer cmd_bufs[] = {p_vkrs->cmd};
  // VkFenceCreateInfo fenceInfo;
  // VkFence drawFence;
  // fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  // fenceInfo.pNext = NULL;
  // fenceInfo.flags = 0;
  // vkCreateFence(p_vkrs->device, &fenceInfo, NULL, &drawFence);

  // VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  // VkSubmitInfo submit_info[1] = {};
  // submit_info[0].pNext = NULL;
  // submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  // submit_info[0].waitSemaphoreCount = 1;
  // submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
  // submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
  // submit_info[0].commandBufferCount = 1;
  // submit_info[0].pCommandBuffers = cmd_bufs;
  // submit_info[0].signalSemaphoreCount = 0;
  // submit_info[0].pSignalSemaphores = NULL;

  // /* Queue the command buffer for execution */
  // res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
  // assert(res == VK_SUCCESS);

  // /* Now present the image in the window */

  // VkPresentInfoKHR present;
  // present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
  // present.pNext = NULL;
  // present.swapchainCount = 1;
  // present.pSwapchains = &p_vkrs->swap_chain;
  // present.pImageIndices = &p_vkrs->current_buffer;
  // present.pWaitSemaphores = NULL;
  // present.waitSemaphoreCount = 0;
  // present.pResults = NULL;

  // /* Make sure command buffer is finished before presenting */
  // do {
  //   res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
  // } while (res == VK_TIMEOUT);

  // assert(res == VK_SUCCESS);
  // res = vkQueuePresentKHR(p_vkrs->present_queue, &present);
  // assert(res == VK_SUCCESS);

  // const struct timespec ts = {1L, 0L};
  // nanosleep(&ts, NULL);

  // /* VULKAN_KEY_END */
  // // if (p_vkrs->save_images)
  // //   write_ppm(p_vkrs, "15-draw_cube");

  // vkDestroySemaphore(p_vkrs->device, imageAcquiredSemaphore, NULL);
  // vkDestroyFence(p_vkrs->device, drawFence, NULL);

  return res;
}

VkResult create_texture_image(vk_render_state *p_vkrs)
{
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load("res/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  printf("loaded texture.png> width:%i height:%i channels:%i\n", texWidth, texHeight, texChannels);

  if (!pixels) {
    return VK_ERROR_UNKNOWN;
  }

  // Copy to buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = imageSize;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult res = vkCreateBuffer(p_vkrs->device, &bufferInfo, nullptr, &stagingBuffer);
  assert(res == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(p_vkrs->device, stagingBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  bool pass = get_memory_type_index_from_properties(p_vkrs, memRequirements.memoryTypeBits,
                                                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                    &allocInfo.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &allocInfo, nullptr, &stagingBufferMemory);
  assert(res == VK_SUCCESS);

  res = vkBindBufferMemory(p_vkrs->device, stagingBuffer, stagingBufferMemory, 0);
  assert(res == VK_SUCCESS);

  // VkResult res = createBuffer(p_vkrs, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
  //                             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &stagingBuffer,
  //                             &stagingBufferMemory);
  // assert(res == VK_SUCCESS);

  void *data;
  res = vkMapMemory(p_vkrs->device, stagingBufferMemory, 0, imageSize, 0, &data);
  assert(res == VK_SUCCESS);
  memcpy(data, pixels, static_cast<size_t>(imageSize));
  vkUnmapMemory(p_vkrs->device, stagingBufferMemory);

  stbi_image_free(pixels);

  // Create Image
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;

  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  res = vkCreateImage(p_vkrs->device, &imageInfo, nullptr, &textureImage);
  assert(res == VK_SUCCESS);

  vkGetImageMemoryRequirements(p_vkrs->device, textureImage, &memRequirements);

  allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  pass = get_memory_type_index_from_properties(p_vkrs, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               &allocInfo.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &allocInfo, nullptr, &textureImageMemory);
  assert(res == VK_SUCCESS);

  res = vkBindImageMemory(p_vkrs->device, textureImage, textureImageMemory, 0);
  assert(res == VK_SUCCESS);

  // Single-Use Command Buffer
  VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
  cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmdBufferAllocInfo.commandPool = p_vkrs->cmd_pool;
  cmdBufferAllocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(p_vkrs->device, &cmdBufferAllocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  // Transition layout
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = image;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;

  VkPipelineStageFlags sourceStage;
  VkPipelineStageFlags destinationStage;

  if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

    sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
  }
  else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
  }
  else {
    throw std::invalid_argument("unsupported layout transition!");
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  // End single time commands
  vkEndCommandBuffer(commandBuffer);

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
  submitInfo.commandBufferCount = 1;
  submitInfo.pCommandBuffers = &commandBuffer;

  vkQueueSubmit(p_vkrs->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
  vkQueueWaitIdle(p_vkrs->graphics_queue);

  vkFreeCommandBuffers(p_vkrs->device, p_vkrs->cmd_pool, 1, &commandBuffer);

  // Transition Image Layout
  VkImageMemoryBarrier barrier{};
  barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
  barrier.oldLayout = oldLayout;
  barrier.newLayout = newLayout;
  barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
  barrier.image = textureImage;
  barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  barrier.subresourceRange.baseMipLevel = 0;
  barrier.subresourceRange.levelCount = 1;
  barrier.subresourceRange.baseArrayLayer = 0;
  barrier.subresourceRange.layerCount = 1;
  barrier.srcAccessMask = 0; // TODO
  barrier.dstAccessMask = 0; // TODO
  vkCmdPipelineBarrier(commandBuffer, 0 /* TODO */, 0 /* TODO */, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  return VK_SUCCESS;
}