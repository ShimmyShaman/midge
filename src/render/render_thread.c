/* render_thread.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "stb/stb_truetype.h"

#include "cglm/include/cglm/cglm.h"

// #include "m_threads.h"
#include "core/core_definitions.h"
#include "core/midge_app.h"
#include "env/environment_definitions.h"
#include "mc_error_handling.h"
#include "render/mc_vk_utils.h"
#include "render/render_thread.h"

struct _mrt_resize_args {
  render_thread_info *rt;
  vk_render_state *vkrs;
};

int _mrt_handle_window_resize(mxcb_window_info *xcb_winfo, uint32_t prev_width, uint32_t prev_height, void *state)
{
  VkResult res;

  // puts("_mrt_handle_window_resize:");
  // midge_error_print_thread_info();

  struct _mrt_resize_args *args = (struct _mrt_resize_args *)state;

  if (xcb_winfo->width != args->vkrs->swap_chain.extents.width ||
      xcb_winfo->height != args->vkrs->swap_chain.extents.height) {
    // Invalidate the swap chain
    pthread_mutex_lock(&args->vkrs->swap_chain.surface_specs.mutex);
    args->vkrs->swap_chain.surface_specs.width = xcb_winfo->width;
    args->vkrs->swap_chain.surface_specs.height = xcb_winfo->height;
    args->vkrs->swap_chain.surface_specs.valid = false;
    pthread_mutex_unlock(&args->vkrs->swap_chain.surface_specs.mutex);
  }
  return 0;
}

VkResult handle_resource_commands(vk_render_state *p_vkrs, resource_queue *resource_queue)
{
  VkResult res;

  for (int i = 0; i < resource_queue->count; ++i) {
    resource_command *resource_cmd = &resource_queue->commands[i];

    switch (resource_cmd->type) {
    case RESOURCE_COMMAND_LOAD_TEXTURE: {
      res = mvk_load_texture_from_file(p_vkrs, resource_cmd->load_texture.path,
                                       (mcr_texture_image **)resource_cmd->p_resource);

      // TODO manage the freeing or strdup of the path char string
      VK_CHECK(res, "load_texture_from_file");

    } break;
    case RESOURCE_COMMAND_CREATE_TEXTURE: {
      res = mvk_create_empty_render_target(
          p_vkrs, resource_cmd->create_texture.width, resource_cmd->create_texture.height,
          resource_cmd->create_texture.image_usage, (mcr_texture_image **)resource_cmd->p_resource);
      VK_CHECK(res, "create_empty_render_target");

    } break;
    case RESOURCE_COMMAND_LOAD_FONT: {
      // printf("hrc-resource_cmd->font.height:%f\n", resource_cmd->font.height);
      res = mvk_load_font(p_vkrs, resource_cmd->font.path, resource_cmd->font.height,
                          (mcr_font_resource **)resource_cmd->p_resource);
      VK_CHECK(res, "load_font");

    } break;
    case RESOURCE_COMMAND_CREATE_RENDER_PROGRAM: {
      // printf("hrc-resource_cmd->font.height:%f\n", resource_cmd->font.height);
      res = mvk_create_render_program(p_vkrs, &resource_cmd->create_render_program.create_info,
                                      (mcr_render_program **)resource_cmd->p_resource);
      VK_CHECK(res, "load_font");

    } break;
    case RESOURCE_COMMAND_LOAD_VERTEX_BUFFER: {
      res = mvk_load_vertex_data_buffer(p_vkrs, resource_cmd->load_mesh.p_data, resource_cmd->load_mesh.data_count,
                                        resource_cmd->load_mesh.release_original_data_on_copy,
                                        (mcr_vertex_buffer **)resource_cmd->p_resource);
      VK_CHECK(res, "mvk_load_vertex_data_buffer");

    } break;
    case RESOURCE_COMMAND_LOAD_INDEX_BUFFER: {
      res = mvk_load_index_buffer(p_vkrs, resource_cmd->load_indices.p_data, resource_cmd->load_indices.data_count,
                                  resource_cmd->load_indices.release_original_data_on_copy,
                                  (mcr_index_buffer **)resource_cmd->p_resource);
      VK_CHECK(res, "mvk_load_index_buffer");

    } break;
    // case RESOURCE_COMMAND_RECREATE_SWAPCHAIN: {
    //   res = mvk_recreate_swapchain(p_vkrs);
    //   VK_CHECK(res, "mvk_recreate_swapchain");

    //   printf("recreated swapchain\n");
    // } break;
    default:
      printf("COULD NOT HANDLE RESOURCE COMMAND TYPE=%i\n", resource_cmd->type);
      continue;
    }
  }

  return res;
}

int set_viewport_cmd(VkCommandBuffer command_buffer, float x, float y, float width, float height)
{
  // printf("set_viewport_cmd: %f %f %f %f\n", x, y, width, height);
  VkViewport viewport;
  viewport.x = x;
  viewport.y = y;
  viewport.width = width;
  viewport.height = height;
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  vkCmdSetViewport(command_buffer, 0, 1, &viewport);

  return 0;
}

int set_scissor_cmd(VkCommandBuffer command_buffer, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  // printf("set_scissor_cmd: %i %i %u %u\n", x, y, width, height);
  VkRect2D scissor;
  scissor.offset.x = x;
  scissor.offset.y = y;
  scissor.extent.width = width;
  scissor.extent.height = height;
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  return 0;
}

VkResult mrt_execute_render_buffer_queued_copies(vk_render_state *p_vkrs, mvk_dynamic_buffer_block *buffer_block)
{
  if (!buffer_block) {
    // Set the block to the latest
    if (p_vkrs->render_data_buffer.dynamic_buffers.activated == 0 ||
        p_vkrs->render_data_buffer.dynamic_buffers.size == 0) {
      return VK_SUCCESS;
    }

    buffer_block =
        p_vkrs->render_data_buffer.dynamic_buffers.blocks[p_vkrs->render_data_buffer.dynamic_buffers.activated - 1];
  }

  uint8_t *pData;
  VkResult res = vkMapMemory(p_vkrs->device, buffer_block->memory, 0, buffer_block->utilized, 0, (void **)&pData);
  VK_CHECK(res, "vkMapMemory");

  // Buffer Copies
  for (int k = 0; k < p_vkrs->render_data_buffer.queued_copies_count; ++k) {
    // printf("BufferCopy: %p (+%lu) bytes:%lu\n", pData + p_vkrs->render_data_buffer.queued_copies[k].dest_offset,
    //        p_vkrs->render_data_buffer.queued_copies[k].dest_offset,
    //        p_vkrs->render_data_buffer.queued_copies[k].size_in_bytes);
    memcpy(pData + p_vkrs->render_data_buffer.queued_copies[k].dest_offset,
           p_vkrs->render_data_buffer.queued_copies[k].p_source,
           p_vkrs->render_data_buffer.queued_copies[k].size_in_bytes);
  }

  vkUnmapMemory(p_vkrs->device, buffer_block->memory);
  return res;
}

VkResult mrt_write_desc_and_queue_render_data(vk_render_state *p_vkrs, unsigned long size_in_bytes, void *p_src,
                                              VkDescriptorBufferInfo *descriptor_buffer_info)
{
  VkResult res = VK_SUCCESS;

  mvk_dynamic_buffer_block *buffer_block;
  {
    if (p_vkrs->render_data_buffer.dynamic_buffers.activated == 0) {
      buffer_block = NULL;
    }
    else {
      buffer_block =
          p_vkrs->render_data_buffer.dynamic_buffers.blocks[p_vkrs->render_data_buffer.dynamic_buffers.activated - 1];
    }

    if (!buffer_block || buffer_block->utilized + size_in_bytes >= buffer_block->allocated_size) {
      // Increment
      if (buffer_block) {
        mrt_execute_render_buffer_queued_copies(p_vkrs, buffer_block);
      }
      ++p_vkrs->render_data_buffer.dynamic_buffers.activated;
      p_vkrs->render_data_buffer.queued_copies_count = 0;

      if (p_vkrs->render_data_buffer.dynamic_buffers.activated > p_vkrs->render_data_buffer.dynamic_buffers.size) {
        // Dynamically create another
        res = mvk_allocate_dynamic_render_data_memory(p_vkrs, size_in_bytes);
        VK_CHECK(res, "mvk_allocate_dynamic_render_data_memory");
      }

      buffer_block =
          p_vkrs->render_data_buffer.dynamic_buffers.blocks[p_vkrs->render_data_buffer.dynamic_buffers.activated - 1];
      buffer_block->utilized = 0;
    }
  }

  if (p_vkrs->render_data_buffer.queued_copies_count + 1 >= p_vkrs->render_data_buffer.queued_copies_alloc) {
    printf("Requested a queued copy when one isn't allocated for\n");
    return VK_ERROR_OUT_OF_HOST_MEMORY;
  }

  // TODO -- refactor this
  descriptor_buffer_info->buffer = buffer_block->buffer;
  descriptor_buffer_info->offset = buffer_block->utilized;
  descriptor_buffer_info->range = size_in_bytes;

  // printf("wdqrd- : %lu\n", p_vkrs->render_data_buffer.frame_utilized_amount);

  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].p_source = p_src;
  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count].dest_offset =
      buffer_block->utilized;
  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count++].size_in_bytes =
      size_in_bytes;
  buffer_block->utilized += ((size_in_bytes / p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment) + 1UL) *
                            p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment;
  // printf("minUniformBufferOffsetAlignment:%lu\n", p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment);

  // printf("wdqrd- : %lu\n", p_vkrs->render_data_buffer.frame_utilized_amount);
  return res;
}

VkResult mrt_render_colored_quad(vk_render_state *p_vkrs, VkCommandBuffer command_buffer,
                                 image_render_details *image_render, element_render_command *cmd,
                                 mrt_sequence_copy_buffer *copy_buffer)
{
  // Bounds check
  if (cmd->colored_rect_info.width == 0 || cmd->x >= image_render->image_width || cmd->colored_rect_info.height == 0 ||
      cmd->y >= image_render->image_height)
    return VK_SUCCESS;

  VkResult res;
  // printf("mrt_rcq-0 %u %u\n", cmd->colored_rect_info.width, cmd->colored_rect_info.height);

  // Setup viewport and clip
  set_viewport_cmd(command_buffer, 0.f, 0.f, (float)image_render->image_width, (float)image_render->image_height);
  set_scissor_cmd(command_buffer, (int32_t)(cmd->x > 0 ? cmd->x : 0), (int32_t)(cmd->y > 0 ? cmd->y : 0),
                  (uint32_t)cmd->colored_rect_info.width, (uint32_t)cmd->colored_rect_info.height);

  // printf("%u %u %u %u\n", cmd->x, cmd->y, cmd->colored_rect_info.width, cmd->colored_rect_info.height);

  // Vertex Uniform Buffer Object
  vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer->data[copy_buffer->index];
  copy_buffer->index += sizeof(vert_data_scale_offset);
  MCassert(copy_buffer->index < MRT_SEQUENCE_COPY_BUFFER_SIZE, "BUFFER TOO SMALL");

  float scale_multiplier =
      1.f / (float)(image_render->image_width < image_render->image_height ? image_render->image_width
                                                                           : image_render->image_height);
  vert_ubo_data->scale.x = 2.f * cmd->colored_rect_info.width * scale_multiplier;
  vert_ubo_data->scale.y = 2.f * cmd->colored_rect_info.height * scale_multiplier;
  vert_ubo_data->offset.x = -1.0f + 2.0f * (float)cmd->x / (float)(image_render->image_width) +
                            1.0f * (float)cmd->colored_rect_info.width / (float)(image_render->image_width);
  vert_ubo_data->offset.y = -1.0f + 2.0f * (float)cmd->y / (float)(image_render->image_height) +
                            1.0f * (float)cmd->colored_rect_info.height / (float)(image_render->image_height);

  // printf("mrt_rcq-1\n");
  // Fragment Data
  render_color *frag_ubo_data = (render_color *)&copy_buffer->data[copy_buffer->index];
  copy_buffer->index += sizeof(render_color);
  MCassert(copy_buffer->index < MRT_SEQUENCE_COPY_BUFFER_SIZE, "BUFFER TOO SMALL");

  memcpy(frag_ubo_data, &cmd->colored_rect_info.color, sizeof(render_color));

  // Allocate the descriptor set from the pool.
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.pNext = NULL;
  setAllocInfo.descriptorPool = p_vkrs->descriptor_pool;
  setAllocInfo.descriptorSetCount = 1;
  setAllocInfo.pSetLayouts = &p_vkrs->tint_prog.descriptor_layout;

  unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
  res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
  VK_CHECK(res, "vkAllocateDescriptorSets");

  VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
  p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

  // Queue Buffer and Descriptor Writes
  const unsigned int MAX_DESC_SET_WRITES = 3;
  VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
  VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
  int buffer_info_index = 0;
  int write_index = 0;

  // printf("mrt_rcq-2\n");
  VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
  res = mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(render_color), frag_ubo_data, frag_ubo_info);
  VK_CHECK(res, "mrt_write_desc_and_queue_render_data");

  VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
  res = mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);
  VK_CHECK(res, "mrt_write_desc_and_queue_render_data");

  // Global Vertex Shader Uniform Buffer
  VkWriteDescriptorSet *write = &writes[write_index++];
  write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write->pNext = NULL;
  write->dstSet = desc_set;
  write->descriptorCount = 1;
  write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write->pBufferInfo = &copy_buffer->vpc_desc_buffer_info;
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

  // printf("mrt_rcq-3\n");
  vkUpdateDescriptorSets(p_vkrs->device, write_index, writes, 0, NULL);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->tint_prog.pipeline_layout, 0, 1,
                          &desc_set, 0, NULL);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->tint_prog.pipeline);

  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_vkrs->shape_vertices.buf, offsets);

  vkCmdDraw(command_buffer, 2 * 3, 1, 0, 0);

  return res;
}

VkResult mrt_render_textured_quad(vk_render_state *p_vkrs, VkCommandBuffer command_buffer,
                                  image_render_details *image_render, element_render_command *cmd,
                                  mrt_sequence_copy_buffer *copy_buffer)
{
  VkResult res;

  if (!cmd->textured_rect_info.texture) {
    // Ignore call
    printf("Ignored mrt_render_textured_quad call because texture was NULL\n");
    return VK_SUCCESS;
  }

  // Find the texture
  mcr_texture_image *image_sampler = cmd->textured_rect_info.texture;

  // Vertex Uniform Buffer Object
  vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer->data[copy_buffer->index];
  copy_buffer->index += sizeof(vert_data_scale_offset);
  MCassert(copy_buffer->index < MRT_SEQUENCE_COPY_BUFFER_SIZE, "BUFFER TOO SMALL");

  float scale_multiplier =
      1.f / (float)(image_render->image_width < image_render->image_height ? image_render->image_width
                                                                           : image_render->image_height);
  vert_ubo_data->scale.x = 2.f * (float)cmd->textured_rect_info.width * scale_multiplier;
  vert_ubo_data->scale.y = 2.f * (float)cmd->textured_rect_info.height * scale_multiplier;
  vert_ubo_data->offset.x = -1.0f + 2.0f * (float)cmd->x / (float)(image_render->image_width) +
                            1.0f * (float)cmd->textured_rect_info.width / (float)(image_render->image_width);
  vert_ubo_data->offset.y = -1.0f + 2.0f * (float)cmd->y / (float)(image_render->image_height) +
                            1.0f * (float)cmd->textured_rect_info.height / (float)(image_render->image_height);

  // printf("x:%u y:%u tri.width:%u tri.height:%u seq.width:%u seq.height:%u\n", cmd->x, cmd->y,
  //        cmd->textured_rect_info.width, cmd->textured_rect_info.height, image_render->image_width,
  //        image_render->image_height);

  // Setup viewport and clip
  set_viewport_cmd(command_buffer, 0.f, 0.f, (float)image_render->image_width, (float)image_render->image_height);
  set_scissor_cmd(command_buffer, (int32_t)cmd->x, (int32_t)cmd->y, (uint32_t)cmd->textured_rect_info.width,
                  (uint32_t)cmd->textured_rect_info.height);

  // Queue Buffer Write
  const unsigned int MAX_DESC_SET_WRITES = 8;
  VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
  VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
  int buffer_info_index = 0;
  int write_index = 0;

  // Allocate the descriptor set from the pool.
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.pNext = NULL;
  // Use the pool we created earlier ( the one dedicated to this frame )
  setAllocInfo.descriptorPool = p_vkrs->descriptor_pool;
  // We only need to allocate one
  setAllocInfo.descriptorSetCount = 1;
  setAllocInfo.pSetLayouts = &p_vkrs->texture_prog.descriptor_layout;

  unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
  res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
  VK_CHECK(res, "vkAllocateDescriptorSets");

  VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
  p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

  VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
  res = mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);
  VK_CHECK(res, "mrt_write_desc_and_queue_render_data");

  // Global Vertex Shader Uniform Buffer
  VkWriteDescriptorSet *write = &writes[write_index++];
  write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write->pNext = NULL;
  write->dstSet = desc_set;
  write->descriptorCount = 1;
  write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  write->pBufferInfo = &copy_buffer->vpc_desc_buffer_info;
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
  image_sampler_info.imageView = image_sampler->view;
  image_sampler_info.sampler = image_sampler->sampler;

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

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->texture_prog.pipeline_layout, 0, 1,
                          &desc_set, 0, NULL);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->texture_prog.pipeline);

  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_vkrs->textured_shape_vertices.buf, offsets);

  vkCmdDraw(command_buffer, 2 * 3, 1, 0, 0);

  return res;
}

VkResult mrt_render_text(vk_render_state *p_vkrs, VkCommandBuffer command_buffer, image_render_details *image_render,
                         element_render_command *cmd, mrt_sequence_copy_buffer *copy_buffer)
{
  VkResult res;

  if (!cmd->print_text.text || cmd->print_text.text[0] == '\0') {
    return VK_SUCCESS;
  }

  // printf("mrt-0\n");
  // Get the font image
  mcr_font_resource *font = cmd->print_text.font;
  if (!font) {
    printf("Could not render text because font was NULL\n");
    return VK_ERROR_UNKNOWN;
  }

  mcr_texture_image *font_image = font->texture;

  float align_x = cmd->x;
  float align_y = cmd->y;

  // printf("mrt-1 %s\n", cmd->print_text.text);
  int text_length = strlen(cmd->print_text.text);
  for (int c = 0; c < text_length; ++c) {

    // if (align_x > font_image->width) {
    //   break;
    // }

    char letter = cmd->print_text.text[c];
    if (letter < 32 || letter > 127) {
      printf("TODO character not supported.\n");
      return VK_SUCCESS;
    }
    // printf("printing character '%c' %i\n", letter, (int)letter);

    // Source texture bounds
    stbtt_aligned_quad q;

    // printf("garbagein: %i %i %f %f %i\n", (int)font_image->width, (int)font_image->height, align_x, align_y,
    //        letter - 32);

    stbtt_GetBakedQuad(font->char_data, (int)font_image->width, (int)font_image->height, letter - 32, &align_x,
                       &align_y, &q,
                       1); // 1=opengl & d3d10+,0=d3d9

    // TODO -- figure out why this won't work
    // if (letter == ' ') {
    //   // No need to print empty space (yet...)
    //   continue;
    // }

    // printf("mrt-2\n");
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

    q.y0 += font->draw_vertical_offset;
    q.y1 += font->draw_vertical_offset;

    // Determine the clip
    VkRect2D clip = {(int32_t)(q.x0 < 0 ? 0 : q.x0), (int32_t)(q.y0 < 0 ? 0 : q.y0), (uint32_t)width, (uint32_t)height};
    {
      mc_rect *cc = &cmd->print_text.clip;
      // printf("clbb:%i %i %u %u || cc:%i %i %u %u\n", clip.offset.x, clip.offset.y, clip.extent.width,
      //        clip.extent.height, cc->offset.x, cc->offset.y, cc->extents.width, cc->extents.height);
      if (cc->extents.width && cc->extents.height) {
        if (clip.offset.x < cc->offset.x)
          clip.offset.x = cc->offset.x;
        if (clip.offset.y < cc->offset.y)
          clip.offset.y = cc->offset.y;
        if (clip.offset.x + clip.extent.width > cc->offset.x + cc->extents.width) {
          if (cc->offset.x + cc->extents.width <= clip.offset.x)
            clip.extent.width = 0U;
          else
            clip.extent.width = cc->offset.x + cc->extents.width - clip.offset.x;
        }
        if (clip.offset.y + clip.extent.height > cc->offset.y + cc->extents.height) {
          if (cc->offset.y + cc->extents.height <= clip.offset.y)
            clip.extent.height = 0U;
          else
            clip.extent.height = cc->offset.y + cc->extents.height - clip.offset.y;
        }
      }

      if (clip.extent.width <= 0U || clip.extent.height <= 0U)
        continue;
      if (clip.offset.x >= image_render->image_width || clip.offset.y >= image_render->image_height)
        continue;

      // if (clip.extent.width > 4444) {
      //   printf("clip:%i %i %u %u || cc:%i %i %u %u\n", clip.offset.x, clip.offset.y, clip.extent.width,
      //          clip.extent.height, cc->offset.x, cc->offset.y, cc->extents.width, cc->extents.height);
      // }
    }
    // printf("baked_quad: s0=%.2f s1==%.2f t0=%.2f t1=%.2f x0=%.2f x1=%.2f y0=%.2f y1=%.2f xoff=%.2f yoff=%.2f\n",
    // q.s0,
    //        q.s1, q.t0, q.t1, q.x0, q.x1, q.y0, q.y1, font->char_data->xoff, font->char_data->yoff);
    // printf("align_x=%.2f align_y=%.2f\n", align_x, align_y);
    // printf("font->draw_vertical_offset=%.2f\n", font->draw_vertical_offset);

    // Vertex Uniform Buffer Object  -- TODO do checking on copy_buffer->index and data array size
    vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer->data[copy_buffer->index];
    copy_buffer->index += sizeof(vert_data_scale_offset);
    MCassert(copy_buffer->index < MRT_SEQUENCE_COPY_BUFFER_SIZE, "BUFFER TOO SMALL");
    // printf("mrt-2a\n");

    float scale_multiplier =
        1.f / (float)(image_render->image_width < image_render->image_height ? image_render->image_width
                                                                             : image_render->image_height);
    vert_ubo_data->scale.x = 2.f * width * scale_multiplier;
    vert_ubo_data->scale.y = 2.f * height * scale_multiplier;
    vert_ubo_data->offset.x = -1.0f + 2.0f * q.x0 / (float)(image_render->image_width) +
                              1.0f * (float)width / (float)(image_render->image_width);
    vert_ubo_data->offset.y = -1.0f + 2.0f * q.y0 / (float)(image_render->image_height) +
                              1.0f * (float)height / (float)(image_render->image_height);

    // printf("mrt-3\n");
    // Fragment Data
    frag_ubo_tint_texcoordbounds *frag_ubo_data =
        (frag_ubo_tint_texcoordbounds *)&copy_buffer->data[copy_buffer->index];
    copy_buffer->index += sizeof(frag_ubo_tint_texcoordbounds);
    MCassert(copy_buffer->index < MRT_SEQUENCE_COPY_BUFFER_SIZE, "BUFFER TOO SMALL");

    memcpy(&frag_ubo_data->tint, &cmd->print_text.color, sizeof(float) * 4);
    frag_ubo_data->tex_coord_bounds.s0 = q.s0;
    frag_ubo_data->tex_coord_bounds.s1 = q.s1;
    frag_ubo_data->tex_coord_bounds.t0 = q.t0;
    frag_ubo_data->tex_coord_bounds.t1 = q.t1;

    // Setup viewport and clip
    // printf("image_render : %f, %f\n",  (float)image_render->image_width,  (float)image_render->image_height);
    set_viewport_cmd(command_buffer, 0.f, 0.f, (float)image_render->image_width, (float)image_render->image_height);
    vkCmdSetScissor(command_buffer, 0, 1, &clip);
    // set_scissor_cmd(command_buffer, clip.offset.x, clip.offset.y, clip.extent.width, clip.extent.height);

    // printf("mrt-4\n");
    // Allocate the descriptor set from the pool.
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.pNext = NULL;
    setAllocInfo.descriptorPool = p_vkrs->descriptor_pool;
    setAllocInfo.descriptorSetCount = 1;
    setAllocInfo.pSetLayouts = &p_vkrs->font_prog.descriptor_layout;

    unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
    // printf("cmd->text:'%s' descriptor_set_index=%u\n", cmd->print_text.text, descriptor_set_index);
    res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
    VK_CHECK(res, "vkAllocateDescriptorSets");

    VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
    p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

    // Queue Buffer and Descriptor Writes
    const unsigned int MAX_DESC_SET_WRITES = 4;
    VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
    VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
    int buffer_info_index = 0;
    int write_index = 0;

    // printf("mrt-5\n");
    VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
    res = mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);
    VK_CHECK(res, "mrt_write_desc_and_queue_render_data");

    VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
    res = mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(frag_ubo_tint_texcoordbounds), frag_ubo_data,
                                               frag_ubo_info);
    VK_CHECK(res, "mrt_write_desc_and_queue_render_data");

    // printf("mrt-6\n");
    // Global Vertex Shader Uniform Buffer
    VkWriteDescriptorSet *write = &writes[write_index++];
    write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write->pNext = NULL;
    write->dstSet = desc_set;
    write->descriptorCount = 1;
    write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write->pBufferInfo = &copy_buffer->vpc_desc_buffer_info;
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

    // printf("mrt-7\n");
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

    // printf("mrt-8\n");
    vkUpdateDescriptorSets(p_vkrs->device, write_index, writes, 0, NULL);

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->font_prog.pipeline_layout, 0, 1,
                            &desc_set, 0, NULL);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->font_prog.pipeline);

    const VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_vkrs->textured_shape_vertices.buf, offsets);

    vkCmdDraw(command_buffer, 2 * 3, 1, 0, 0);
    // printf("mrt-9\n");
  }

  free(cmd->print_text.text);

  return res;
}

VkResult mrt_render_render_program(vk_render_state *p_vkrs, VkCommandBuffer command_buffer,
                                   image_render_details *image_render, element_render_command *cmd,
                                   mrt_sequence_copy_buffer *copy_buffer)
{
  VkResult res;

  // Get the resources
  mcr_render_program *render_prog = cmd->render_program.program;
  mcr_render_program_data *program_data = cmd->render_program.data;

  // // Setup viewport and clip
  set_viewport_cmd(command_buffer, 0.f, 0.f, (float)image_render->image_width, (float)image_render->image_height);
  set_scissor_cmd(command_buffer, (int32_t)0, (int32_t)0, (uint32_t)image_render->image_width,
                  (uint32_t)image_render->image_height);

  // Allocate the descriptor set from the pool.
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.pNext = NULL;
  setAllocInfo.descriptorPool = p_vkrs->descriptor_pool;
  setAllocInfo.descriptorSetCount = 1;
  setAllocInfo.pSetLayouts = &render_prog->descriptor_layout;

  unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
  res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
  VK_CHECK(res, "vkAllocateDescriptorSets");

  VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
  p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

  // Queue Buffer and Descriptor Writes
  VkWriteDescriptorSet writes[render_prog->layout_binding_count];
  VkDescriptorBufferInfo buffer_infos[render_prog->layout_binding_count];
  VkDescriptorImageInfo image_sampler_infos[render_prog->layout_binding_count];
  int buffer_info_index = 0;
  int write_index = 0;

  for (int i = 0; i < render_prog->layout_binding_count; ++i) {
    switch (render_prog->layout_bindings[i].type) {
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
      mcr_texture_image *image_sampler = (mcr_texture_image *)cmd->render_program.data->input_buffers[i];
      VkDescriptorImageInfo *image_sampler_info = &image_sampler_infos[i];
      image_sampler_info->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      image_sampler_info->imageView = image_sampler->view;
      image_sampler_info->sampler = image_sampler->sampler;

      // Element Fragment Shader Combined Image Sampler
      VkWriteDescriptorSet *write = &writes[i];
      write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write->pNext = NULL;
      write->dstSet = desc_set;
      write->descriptorCount = 1;
      write->descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
      write->pImageInfo = image_sampler_info;
      write->dstArrayElement = 0;
      write->dstBinding = i;
    } break;
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
      VkDescriptorBufferInfo *buffer_info = &buffer_infos[i];
      res = mrt_write_desc_and_queue_render_data(p_vkrs, render_prog->layout_bindings[i].size_in_bytes,
                                                 program_data->input_buffers[i], buffer_info);
      VK_CHECK(res, "mrt_write_desc_and_queue_render_data");

      VkWriteDescriptorSet *write = &writes[i];
      write->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      write->pNext = NULL;
      write->dstSet = desc_set;
      write->descriptorCount = 1;
      write->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      write->pBufferInfo = buffer_info;
      write->dstArrayElement = 0;
      write->dstBinding = i;
    } break;
    default: {
      MCerror(1061, "TODO : %i", render_prog->layout_bindings[i].type);
    }
    }
  }

  // // printf("mrt_rcq-3\n");
  vkUpdateDescriptorSets(p_vkrs->device, render_prog->layout_binding_count, writes, 0, NULL);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_prog->pipeline_layout, 0, 1,
                          &desc_set, 0, NULL);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, render_prog->pipeline);

  mcr_index_buffer *indices = cmd->render_program.data->indices;
  vkCmdBindIndexBuffer(command_buffer, indices->buf, 0, VK_INDEX_TYPE_UINT32);

  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &cmd->render_program.data->vertices->buf, offsets);
  // vkCmdDraw(command_buffer, 3 * 2 * 6, 1, 0, 0);
  vkCmdDrawIndexed(command_buffer, indices->count, 1, 0, 0, 0);

  return res;
}

VkResult render_sequence(vk_render_state *p_vkrs, VkCommandBuffer command_buffer, image_render_details *image_render)
{
  // Descriptor Writes
  VkResult res;

  coloured_rect_draw_data rect_draws[128];
  int rect_draws_index = 0;

  // TODO -- this still isn't very seamly no size checking on entry into the buffer, and keeps recreating it?
  // Donno...
  mrt_sequence_copy_buffer copy_buffer;
  copy_buffer.index = 0;

  p_vkrs->render_data_buffer.dynamic_buffers.activated = 0;
  p_vkrs->render_data_buffer.queued_copies_count = 0U;

  mat4 vpc;
  {
    // TODO -- this isn't used for 3D?
    // Construct the Vulkan View/Projection/Clip for the render target image
    mat4 view;
    mat4 proj;
    mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};

    glm_lookat((vec3){0, 0, -10}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)&view);
    glm_ortho_default((float)image_render->image_width / image_render->image_height, (vec4 *)&proj);
    glm_mat4_mul((vec4 *)&proj, (vec4 *)&view, (vec4 *)&vpc);
    glm_mat4_mul((vec4 *)&clip, (vec4 *)&vpc, (vec4 *)&vpc);

    res = mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(mat4), &vpc, &copy_buffer.vpc_desc_buffer_info);
    VK_CHECK(res, "mrt_write_desc_and_queue_render_data");
    // printf("(&copy_buffer->vpc_desc_buffer_info)[0].offset=%lu\n",
    // (&copy_buffer.vpc_desc_buffer_info)[0].offset);
  }
  // printf("image_render : %u, %u\n", image_render->image_width, image_render->image_height);

  for (int j = 0; j < image_render->command_count; ++j) {

    element_render_command *cmd = &image_render->commands[j];
    switch (cmd->type) {
    case RENDER_COMMAND_COLORED_QUAD: {
      // printf("RENDER_COMMAND_COLORED_QUAD\n");
      res = mrt_render_colored_quad(p_vkrs, command_buffer, image_render, cmd, &copy_buffer);
      VK_CHECK(res, "mrt_render_colored_rectangle");
    } break;

    case RENDER_COMMAND_TEXTURED_QUAD: {
      // printf("RENDER_COMMAND_TEXTURED_QUAD\n");
      res = mrt_render_textured_quad(p_vkrs, command_buffer, image_render, cmd, &copy_buffer);
      VK_CHECK(res, "mrt_render_textured_quad");
    } break;

    case RENDER_COMMAND_PRINT_TEXT: {
      // printf("RENDER_COMMAND_PRINT_TEXT\n");
      res = mrt_render_text(p_vkrs, command_buffer, image_render, cmd, &copy_buffer);
      VK_CHECK(res, "mrt_render_text");
    } break;

    case RENDER_COMMAND_PROGRAM: {
      // printf("RENDER_COMMAND_PROGRAM\n");
      res = mrt_render_render_program(p_vkrs, command_buffer, image_render, cmd, &copy_buffer);
      VK_CHECK(res, "mrt_render_render_program");
    } break;

    default:
      printf("COULD NOT HANDLE RENDER COMMAND TYPE=%i\n", cmd->type);
      continue;
    }
  }

  if (copy_buffer.index >= MRT_SEQUENCE_COPY_BUFFER_SIZE) {
    printf("ERROR Copy Buffer Allocation is insufficient!!\n");
    return VK_ERROR_UNKNOWN;
  }
  if (p_vkrs->render_data_buffer.queued_copies_count) {
    mrt_execute_render_buffer_queued_copies(p_vkrs, NULL);
  }

  return VK_SUCCESS;
}

VkResult render_through_queue(vk_render_state *p_vkrs, image_render_list *image_render_queue)
{
  VkResult res;

  for (int i = 0; i < image_render_queue->count; ++i) {

    image_render_details *image_render = image_render_queue->items[i];

    // if (image_render->command_count < 1) {
    //   return VK_ERROR_UNKNOWN;
    // }

    // printf("image_render: rt:%i cmd_count:%i\n", image_render->render_target, image_render->command_count);
    bool skip_render_presents = false;

    switch (image_render->render_target) {
    case NODE_RENDER_TARGET_PRESENT: {
      // printf("NODE_RENDER_TARGET_PRESENT\n");
      if (skip_render_presents)
        break;

      VkSemaphore imageAcquiredSemaphore;
      VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
      imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      imageAcquiredSemaphoreCreateInfo.pNext = NULL;
      imageAcquiredSemaphoreCreateInfo.flags = 0;

      res = vkCreateSemaphore(p_vkrs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
      VK_CHECK(res, "vkCreateSemaphore");

      // Get the index of the next available swapchain image:
      res = vkAcquireNextImageKHR(p_vkrs->device, p_vkrs->swap_chain.instance, UINT64_MAX, imageAcquiredSemaphore,
                                  VK_NULL_HANDLE, &p_vkrs->swap_chain.current_index);
      // TODO: Deal with the VK_SUBOPTIMAL_KHR and VK_ERROR_OUT_OF_DATE_KHR
      // return codes
      VK_CHECK(res, "vkAcquireNextImageKHR");

      res = vkResetDescriptorPool(p_vkrs->device, p_vkrs->descriptor_pool, 0);
      VK_CHECK(res, "vkResetDescriptorPool");
      p_vkrs->descriptor_sets_count = 0U;

      // Begin Command Buffer Recording
      VkCommandBuffer command_buffer = p_vkrs->swap_chain.command_buffers[p_vkrs->swap_chain.current_index];
      res = vkResetCommandBuffer(command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
      VK_CHECK(res, "vkResetCommandBuffer");

      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
      beginInfo.pInheritanceInfo = NULL;                             // Optional
      vkBeginCommandBuffer(command_buffer, &beginInfo);

      VkClearValue clear_values[1];
      clear_values[0].color.float32[0] = image_render->clear_color.r;
      clear_values[0].color.float32[1] = image_render->clear_color.g;
      clear_values[0].color.float32[2] = image_render->clear_color.b;
      clear_values[0].color.float32[3] = image_render->clear_color.a;
      // clear_values[1].depthStencil.depth = 1.0f;
      // clear_values[1].depthStencil.stencil = 0;

      VkRenderPassBeginInfo rp_begin;
      rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      rp_begin.pNext = NULL;
      rp_begin.renderPass = p_vkrs->present_render_pass;
      rp_begin.framebuffer = p_vkrs->swap_chain.framebuffers[p_vkrs->swap_chain.current_index];
      rp_begin.renderArea.offset.x = 0;
      rp_begin.renderArea.offset.y = 0;
      // rp_begin.renderArea.extent.width = image_render->image_width;
      // rp_begin.renderArea.extent.height = image_render->image_height;
      rp_begin.renderArea.extent.width = min(p_vkrs->xcb_winfo->width, image_render->image_width);
      rp_begin.renderArea.extent.height = min(p_vkrs->xcb_winfo->height, image_render->image_height);
      rp_begin.clearValueCount = 1;
      rp_begin.pClearValues = clear_values;

      vkCmdBeginRenderPass(command_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

      res = render_sequence(p_vkrs, command_buffer, image_render);
      VK_CHECK(res, "render_sequence");

      vkCmdEndRenderPass(command_buffer);
      res = vkEndCommandBuffer(command_buffer);
      VK_CHECK(res, "vkEndCommandBuffer");

      VkFenceCreateInfo fenceInfo;
      VkFence drawFence;
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.pNext = NULL;
      fenceInfo.flags = 0;
      vkCreateFence(p_vkrs->device, &fenceInfo, NULL, &drawFence);
      VK_CHECK(res, "vkCreateFence");

      VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkSubmitInfo submit_info[1] = {};
      submit_info[0].pNext = NULL;
      submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info[0].waitSemaphoreCount = 1;
      submit_info[0].pWaitSemaphores = &imageAcquiredSemaphore;
      submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
      submit_info[0].commandBufferCount = 1;
      const VkCommandBuffer cmd_bufs[] = {command_buffer};
      submit_info[0].pCommandBuffers = cmd_bufs;
      submit_info[0].signalSemaphoreCount = 0;
      submit_info[0].pSignalSemaphores = NULL;

      /* Queue the command buffer for execution */
      res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
      VK_CHECK(res, "vkQueueSubmit");

      /* Now present the image in the window */
      VkPresentInfoKHR present;
      present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      present.pNext = NULL;
      present.swapchainCount = 1;
      present.pSwapchains = &p_vkrs->swap_chain.instance;
      present.pImageIndices = &p_vkrs->swap_chain.current_index;
      present.pWaitSemaphores = NULL;
      present.waitSemaphoreCount = 0;
      present.pResults = NULL;

      /* Make sure command buffer is finished before presenting */
      // TODO work on better synchronization (fences with each image in the swap chain?);
      do {
        res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
      } while (res == VK_TIMEOUT);
      VK_CHECK(res, "vkWaitForFences");
      res = vkResetFences(p_vkrs->device, 1, &drawFence);
      VK_CHECK(res, "vkResetFences");

      res = vkQueuePresentKHR(p_vkrs->present_queue, &present);
      if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        // For now assume this is because the xcb window has resized while this render queue was being processed.
        // swapchain->surface_specs will then be set accordingly requiring no more action from us.

        // Maybe return (but then any headless image renders will be lost to the wind and they might not get redone on
        // swapchain recreation), so just skip any further render-present calls. There shouldn't be any, but...
        skip_render_presents = true;
      }
      else {
        VK_CHECK(res, "vkQueuePresentKHR");
      }

      vkDestroySemaphore(p_vkrs->device, imageAcquiredSemaphore, NULL);
      vkDestroyFence(p_vkrs->device, drawFence, NULL);

      ++p_vkrs->presentation_updates;
    } break;
    case NODE_RENDER_TARGET_IMAGE: {
      // printf("NODE_RENDER_TARGET_IMAGE\n");
      // Adjust render command absolute coordinates for relative render target
      for (int j = 0; j < image_render->command_count; ++j) {
        element_render_command *cmd = &image_render->commands[j];

        // printf("node_render_target_image: %p %i %u %u : %u %u\n", cmd, cmd->type, cmd->x,
        //        image_render->data.target_image.screen_offset_coordinates.x, cmd->y,
        //        image_render->data.target_image.screen_offset_coordinates.y);
        cmd->x -= image_render->data.target_image.screen_offset_coordinates.x;
        cmd->y -= image_render->data.target_image.screen_offset_coordinates.y;
      }

      // Obtain the target image
      mcr_texture_image *target_image = image_render->data.target_image.image;

      if (!target_image->framebuffer) {
        MCerror(1079, "target %u not specified as render target", target_image->resource_uid);
      }

      VkRenderPass offscreen_render_pass;
      switch (target_image->sampler_usage) {
      case MVK_IMAGE_USAGE_RENDER_TARGET_2D: {
        offscreen_render_pass = p_vkrs->offscreen_render_pass_2d;
      } break;
      case MVK_IMAGE_USAGE_RENDER_TARGET_3D: {
        offscreen_render_pass = p_vkrs->offscreen_render_pass_3d;
      } break;
      default: {
        printf("NODE_RENDER_TARGET_IMAGE: target_image does not support render target usage\n");
        return (VkResult)1098;
      }
      }

      // TODO -- free only used descriptor sets in favor of safe vulkan multi-threading?
      res = vkResetDescriptorPool(p_vkrs->device, p_vkrs->descriptor_pool, 0);
      VK_CHECK(res, "vkResetDescriptorPool");
      p_vkrs->descriptor_sets_count = 0U;

      // Obtain the command buffer
      res = vkResetCommandBuffer(p_vkrs->headless.command_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
      VK_CHECK(res, "vkResetCommandBuffer");

      // Begin Command Buffer Recording
      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Optional
      beginInfo.pInheritanceInfo = NULL;                             // Optional
      vkBeginCommandBuffer(p_vkrs->headless.command_buffer, &beginInfo);

      VkClearValue clear_values[2];
      clear_values[0].color.float32[0] = image_render->clear_color.r;
      clear_values[0].color.float32[1] = image_render->clear_color.g;
      clear_values[0].color.float32[2] = image_render->clear_color.b;
      clear_values[0].color.float32[3] = image_render->clear_color.a;
      clear_values[1].depthStencil.depth = 1.0f;
      clear_values[1].depthStencil.stencil = 0;

      VkRenderPassBeginInfo rp_begin;
      rp_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      rp_begin.pNext = NULL;
      rp_begin.renderPass = offscreen_render_pass;
      rp_begin.framebuffer = target_image->framebuffer;
      rp_begin.renderArea.offset.x = 0;
      rp_begin.renderArea.offset.y = 0;
      rp_begin.renderArea.extent.width = min(p_vkrs->xcb_winfo->width, target_image->width);
      rp_begin.renderArea.extent.height = min(p_vkrs->xcb_winfo->height, target_image->height);
      rp_begin.clearValueCount = 2;
      rp_begin.pClearValues = clear_values;

      vkCmdBeginRenderPass(p_vkrs->headless.command_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

      res = render_sequence(p_vkrs, p_vkrs->headless.command_buffer, image_render);
      VK_CHECK(res, "render_sequence");

      vkCmdEndRenderPass(p_vkrs->headless.command_buffer);
      res = vkEndCommandBuffer(p_vkrs->headless.command_buffer);
      VK_CHECK(res, "vkEndCommandBuffer");

      VkFenceCreateInfo fenceInfo;
      VkFence drawFence;
      fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceInfo.pNext = NULL;
      fenceInfo.flags = 0;
      vkCreateFence(p_vkrs->device, &fenceInfo, NULL, &drawFence);
      VK_CHECK(res, "vkCreateFence");

      VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      VkSubmitInfo submit_info[1] = {};
      submit_info[0].pNext = NULL;
      submit_info[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info[0].waitSemaphoreCount = 0;
      submit_info[0].pWaitSemaphores = NULL;
      submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
      submit_info[0].commandBufferCount = 1;
      const VkCommandBuffer cmd_bufs[] = {p_vkrs->headless.command_buffer};
      submit_info[0].pCommandBuffers = cmd_bufs;
      submit_info[0].signalSemaphoreCount = 0;
      submit_info[0].pSignalSemaphores = NULL;

      /* Queue the command buffer for execution */
      res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
      VK_CHECK(res, "vkQueueSubmit");

      /* Make sure command buffer is finished before leaving */
      do {
        res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
      } while (res == VK_TIMEOUT);
      VK_CHECK(res, "vkWaitForFences");
      res = vkResetFences(p_vkrs->device, 1, &drawFence);
      VK_CHECK(res, "vkResetFences");

      vkDestroyFence(p_vkrs->device, drawFence, NULL);
    } break;
    default:
      printf("WAS:%i\n", image_render->render_target);
      return VK_ERROR_UNKNOWN;
    }
  }

  return VK_SUCCESS;
}

VkResult mrt_process_render_queues(render_thread_info *render_thread, vk_render_state *vkrs,
                                   image_render_list *rt_render_queue)
{
  // DEBUG
  // struct timespec render_start_time;
  // clock_gettime(CLOCK_REALTIME, &render_start_time);
  // DEBUG

  // Copy render requests from transient queue to render-thread only queue
  pthread_mutex_lock(&render_thread->image_queue->mutex);
  if (!render_thread->image_queue->count) {
    pthread_mutex_unlock(&render_thread->image_queue->mutex);
    return VK_SUCCESS;
  }

  if (rt_render_queue->alloc < render_thread->image_queue->count) {
    reallocate_collection((void ***)&rt_render_queue->items, &rt_render_queue->alloc,
                          render_thread->image_queue->count + 4 + render_thread->image_queue->count / 4, 0);
  }

  memcpy(rt_render_queue->items, render_thread->image_queue->items,
         sizeof(image_render_details *) * render_thread->image_queue->count);
  rt_render_queue->count = render_thread->image_queue->count;
  render_thread->image_queue->count = 0;

  pthread_mutex_unlock(&render_thread->image_queue->mutex);

  // {
  //   // DEBUG
  //   uint cmd_count = 0;
  //   for (int r = 0; r < render_thread->image_queue->count; ++r) {
  //     cmd_count += render_thread->image_queue.items[r].command_count;
  //     if(render_thread->image_queue.items[r]->type == )
  //   }
  //   printf("Vulkan entered image_render_queue! %u sequences using %u draw-commands\n",
  //          render_thread->image_queue->count, cmd_count);
  // }

  // Queue Render
  VkResult res = render_through_queue(vkrs, rt_render_queue);
  // printf("rendered %u image requests\n", rt_render_queue->count);
  VK_CHECK(res, "render_through_queue");

  // After frame statistics
  // DEBUG
  // struct timespec render_end_time;
  // clock_gettime(CLOCK_REALTIME, &render_end_time);
  // printf("Vulkan rendered image_render_queue! %.3f ms\n",
  //        (double)(render_end_time.tv_sec - render_start_time.tv_sec) * 1000 +
  //            1e-6 * (render_end_time.tv_nsec - render_start_time.tv_nsec));
  // DEBUG

  // Return the image render request objects to the pool
  pthread_mutex_lock(&render_thread->render_request_object_pool->mutex);
  // if (render_thread->render_request_object_pool->count + rt_render_queue->count >
  //     render_thread->render_request_object_pool->alloc) {
  //   reallocate_collection((void ***)&render_thread->render_request_object_pool->items,
  //                         &render_thread->render_request_object_pool->alloc,
  //                         render_thread->render_request_object_pool->count + rt_render_queue->count + 4 +
  //                             (render_thread->render_request_object_pool->count + rt_render_queue->count) / 4,
  //                         0);
  // }

  // memcpy(render_thread->render_request_object_pool->items +
  //            render_thread->render_request_object_pool->count * sizeof(image_render_details *),
  //        rt_render_queue->items, sizeof(image_render_details *) * rt_render_queue->count);
  // render_thread->render_request_object_pool->count += rt_render_queue->count;
  // rt_render_queue->count = 0;

  for (int i = 0; i < rt_render_queue->count; ++i) {
    append_to_collection((void ***)&render_thread->render_request_object_pool->items,
                         &render_thread->render_request_object_pool->alloc,
                         &render_thread->render_request_object_pool->count, rt_render_queue->items[i]);
  }
  rt_render_queue->count = 0;

  pthread_mutex_unlock(&render_thread->render_request_object_pool->mutex);

  return VK_SUCCESS;
}

void *window_events_loop(void *state)
{
  mthread_info *thr = *(mthread_info **)((void **)state)[0];
  mxcb_window_info *winfo = (mxcb_window_info *)((void **)state)[1];
  window_input_buffer *wib = (window_input_buffer *)((void **)state)[2];

  midge_error_set_thread_name("window-events-thread");

  int wures;
  while (!thr->should_exit) {
    // Update Window
    if (mxcb_update_window(winfo, wib)) {
      puts("ERR 5267 mxcb_update_window(winfo, wib)\n");
      break;
    }

    usleep(100);
  }

  puts("Leaving Input Thread");
  thr->has_concluded = true;
  return NULL;
}

VkResult mrt_run_update_loop(render_thread_info *render_thread, vk_render_state *vkrs)
{
  // printf("mrt-rul-0\n");
  VkResult res;
  midge_app_info *global_data;
  mc_obtain_midge_app_info(&global_data);

  mthread_info *thr = render_thread->thread_info;
  render_thread->loaded_fonts = &vkrs->loaded_fonts;

  // -- Update
  // printf("mrt-rul-1\n");
  int wures = mxcb_update_window(vkrs->xcb_winfo, &render_thread->input_buffer);
  // printf("mrt-rul-2\n");
  global_data->screen.width = vkrs->swap_chain.extents.width;
  global_data->screen.height = vkrs->swap_chain.extents.height;
  printf("Vulkan Initialized!\n");

  VK_CHECK((VkResult)wures, "mxcb_update_window");
  // printf("mrt-2:good\n");
  render_thread->render_thread_initialized = true;

  image_render_list rt_render_queue;
  rt_render_queue.alloc = 0;
  rt_render_queue.count = 0;

  // Begin the input thread
  mthread_info *window_events_thr;
  void *itargs[3];
  itargs[0] = (void *)&window_events_thr;
  itargs[1] = (void *)vkrs->xcb_winfo;
  itargs[2] = (void *)&render_thread->input_buffer;
  MCcall(begin_mthread(&window_events_loop, &window_events_thr, itargs));

  // printf("mrt-2: %p\n", thr);
  // printf("mrt-2: %p\n", &winfo);
  while (!thr->should_exit && !vkrs->xcb_winfo->input_requests_exit) {
    // TODO DEBUG
    // printf("mrt-rul-3:loop\n");
    // usleep(1000);
    if (!vkrs->swap_chain.surface_specs.valid) {
      while (!vkrs->swap_chain.surface_specs.valid) {
        pthread_mutex_lock(&vkrs->swap_chain.surface_specs.mutex);
        res = mvk_recreate_swapchain(vkrs);
        VK_CHECK(res, "mvk_recreate_swapchain");
        pthread_mutex_unlock(&vkrs->swap_chain.surface_specs.mutex);
      }

      render_thread->window_surface.width = vkrs->swap_chain.extents.width;
      render_thread->window_surface.height = vkrs->swap_chain.extents.height;
      render_thread->window_surface.modified = true;
    }

    // Resource Commands
    // usleep(700000);
    // printf("render_thread->resource_queue->count=%u\n", render_thread->resource_queue->count);
    pthread_mutex_lock(&render_thread->resource_queue->mutex);
    if (render_thread->resource_queue->count) {
      // printf("Vulkan entered resources!\n");
      res = handle_resource_commands(vkrs, render_thread->resource_queue);
      VK_CHECK(res, "handle_resource_commands");
      render_thread->resource_queue->count = 0;
      // printf("Vulkan loaded resources!\n");
    }
    pthread_mutex_unlock(&render_thread->resource_queue->mutex);
    // printf("mrt-rul-4:loop\n");

    // Render Commands
    pthread_mutex_lock(&render_thread->image_queue->mutex);
    // TODO review -- should be atomic anyway?
    bool queue_not_empty = render_thread->image_queue->count;
    // printf("mrt-rul-5:loop %u\n", render_thread->image_queue->count);
    pthread_mutex_unlock(&render_thread->image_queue->mutex);
    if (queue_not_empty) {
      mrt_process_render_queues(render_thread, vkrs, &rt_render_queue);
    }

    // TODO
    usleep(100);
  }

  end_mthread(window_events_thr);
  printf("Leaving Render-Thread loop...\n--total_rendered_frames = %i\n", vkrs->presentation_updates);

  return VK_SUCCESS;
}

void _mrt_init_vk_render_state(vk_render_state *p_vkrs)
{
  // Set initial values
  p_vkrs->presentation_updates = 0;
  p_vkrs->resource_uid_counter = 300;
  p_vkrs->maximal_image_width = 2048;
  p_vkrs->maximal_image_height = 2048;

  // Surface Recreation
  pthread_mutex_init(&p_vkrs->swap_chain.surface_specs.mutex, NULL);
  p_vkrs->swap_chain.surface_specs.valid = false;
}

// TODO -- look into the use of the volatile keyword with multi-threaded-access-variables
void *midge_render_thread(void *vargp)
{
  render_thread_info *render_thread = (render_thread_info *)vargp;

  printf("~~midge_render_thread called!~~\n");
  midge_error_set_thread_name("render-thread");

  mthread_info *thr = render_thread->thread_info;
  // printf("mrt-2: %p\n", thr);

  // -- States
  mxcb_window_info winfo = {};
  // winfo.input_requests_exit = 0;

  vk_render_state vkrs = {};
  _mrt_init_vk_render_state(&vkrs);
  vkrs.xcb_winfo = &winfo;

  // vkrs.textures.allocated = 0;
  // Handle Window Resize
  struct _mrt_resize_args rza = (struct _mrt_resize_args){render_thread, &vkrs};
  winfo.window_resized_callback.delegate = &_mrt_handle_window_resize;
  winfo.window_resized_callback.state = (void *)&rza;

  // Vulkan Initialize
  VkResult res = mvk_init_vulkan(&vkrs);
  if (res) {
    printf("--ERR[%i] mvk_init_vulkan\n", res);
    render_thread->thread_info->has_concluded = 1;
    return NULL;
  }
  // printf("mrt-3\n");

  res = mvk_init_resources(&vkrs);
  if (res) {
    printf("--ERR[%i] mvk_init_resources\n", res);
    render_thread->thread_info->has_concluded = 1;
    return NULL;
  }
  // printf("mrt-4\n");

  // Update Loop
  res = mrt_run_update_loop(render_thread, &vkrs);
  if (res) {
    printf("--ERR[%i] mrt_run_update_loop\n", res);
    render_thread->thread_info->has_concluded = 1;
    return NULL;
  }
  // printf("mrt-5\n");

  // Vulkan Cleanup
  mvk_destroy_resources(&vkrs);
  mvk_destroy_vulkan(&vkrs);

  render_thread->thread_info->has_concluded = 1;
  return NULL;
}