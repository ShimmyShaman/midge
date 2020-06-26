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
    .text = "#version 450\n"
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
    .text = "#version 450\n"
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
VkResult handle_resource_commands(vk_render_state *p_vkrs, resource_queue *resource_queue);
VkResult load_texture_from_file(vk_render_state *p_vkrs, const char *const filepath, uint *texture_uid);
VkResult create_empty_render_target(vk_render_state *p_vkrs, const uint width, const uint height, bool use_as_render_target,
                                    uint *texture_uid);
VkResult load_font(vk_render_state *p_vkrs, const char *const filepath, float height, uint *resource_uid);

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
  // vkrs.depth.format = VK_FORMAT_UNDEFINED;
  vkrs.xcb_winfo = &winfo;
  vkrs.textures.allocated = 0;

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
  MRT_RUN(mvk_init_enumerate_device(&vkrs));
  mxcb_init_window(&winfo, vkrs.window_width, vkrs.window_height);
  MRT_RUN(mvk_init_swapchain_extension(&vkrs));
  MRT_RUN(mvk_init_device(&vkrs));

  MRT_RUN(mvk_init_command_pool(&vkrs));
  mvk_init_device_queue(&vkrs);
  MRT_RUN(mvk_init_command_buffer(&vkrs));
  // MRT_RUN(mvk_execute_begin_command_buffer(&vkrs));
  MRT_RUN(mvk_init_swapchain(&vkrs));
  // MRT_RUN(mvk_init_depth_buffer(&vkrs));
  MRT_RUN(mvk_init_headless_image(&vkrs));
  MRT_RUN(mvk_init_uniform_buffer(&vkrs));
  MRT_RUN(mvk_init_descriptor_and_pipeline_layouts(&vkrs));
  MRT_RUN(mvk_init_present_renderpass(&vkrs));
  MRT_RUN(mvk_init_offscreen_renderpass(&vkrs));
  MRT_RUN(mvk_init_shader(&vkrs, &vertex_shader, 0));
  MRT_RUN(mvk_init_shader(&vkrs, &fragment_shader, 1));
  MRT_RUN(mvk_init_textured_render_prog(&vkrs));
  MRT_RUN(mvk_init_font_render_prog(&vkrs));
  MRT_RUN(mvk_init_framebuffers(&vkrs, depth_present));
  // MRT_RUN(mvk_init_cube_vertices(&vkrs, g_vb_solid_face_colors_Data, sizeof(g_vb_solid_face_colors_Data),
  //                                sizeof(g_vb_solid_face_colors_Data[0]), false));
  MRT_RUN(mvk_init_shape_vertices(&vkrs));
  MRT_RUN(mvk_init_descriptor_pool(&vkrs, false));
  MRT_RUN(mvk_init_descriptor_set(&vkrs, false));
  MRT_RUN(mvk_init_pipeline_cache(&vkrs));
  MRT_RUN(mvk_init_pipeline(&vkrs)); // Maybe false?

  // MRT_RUN(draw_cube(&vkrs));

  // -- Update
  printf("Vulkan Initialized!\n");
  mxcb_update_window(&winfo);
  render_thread->render_thread_initialized = true;
  // printf("mrt-2: %p\n", thr);
  // printf("mrt-2: %p\n", &winfo);
  uint frame_updates = 0;
  while (!thr->should_exit && !winfo.shouldExit) {
    // Resource Commands
    pthread_mutex_lock(&render_thread->resource_queue.mutex);
    if (render_thread->resource_queue.count) {
      handle_resource_commands(&vkrs, &render_thread->resource_queue);
      render_thread->resource_queue.count = 0;
    }
    pthread_mutex_unlock(&render_thread->resource_queue.mutex);

    // Render Commands
    if (!render_thread->renderer_queue->in_use && render_thread->renderer_queue->count) {
      // printf("received render queue in renderer : %i items\n", render_thread->renderer_queue->count);
      render_thread->renderer_queue->in_use = true;

      render_through_queue(&vkrs, render_thread->renderer_queue);
      render_thread->renderer_queue->count = 0;
      render_thread->renderer_queue->items = NULL;
      render_thread->renderer_queue->in_use = false;
      ++frame_updates;
    }

    mxcb_update_window(&winfo);
  }
  printf("AfterUpdate! frame_updates = %i\n", frame_updates);

  // -- Cleanup
  mvk_destroy_pipeline(&vkrs);
  mvk_destroy_pipeline_cache(&vkrs);
  mvk_destroy_descriptor_pool(&vkrs);
  // printf("mrt-4\n");
  mvk_destroy_resources(&vkrs);
  mvk_destroy_framebuffers(&vkrs);
  mvk_destroy_shaders(&vkrs);
  mvk_destroy_renderpass(&vkrs);
  mvk_destroy_textured_render_prog(&vkrs);
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

int write_desc_and_queue_render_data(vk_render_state *p_vkrs, unsigned long size_in_bytes, void *p_src,
                                     VkDescriptorBufferInfo *descriptor_buffer_info)
{
  if (p_vkrs->render_data_buffer.frame_utilized_amount + size_in_bytes >= p_vkrs->render_data_buffer.allocated_size) {
    printf("Requested more data then remaining in render_data_buffer\n");
    return 1;
  }

  // TODO -- refactor this
  descriptor_buffer_info->buffer = p_vkrs->render_data_buffer.buffer;
  descriptor_buffer_info->offset = p_vkrs->render_data_buffer.frame_utilized_amount;
  descriptor_buffer_info->range = size_in_bytes;

  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].p_source = p_src;
  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].dest_offset =
      p_vkrs->render_data_buffer.frame_utilized_amount;
  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count++].size_in_bytes = size_in_bytes;
  p_vkrs->render_data_buffer.frame_utilized_amount +=
      ((size_in_bytes / p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment) + 1UL) *
      p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment;

  return 0;
}

int set_viewport_cmd(vk_render_state *p_vkrs, float x, float y, float width, float height)
{
  VkViewport viewport;
  viewport.x = x;
  viewport.y = y;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(p_vkrs->cmd, 0, 1, &viewport);

  return 0;
}

int set_scissor_cmd(vk_render_state *p_vkrs, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  VkRect2D scissor;
  scissor.offset.x = x;
  scissor.offset.y = y;
  scissor.extent.width = width;
  scissor.extent.height = height;
  vkCmdSetScissor(p_vkrs->cmd, 0, 1, &scissor);

  return 0;
}

VkResult render_sequence(vk_render_state *p_vkrs, node_render_sequence *sequence)
{
  // Descriptor Writes
  VkResult res;

  coloured_rect_draw_data rect_draws[128];
  int rect_draws_index = 0;

  // TODO -- this isn't very seamly
  const int COPY_BUFFER_SIZE = 8192;
  u_char copy_buffer[COPY_BUFFER_SIZE];
  u_int32_t copy_buffer_used = 0;

  p_vkrs->render_data_buffer.frame_utilized_amount = 0;
  p_vkrs->render_data_buffer.queued_copies_count = 0U;

  mat4 vpc;
  VkDescriptorBufferInfo vpc_desc_buffer_info;
  {
    // Construct the Vulkan View/Projection/Clip for the render target image
    mat4 view;
    mat4 proj;
    mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};

    glm_lookat((vec3){0, 0, -10}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)&view);
    glm_ortho_default((float)sequence->image_width / sequence->image_height, (vec4 *)&proj);
    glm_mat4_mul((vec4 *)&proj, (vec4 *)&view, (vec4 *)&vpc);
    glm_mat4_mul((vec4 *)&clip, (vec4 *)&vpc, (vec4 *)&vpc);

    write_desc_and_queue_render_data(p_vkrs, sizeof(mat4), &vpc, &vpc_desc_buffer_info);
  }

  for (int j = 0; j < sequence->render_command_count; ++j) {

    render_command *cmd = &sequence->render_commands[j];
    switch (cmd->type) {
    case RENDER_COMMAND_COLORED_RECTANGLE: {
      // Set
      coloured_rect_draw_data &rect_draw_data = rect_draws[rect_draws_index++];

      // Vertex Data
      rect_draw_data.vert.scale[0] = 2.f * cmd->data.colored_rect_info.width / (float)sequence->image_height;
      rect_draw_data.vert.scale[1] = 2.f * cmd->data.colored_rect_info.height / (float)sequence->image_height;
      rect_draw_data.vert.offset[0] = -1.0f + 2.0f * (float)cmd->x / (float)(sequence->image_width) +
                                      1.0f * (float)cmd->data.colored_rect_info.width / (float)(sequence->image_width);
      rect_draw_data.vert.offset[1] = -1.0f + 2.0f * (float)cmd->y / (float)(sequence->image_height) +
                                      1.0f * (float)cmd->data.colored_rect_info.height / (float)(sequence->image_height);

      // Fragment Data
      rect_draw_data.frag.tint_color[0] = cmd->data.colored_rect_info.color.r;
      rect_draw_data.frag.tint_color[1] = cmd->data.colored_rect_info.color.g;
      rect_draw_data.frag.tint_color[2] = cmd->data.colored_rect_info.color.b;
      rect_draw_data.frag.tint_color[3] = cmd->data.colored_rect_info.color.a;

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
        viewport.width = (float)sequence->image_width;
        viewport.height = (float)sequence->image_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(p_vkrs->cmd, 0, 1, &viewport);
      }

      // Apply scissor/clipping rectangle
      {
        VkRect2D scissor;
        scissor.offset.x = (int32_t)(cmd->x);
        scissor.offset.y = (int32_t)(cmd->y);
        scissor.extent.width = (uint32_t)(cmd->data.colored_rect_info.width);
        scissor.extent.height = (uint32_t)(cmd->data.colored_rect_info.height);
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

    case RENDER_COMMAND_TEXTURED_RECTANGLE: {
      // // Set
      coloured_rect_draw_data &rect_draw_data = rect_draws[rect_draws_index++];

      // Vertex Data
      rect_draw_data.vert.scale[0] = 2.f * cmd->data.colored_rect_info.width / (float)sequence->image_height;
      rect_draw_data.vert.scale[1] = 2.f * cmd->data.colored_rect_info.height / (float)sequence->image_height;
      rect_draw_data.vert.offset[0] = -1.0f + 2.0f * (float)cmd->x / (float)(sequence->image_width) +
                                      1.0f * (float)cmd->data.colored_rect_info.width / (float)(sequence->image_width);
      rect_draw_data.vert.offset[1] = -1.0f + 2.0f * (float)cmd->y / (float)(sequence->image_height) +
                                      1.0f * (float)cmd->data.colored_rect_info.height / (float)(sequence->image_height);

      // // Fragment Data
      // glm_vec4_copy((float *)cmd->data, rect_draw_data.frag.tint_color);

      // Queue Buffer Write
      const unsigned int MAX_DESC_SET_WRITES = 8;
      VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
      VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
      int buffer_info_index = 0;
      int write_index = 0;

      // // TODO -- refactor this
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

      // VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
      // frag_ubo_info->buffer = p_vkrs->render_data_buffer.buffer;
      // frag_ubo_info->offset = p_vkrs->render_data_buffer.frame_utilized_amount;
      // frag_ubo_info->range = sizeof(rect_draw_data.frag);

      // p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].p_source = &rect_draw_data.frag;
      // p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].dest_offset =
      //     p_vkrs->render_data_buffer.frame_utilized_amount;
      // p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count++].size_in_bytes =
      //     sizeof(rect_draw_data.frag);
      // p_vkrs->render_data_buffer.frame_utilized_amount +=
      //     ((frag_ubo_info->range / p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment) + 1UL) *
      //     p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment;

      // Setup viewport:
      {
        VkViewport viewport;
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = (float)sequence->image_width;
        viewport.height = (float)sequence->image_height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(p_vkrs->cmd, 0, 1, &viewport);
      }

      // Apply scissor/clipping rectangle
      {
        VkRect2D scissor;
        scissor.offset.x = (int32_t)(cmd->x);
        scissor.offset.y = (int32_t)(cmd->y);
        scissor.extent.width = (uint32_t)(cmd->data.colored_rect_info.width);
        scissor.extent.height = (uint32_t)(cmd->data.colored_rect_info.height);
        // scissor.extent.width = (uint32_t)sequence->image_width;
        // scissor.extent.height = (uint32_t)sequence->image_height;
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
      setAllocInfo.pSetLayouts = &p_vkrs->texture_prog.desc_layout;

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

      VkDescriptorImageInfo image_sampler_info = {};
      image_sampler_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_sampler_info.imageView = p_vkrs->textures.samples[cmd->data.textured_rect_info.texture_uid - RESOURCE_UID_BEGIN].view;
      image_sampler_info.sampler =
          p_vkrs->textures.samples[cmd->data.textured_rect_info.texture_uid - RESOURCE_UID_BEGIN].sampler;

      // Element Fragment Shader Combined Image Sampler
      write = &writes[write_index++];
      write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write->pNext = NULL;
      write->dstSet = desc_set;
      write->descriptorCount = 1;
      write->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      write->pImageInfo = &image_sampler_info;
      write->dstArrayElement = 0;
      write->dstBinding = 2;

      vkUpdateDescriptorSets(p_vkrs->device, write_index, writes, 0, NULL);

      vkCmdBindDescriptorSets(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->texture_prog.pipeline_layout, 0, 1, &desc_set,
                              0, NULL);

      vkCmdBindPipeline(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->texture_prog.pipeline);

      const VkDeviceSize offsets[1] = {0};
      vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->textured_shape_vertices.buf, offsets);

      vkCmdDraw(p_vkrs->cmd, 2 * 3, 1, 0, 0);
    } break;

    case RENDER_COMMAND_PRINT_TEXT: {

      // Get the font image
      loaded_font_info *font = NULL;
      for (int f = 0; f < p_vkrs->loaded_fonts.count; ++f) {
        if (p_vkrs->loaded_fonts.fonts[f].resource_uid == cmd->data.print_text.font_resource_uid) {
          font = &p_vkrs->loaded_fonts.fonts[f];
          break;
        }
      }
      if (!font) {
        printf("Could not find requested font\n");
        return VK_ERROR_UNKNOWN;
      }
      sampled_image *font_image = &p_vkrs->textures.samples[font->resource_uid - RESOURCE_UID_BEGIN];

      float align_x = cmd->x;
      float align_y = cmd->y;

      int text_length = strlen(cmd->data.print_text.text);
      for (int c = 0; c < text_length; ++c) {

        char letter = cmd->data.print_text.text[c];
        if (letter < 32 || letter > 127) {
          printf("TODO character not supported.\n");
          continue;
        }

        // Source texture bounds
        stbtt_aligned_quad q;
        stbtt_GetBakedQuad(font->char_data, font_image->width, font_image->height, letter - 32, &align_x, &align_y, &q,
                           1); // 1=opengl & d3d10+,0=d3d9

        // stbtt_bakedchar *b = font->char_data + (letter - 32);
        // q.y0 += b->y1 - b->y0;
        // q.y1 += b->y1 - b->y0;
        // {
        //   // opengl y invert
        //   float t = q.y1;
        //   q.y1 += (q.y1 - q.y0);
        //   q.y0 = t;
        // }
        float width = q.x1 - q.x0;
        float height = q.y1 - q.y0;

        printf("baked_quad: s0=%.2f s1==%.2f t0=%.2f t1=%.2f x0=%.2f x1=%.2f y0=%.2f y1=%.2f\n", q.s0, q.s1, q.t0, q.t1, q.x0,
               q.x1, q.y0, q.y1);
        printf("align_x=%.2f align_y=%.2f\n", align_x, align_y);

        // Vertex Uniform Buffer Object
        vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer[copy_buffer_used];
        copy_buffer_used += sizeof(vert_data_scale_offset);

        vert_ubo_data->scale.x = 2.f * width / (float)sequence->image_height;
        vert_ubo_data->scale.y = 2.f * height / (float)sequence->image_height;
        vert_ubo_data->offset.x =
            -1.0f + 2.0f * (float)q.x0 / (float)(sequence->image_width) + 1.0f * (float)width / (float)(sequence->image_width);
        vert_ubo_data->offset.y =
            -1.0f + 2.0f * (float)q.y0 / (float)(sequence->image_height) + 1.0f * (float)height / (float)(sequence->image_height);

        // Fragment Data
        frag_ubo_tint_texcoordbounds *frag_ubo_data = (frag_ubo_tint_texcoordbounds *)&copy_buffer[copy_buffer_used];
        copy_buffer_used += sizeof(frag_ubo_tint_texcoordbounds);

        memcpy(&frag_ubo_data->tint, &cmd->data.print_text.color, sizeof(float) * 4);
        frag_ubo_data->tex_coord_bounds.s0 = q.s0;
        frag_ubo_data->tex_coord_bounds.s1 = q.s1;
        frag_ubo_data->tex_coord_bounds.t0 = q.t0;
        frag_ubo_data->tex_coord_bounds.t1 = q.t1;

        // Setup viewport and clip
        set_viewport_cmd(p_vkrs, 0, 0, (float)sequence->image_width, (float)sequence->image_height);
        set_scissor_cmd(p_vkrs, q.x0, q.y0, width, height);

        // Allocate the descriptor set from the pool.
        VkDescriptorSetAllocateInfo setAllocInfo = {};
        setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocInfo.pNext = NULL;
        setAllocInfo.descriptorPool = p_vkrs->desc_pool;
        setAllocInfo.descriptorSetCount = 1;
        setAllocInfo.pSetLayouts = &p_vkrs->font_prog.desc_layout;

        unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
        res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
        assert(res == VK_SUCCESS);

        VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
        p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

        // Queue Buffer and Descriptor Writes
        const unsigned int MAX_DESC_SET_WRITES = 8;
        VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
        VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
        int buffer_info_index = 0;
        int write_index = 0;

        VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
        write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);

        VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
        write_desc_and_queue_render_data(p_vkrs, sizeof(frag_ubo_tint_texcoordbounds), frag_ubo_data, frag_ubo_info);

        // Global Vertex Shader Uniform Buffer
        VkWriteDescriptorSet *write = &writes[write_index++];
        write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write->pNext = NULL;
        write->dstSet = desc_set;
        write->descriptorCount = 1;
        write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        write->pBufferInfo = &vpc_desc_buffer_info;
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

        VkDescriptorImageInfo image_sampler_info = {};
        image_sampler_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_sampler_info.imageView = font_image->view;
        image_sampler_info.sampler = font_image->sampler;

        // Element Fragment Shader Combined Image Sampler
        write = &writes[write_index++];
        write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write->pNext = NULL;
        write->dstSet = desc_set;
        write->descriptorCount = 1;
        write->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write->pImageInfo = &image_sampler_info;
        write->dstArrayElement = 0;
        write->dstBinding = 3;

        vkUpdateDescriptorSets(p_vkrs->device, write_index, writes, 0, NULL);

        vkCmdBindDescriptorSets(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->font_prog.pipeline_layout, 0, 1, &desc_set,
                                0, NULL);

        vkCmdBindPipeline(p_vkrs->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->font_prog.pipeline);

        const VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(p_vkrs->cmd, 0, 1, &p_vkrs->textured_shape_vertices.buf, offsets);

        vkCmdDraw(p_vkrs->cmd, 2 * 3, 1, 0, 0);
      }
    } break;

    default:
      printf("COULD NOT HANDLE RENDER COMMAND TYPE=%i\n", cmd->type);
      continue;
    }
  }

  if (copy_buffer_used >= COPY_BUFFER_SIZE) {
    printf("ERROR Copy Buffer Allocation is insufficient!!\n");
    return VK_ERROR_UNKNOWN;
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

VkResult handle_resource_commands(vk_render_state *p_vkrs, resource_queue *resource_queue)
{
  for (int i = 0; i < resource_queue->count; ++i) {
    resource_command *resource_cmd = &resource_queue->commands[i];

    switch (resource_cmd->type) {
    case RESOURCE_COMMAND_LOAD_TEXTURE: {
      VkResult res = load_texture_from_file(p_vkrs, resource_cmd->data.path, resource_cmd->p_uid);
      assert(res == VK_SUCCESS);

    } break;
    case RESOURCE_COMMAND_CREATE_TEXTURE: {
      VkResult res =
          create_empty_render_target(p_vkrs, resource_cmd->data.create_texture.width, resource_cmd->data.create_texture.height,
                                     resource_cmd->data.create_texture.use_as_render_target, resource_cmd->p_uid);
      assert(res == VK_SUCCESS);

    } break;
    case RESOURCE_COMMAND_LOAD_FONT: {
      VkResult res = load_font(p_vkrs, resource_cmd->data.font.path, resource_cmd->data.font.height, resource_cmd->p_uid);
      assert(res == VK_SUCCESS);

    } break;

    default:
      printf("COULD NOT HANDLE RESOURCE COMMAND TYPE=%i\n", resource_cmd->type);
      continue;
    }
  }

  return VK_SUCCESS;
}

VkResult render_through_queue(vk_render_state *p_vkrs, renderer_queue *render_queue)
{
  VkResult res;

  for (int i = 0; i < render_queue->count; ++i) {

    node_render_sequence *sequence = render_queue->items[i];

    if (sequence->render_command_count < 1) {
      return VK_ERROR_UNKNOWN;
    }

    printf("sequence: rt:%i cmd_count:%i\n", sequence->render_target, sequence->render_command_count);

    switch (sequence->render_target) {
    case NODE_RENDER_TARGET_PRESENT: {
      VkSemaphore imageAcquiredSemaphore;
      VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
      imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      imageAcquiredSemaphoreCreateInfo.pNext = NULL;
      imageAcquiredSemaphoreCreateInfo.flags = 0;

      res = vkCreateSemaphore(p_vkrs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
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
      rp_begin.renderPass = p_vkrs->present_render_pass;
      rp_begin.framebuffer = p_vkrs->framebuffers[p_vkrs->current_buffer];
      rp_begin.renderArea.offset.x = 0;
      rp_begin.renderArea.offset.y = 0;
      rp_begin.renderArea.extent.width = sequence->image_width;
      rp_begin.renderArea.extent.height = sequence->image_height;
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
    } break;
    case NODE_RENDER_TARGET_IMAGE: {
      // Obtain the target image
      sampled_image *target_image = &p_vkrs->textures.samples[sequence->data.target_image.image_uid - RESOURCE_UID_BEGIN];

      if (!target_image->framebuffer) {
        // Create?
        VkFramebufferCreateInfo framebuffer_create_info{};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = NULL;
        framebuffer_create_info.renderPass = p_vkrs->offscreen_render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &target_image->view;
        framebuffer_create_info.width = target_image->width;
        framebuffer_create_info.height = target_image->height;
        framebuffer_create_info.layers = 1;

        res = vkCreateFramebuffer(p_vkrs->device, &framebuffer_create_info, NULL, &target_image->framebuffer);
        assert(res == VK_SUCCESS);
        // res = vkResetDescriptorPool(p_vkrs->device, p_vkrs->desc_pool, 0);
        // assert(res == VK_SUCCESS);
        // p_vkrs->descriptor_sets_count = 0U;
      }

      // Begin Command Buffer Recording
      res = vkResetCommandPool(p_vkrs->device, p_vkrs->cmd_pool, 0);
      assert(res == VK_SUCCESS);

      VkCommandBufferBeginInfo beginInfo{};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
      beginInfo.pInheritanceInfo = NULL;                             // Optional
      vkBeginCommandBuffer(p_vkrs->cmd, &beginInfo);

      VkClearValue clear_values[1];
      clear_values[0].color.float32[0] = 0.34f;
      clear_values[0].color.float32[1] = 0.83f;
      clear_values[0].color.float32[2] = 0.19f;
      clear_values[0].color.float32[3] = 1.f;

      VkRenderPassBeginInfo rp_begin;
      rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      rp_begin.pNext = NULL;
      rp_begin.renderPass = p_vkrs->offscreen_render_pass;
      rp_begin.framebuffer = target_image->framebuffer;
      rp_begin.renderArea.offset.x = 0;
      rp_begin.renderArea.offset.y = 0;
      rp_begin.renderArea.extent.width = target_image->width;
      rp_begin.renderArea.extent.height = target_image->height;
      rp_begin.clearValueCount = 1;
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
      submit_info[0].waitSemaphoreCount = 0;
      submit_info[0].pWaitSemaphores = NULL;
      submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
      submit_info[0].commandBufferCount = 1;
      const VkCommandBuffer cmd_bufs[] = {p_vkrs->cmd};
      submit_info[0].pCommandBuffers = cmd_bufs;
      submit_info[0].signalSemaphoreCount = 0;
      submit_info[0].pSignalSemaphores = NULL;

      /* Queue the command buffer for execution */
      res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
      assert(res == VK_SUCCESS);

      /* Make sure command buffer is finished before leaving */
      do {
        res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
      } while (res == VK_TIMEOUT);
      assert(res == VK_SUCCESS);
      res = vkResetFences(p_vkrs->device, 1, &drawFence);
      assert(res == VK_SUCCESS);

      vkDestroyFence(p_vkrs->device, drawFence, NULL);
    } break;
    default:
      return VK_ERROR_UNKNOWN;
    }
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
  // rp_begin.renderPass = p_vkrs->present_render_pass;
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

VkCommandBuffer beginSingleTimeCommands(vk_render_state *p_vkrs)
{
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = p_vkrs->cmd_pool;
  allocInfo.commandBufferCount = 1;

  VkCommandBuffer commandBuffer;
  vkAllocateCommandBuffers(p_vkrs->device, &allocInfo, &commandBuffer);

  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  vkBeginCommandBuffer(commandBuffer, &beginInfo);

  return commandBuffer;
}

VkResult transitionImageLayout(vk_render_state *p_vkrs, VkImage image, VkFormat format, VkImageLayout oldLayout,
                               VkImageLayout newLayout)
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(p_vkrs);

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
    printf("Unsupported layout transition\n");
    return VK_ERROR_UNKNOWN;
  }

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

  // endSingleTimeCommands(p_vkrs, commandBuffer);
  {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(p_vkrs->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(p_vkrs->graphics_queue);

    vkFreeCommandBuffers(p_vkrs->device, p_vkrs->cmd_pool, 1, &commandBuffer);
  }
  return VK_SUCCESS;
}

void copyBufferToImage(vk_render_state *p_vkrs, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
  VkCommandBuffer commandBuffer = beginSingleTimeCommands(p_vkrs);

  VkBufferImageCopy region{};
  region.bufferOffset = 0;
  region.bufferRowLength = 0;
  region.bufferImageHeight = 0;
  region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  region.imageSubresource.mipLevel = 0;
  region.imageSubresource.baseArrayLayer = 0;
  region.imageSubresource.layerCount = 1;
  region.imageOffset = {0, 0, 0};
  region.imageExtent = {width, height, 1};

  vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

  // endSingleTimeCommands(p_vkrs, commandBuffer);
  {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(p_vkrs->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(p_vkrs->graphics_queue);

    vkFreeCommandBuffers(p_vkrs->device, p_vkrs->cmd_pool, 1, &commandBuffer);
  }
}

VkResult load_image_sampler(vk_render_state *p_vkrs, const int texWidth, const int texHeight, const int texChannels,
                            bool use_as_render_target, const unsigned char *const pixels, sampled_image *image_sampler)
{
  image_sampler->width = texWidth;
  image_sampler->height = texHeight;
  image_sampler->size = texWidth * texHeight * 4; // TODO

  // Copy to buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = image_sampler->size;
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

  void *data;
  res = vkMapMemory(p_vkrs->device, stagingBufferMemory, 0, image_sampler->size, 0, &data);
  assert(res == VK_SUCCESS);
  memcpy(data, pixels, static_cast<size_t>(image_sampler->size));
  vkUnmapMemory(p_vkrs->device, stagingBufferMemory);

  // Create Image
  VkImageUsageFlags image_usage;
  if (use_as_render_target) {
    image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_sampler->format = p_vkrs->format;
  }
  else {
    image_usage = 0;
    image_sampler->format = VK_IMAGE_FORMAT;
  }
  VkImageCreateInfo imageInfo{};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = static_cast<uint32_t>(texWidth);
  imageInfo.extent.height = static_cast<uint32_t>(texHeight);
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = image_sampler->format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | image_usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  res = vkCreateImage(p_vkrs->device, &imageInfo, nullptr, &image_sampler->image);
  assert(res == VK_SUCCESS);

  vkGetImageMemoryRequirements(p_vkrs->device, image_sampler->image, &memRequirements);

  allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  pass = get_memory_type_index_from_properties(p_vkrs, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                               &allocInfo.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &allocInfo, nullptr, &image_sampler->memory);
  assert(res == VK_SUCCESS);

  res = vkBindImageMemory(p_vkrs->device, image_sampler->image, image_sampler->memory, 0);
  assert(res == VK_SUCCESS);

  res = transitionImageLayout(p_vkrs, image_sampler->image, image_sampler->format, VK_IMAGE_LAYOUT_UNDEFINED,
                              VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  assert(res == VK_SUCCESS);
  copyBufferToImage(p_vkrs, stagingBuffer, image_sampler->image, static_cast<uint32_t>(texWidth),
                    static_cast<uint32_t>(texHeight));
  res = transitionImageLayout(p_vkrs, image_sampler->image, image_sampler->format, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                              VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  assert(res == VK_SUCCESS);

  // Destroy staging resources
  vkDestroyBuffer(p_vkrs->device, stagingBuffer, nullptr);
  vkFreeMemory(p_vkrs->device, stagingBufferMemory, nullptr);

  // Image View
  VkImageViewCreateInfo viewInfo{};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image_sampler->image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = image_sampler->format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  res = vkCreateImageView(p_vkrs->device, &viewInfo, nullptr, &image_sampler->view);
  assert(res == VK_SUCCESS);

  // Sampler
  VkSamplerCreateInfo samplerInfo{};
  samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
  samplerInfo.magFilter = VK_FILTER_LINEAR;
  samplerInfo.minFilter = VK_FILTER_LINEAR;
  samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
  samplerInfo.anisotropyEnable = VK_TRUE;
  samplerInfo.maxAnisotropy = 16.0f;
  samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
  samplerInfo.unnormalizedCoordinates = VK_FALSE;
  samplerInfo.compareEnable = VK_FALSE;
  samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
  samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

  res = vkCreateSampler(p_vkrs->device, &samplerInfo, nullptr, &image_sampler->sampler);
  assert(res == VK_SUCCESS);

  // TODO ??
  image_sampler->framebuffer = NULL;

  return res;
}

VkResult load_texture_from_file(vk_render_state *p_vkrs, const char *const filepath, uint *resource_uid)
{
  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    return VK_ERROR_UNKNOWN;
  };

  // TODO -- Refactor
  if (p_vkrs->textures.allocated < p_vkrs->textures.count + 1) {
    int new_allocated = p_vkrs->textures.allocated + 4 + p_vkrs->textures.allocated / 4;
    sampled_image *new_ary = (sampled_image *)malloc(sizeof(sampled_image) * new_allocated);

    if (p_vkrs->textures.allocated) {
      memcpy(new_ary, p_vkrs->textures.samples, sizeof(sampled_image) * p_vkrs->textures.allocated);
      free(p_vkrs->textures.samples);
    }
    p_vkrs->textures.samples = new_ary;
    p_vkrs->textures.allocated = new_allocated;
  }

  load_image_sampler(p_vkrs, texWidth, texHeight, texChannels, false, pixels, &p_vkrs->textures.samples[p_vkrs->textures.count]);
  *resource_uid = RESOURCE_UID_BEGIN + p_vkrs->textures.count;
  ++p_vkrs->textures.count;

  stbi_image_free(pixels);

  printf("loaded %s> width:%i height:%i channels:%i\n", filepath, texWidth, texHeight, texChannels);

  return VK_SUCCESS;
}

VkResult create_empty_render_target(vk_render_state *p_vkrs, const uint width, const uint height, bool use_as_render_target,
                                    uint *resource_uid)
{
  int texChannels = 4;
  stbi_uc *pixels = (stbi_uc *)malloc(sizeof(stbi_uc) * width * height * texChannels);
  VkDeviceSize imageSize = width * height * 4;
  for (int i = 0; i < (int)imageSize; ++i) {
    pixels[i] = 255;
  }

  if (!pixels) {
    return VK_ERROR_UNKNOWN;
  };

  // TODO -- Refactor
  if (p_vkrs->textures.allocated < p_vkrs->textures.count + 1) {
    int new_allocated = p_vkrs->textures.allocated + 4 + p_vkrs->textures.allocated / 4;
    sampled_image *new_ary = (sampled_image *)malloc(sizeof(sampled_image) * new_allocated);

    if (p_vkrs->textures.allocated) {
      memcpy(new_ary, p_vkrs->textures.samples, sizeof(sampled_image) * p_vkrs->textures.allocated);
      free(p_vkrs->textures.samples);
    }
    p_vkrs->textures.samples = new_ary;
    p_vkrs->textures.allocated = new_allocated;
  }

  load_image_sampler(p_vkrs, width, height, texChannels, use_as_render_target, pixels,
                     &p_vkrs->textures.samples[p_vkrs->textures.count]);
  *resource_uid = RESOURCE_UID_BEGIN + p_vkrs->textures.count;
  ++p_vkrs->textures.count;

  stbi_image_free(pixels);

  printf("generated empty texture> width:%i height:%i channels:%i\n", width, height, texChannels);

  return VK_SUCCESS;
}

VkResult load_font(vk_render_state *p_vkrs, const char *const filepath, float height, uint *resource_uid)
{
  // Font is a common resource -- check font cache for existing
  char *font_name;
  {

    int index_of_last_slash = -1;
    for (int i = 0;; i++) {
      if (filepath[i] == '\0') {
        printf("INVALID FORMAT filepath='%s'\n", filepath);
        return VK_ERROR_UNKNOWN;
      }
      if (filepath[i] == '.') {
        int si = index_of_last_slash >= 0 ? (index_of_last_slash + 1) : 0;
        font_name = (char *)malloc(sizeof(char) * (i - si + 1));
        strncpy(font_name, filepath + si, i - si);
        font_name[i - si] = '\0';
        break;
      }
      else if (filepath[i] == '\\' || filepath[i] == '/') {
        index_of_last_slash = i;
      }
    }

    for (int i = 0; i < p_vkrs->loaded_fonts.count; ++i) {
      if (!strcmp(p_vkrs->loaded_fonts.fonts[i].name, font_name)) {
        *resource_uid = p_vkrs->loaded_fonts.fonts[i].resource_uid;

        printf("using cached font texture> name:%s height:%.2f resource_uid:%u\n", font_name, height, *resource_uid);
        free(font_name);

        return VK_SUCCESS;
      }
    }
  }

  stbi_uc ttf_buffer[1 << 20];
  fread(ttf_buffer, 1, 1 << 20, fopen(filepath, "rb"));

  const int texWidth = 256, texHeight = 256, texChannels = 4;
  stbi_uc temp_bitmap[texWidth * texHeight];
  stbtt_bakedchar *cdata = (stbtt_bakedchar *)malloc(sizeof(stbtt_bakedchar) * 96);           // ASCII 32..126 is 95 glyphs
  stbtt_BakeFontBitmap(ttf_buffer, 0, 32.0, temp_bitmap, texWidth, texHeight, 32, 96, cdata); // no guarantee this fits!

  stbi_uc pixels[texWidth * texHeight * 4];
  {
    int p = 0;
    for (int i = 0; i < texWidth * texHeight; ++i) {
      pixels[p++] = temp_bitmap[i];
      pixels[p++] = temp_bitmap[i];
      pixels[p++] = temp_bitmap[i];
      pixels[p++] = 255;
    }
  }

  // TODO -- Refactor
  if (p_vkrs->textures.allocated < p_vkrs->textures.count + 1) {
    int new_allocated = p_vkrs->textures.allocated + 4 + p_vkrs->textures.allocated / 4;
    sampled_image *new_ary = (sampled_image *)malloc(sizeof(sampled_image) * new_allocated);

    if (p_vkrs->textures.allocated) {
      memcpy(new_ary, p_vkrs->textures.samples, sizeof(sampled_image) * p_vkrs->textures.allocated);
      free(p_vkrs->textures.samples);
    }
    p_vkrs->textures.samples = new_ary;
    p_vkrs->textures.allocated = new_allocated;
  }

  load_image_sampler(p_vkrs, texWidth, texHeight, texChannels, false, pixels, &p_vkrs->textures.samples[p_vkrs->textures.count]);
  *resource_uid = RESOURCE_UID_BEGIN + p_vkrs->textures.count;
  ++p_vkrs->textures.count;

  // Font is a common resource -- cache so multiple loads reference the same resource uid
  {
    if (p_vkrs->loaded_fonts.allocated < p_vkrs->loaded_fonts.count + 1) {
      int new_allocated = p_vkrs->loaded_fonts.allocated + 4 + p_vkrs->loaded_fonts.allocated / 4;
      loaded_font_info *new_ary = (loaded_font_info *)malloc(sizeof(loaded_font_info) * new_allocated);

      if (p_vkrs->loaded_fonts.allocated) {
        memcpy(new_ary, p_vkrs->loaded_fonts.fonts, sizeof(loaded_font_info) * p_vkrs->loaded_fonts.allocated);
        free(p_vkrs->loaded_fonts.fonts);
      }
      p_vkrs->loaded_fonts.fonts = new_ary;
      p_vkrs->loaded_fonts.allocated = new_allocated;
    }

    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].name = font_name;
    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].resource_uid = *resource_uid;
    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count++].char_data = cdata;
  }

  printf("generated font texture> name:%s height:%.2f resource_uid:%u\n", font_name, height, *resource_uid);

  return VK_SUCCESS;
}