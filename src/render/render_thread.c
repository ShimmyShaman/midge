
#include "m_threads.h"
#include "midge_common.h"
#include "render/mc_vk_utils.h"
#include <vulkan/vulkan.h>

VkResult handle_resource_commands(vk_render_state *p_vkrs, resource_queue *resource_queue)
{
  VkResult res;

  for (int i = 0; i < resource_queue->count; ++i) {
    resource_command *resource_cmd = &resource_queue->commands[i];

    switch (resource_cmd->type) {
    case RESOURCE_COMMAND_LOAD_TEXTURE: {
      res = mvk_load_texture_from_file(p_vkrs, resource_cmd->data.load_texture.path, resource_cmd->p_uid);
      VK_CHECK(res, "load_texture_from_file");

    } break;
    case RESOURCE_COMMAND_CREATE_TEXTURE: {
      res = mvk_create_empty_render_target(p_vkrs, resource_cmd->data.create_texture.width,
                                           resource_cmd->data.create_texture.height,
                                           resource_cmd->data.create_texture.use_as_render_target, resource_cmd->p_uid);
      VK_CHECK(res, "create_empty_render_target");

    } break;
    case RESOURCE_COMMAND_LOAD_FONT: {
      res = mvk_load_font(p_vkrs, resource_cmd->data.font.path, resource_cmd->data.font.height, resource_cmd->p_uid);
      VK_CHECK(res, "load_font");

    } break;

    default:
      printf("COULD NOT HANDLE RESOURCE COMMAND TYPE=%i\n", resource_cmd->type);
      continue;
    }
  }

  return res;
}

int set_viewport_cmd(VkCommandBuffer command_buffer, float x, float y, float width, float height)
{
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
  VkRect2D scissor;
  scissor.offset.x = x;
  scissor.offset.y = y;
  scissor.extent.width = width;
  scissor.extent.height = height;
  vkCmdSetScissor(command_buffer, 0, 1, &scissor);

  return 0;
}

int mrt_write_desc_and_queue_render_data(vk_render_state *p_vkrs, unsigned long size_in_bytes, void *p_src,
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
  p_vkrs->render_data_buffer.queued_copies[p_vkrs->render_data_buffer.queued_copies_count++].size_in_bytes =
      size_in_bytes;
  p_vkrs->render_data_buffer.frame_utilized_amount +=
      ((size_in_bytes / p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment) + 1UL) *
      p_vkrs->gpu_props.limits.minUniformBufferOffsetAlignment;

  return 0;
}

VkResult mrt_render_colored_quad(vk_render_state *p_vkrs, VkCommandBuffer *command_buffer, image_render_queue *sequence,
                                 element_render_command *cmd, mrt_sequence_copy_buffer *copy_buffer)
{
  // Bounds check
  if (cmd->data.colored_rect_info.width == 0 || cmd->x >= sequence->image_width ||
      cmd->data.colored_rect_info.height == 0 || cmd->y >= sequence->image_height)
    return VK_SUCCESS;

  VkResult res;

  // Setup viewport and clip
  set_viewport_cmd(p_vkrs, 0, 0, (float)sequence->image_width, (float)sequence->image_height);
  set_scissor_cmd(p_vkrs, cmd->x > 0 ? cmd->x : 0, cmd->y > 0 ? cmd->y : 0, cmd->data.colored_rect_info.width,
                  cmd->data.colored_rect_info.height);

  // Vertex Uniform Buffer Object
  vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer->data[copy_buffer->index];
  copy_buffer->index += sizeof(vert_data_scale_offset);

  float scale_multiplier =
      1.f / (float)(sequence->image_width < sequence->image_height ? sequence->image_width : sequence->image_height);
  vert_ubo_data->scale.x = 2.f * cmd->data.colored_rect_info.width * scale_multiplier;
  vert_ubo_data->scale.y = 2.f * cmd->data.colored_rect_info.height * scale_multiplier;
  vert_ubo_data->offset.x = -1.0f + 2.0f * (float)cmd->x / (float)(sequence->image_width) +
                            1.0f * (float)cmd->data.colored_rect_info.width / (float)(sequence->image_width);
  vert_ubo_data->offset.y = -1.0f + 2.0f * (float)cmd->y / (float)(sequence->image_height) +
                            1.0f * (float)cmd->data.colored_rect_info.height / (float)(sequence->image_height);

  // Fragment Data
  render_color *frag_ubo_data = (render_color *)&copy_buffer->data[copy_buffer->index];
  copy_buffer->index += sizeof(render_color);

  memcpy(frag_ubo_data, &cmd->data.colored_rect_info.color, sizeof(float) * 4);

  // Allocate the descriptor set from the pool.
  VkDescriptorSetAllocateInfo setAllocInfo = {};
  setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  setAllocInfo.pNext = NULL;
  setAllocInfo.descriptorPool = p_vkrs->descriptor_pool;
  setAllocInfo.descriptorSetCount = 1;
  setAllocInfo.pSetLayouts = p_vkrs->present_prog.descriptor_layout;

  unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
  res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
  VK_CHECK(res, "vkAllocateDescriptorSets");

  VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
  p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

  // Queue Buffer and Descriptor Writes
  const unsigned int MAX_DESC_SET_WRITES = 8;
  VkWriteDescriptorSet writes[MAX_DESC_SET_WRITES];
  VkDescriptorBufferInfo buffer_infos[MAX_DESC_SET_WRITES];
  int buffer_info_index = 0;
  int write_index = 0;

  VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
  mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(render_color), frag_ubo_data, frag_ubo_info);

  VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
  mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);

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

  vkUpdateDescriptorSets(p_vkrs->device, write_index, writes, 0, NULL);

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline_layout, 0, 1, &desc_set, 0,
                          NULL);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->pipeline);

  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_vkrs->shape_vertices.buf, offsets);

  vkCmdDraw(command_buffer, 2 * 3, 1, 0, 0);

  return res;
}

VkResult mrt_render_textured_quad(vk_render_state *p_vkrs, VkCommandBuffer *command_buffer,
                                  image_render_queue *sequence, element_render_command *cmd,
                                  mrt_sequence_copy_buffer *copy_buffer)
{
  VkResult res;

  // Vertex Uniform Buffer Object
  vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer->data[copy_buffer->index];
  copy_buffer->index += sizeof(vert_data_scale_offset);

  float scale_multiplier =
      1.f / (float)(sequence->image_width < sequence->image_height ? sequence->image_width : sequence->image_height);
  vert_ubo_data->scale.x = 2.f * (float)cmd->data.textured_rect_info.width * scale_multiplier;
  vert_ubo_data->scale.y = 2.f * (float)cmd->data.textured_rect_info.height * scale_multiplier;
  vert_ubo_data->offset.x = -1.0f + 2.0f * (float)cmd->x / (float)(sequence->image_width) +
                            1.0f * (float)cmd->data.textured_rect_info.width / (float)(sequence->image_width);
  vert_ubo_data->offset.y = -1.0f + 2.0f * (float)cmd->y / (float)(sequence->image_height) +
                            1.0f * (float)cmd->data.textured_rect_info.height / (float)(sequence->image_height);

  // printf("x:%u y:%u tri.width:%u tri.height:%u seq.width:%u seq.height:%u\n", cmd->x, cmd->y,
  //        cmd->data.textured_rect_info.width, cmd->data.textured_rect_info.height, sequence->image_width,
  //        sequence->image_height);

  // Setup viewport and clip
  set_viewport_cmd(p_vkrs, 0, 0, (float)sequence->image_width, (float)sequence->image_height);
  set_scissor_cmd(p_vkrs, cmd->x, cmd->y, cmd->data.textured_rect_info.width, cmd->data.textured_rect_info.height);

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
  setAllocInfo.descriptorPool = p_vkrs->desc_pool;
  // We only need to allocate one
  setAllocInfo.descriptorSetCount = 1;
  setAllocInfo.pSetLayouts = &p_vkrs->texture_prog.desc_layout;

  unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
  res = vkAllocateDescriptorSets(p_vkrs->device, &setAllocInfo, &p_vkrs->descriptor_sets[descriptor_set_index]);
  VK_CHECK(res, "vkAllocateDescriptorSets");

  VkDescriptorSet desc_set = p_vkrs->descriptor_sets[descriptor_set_index];
  p_vkrs->descriptor_sets_count += setAllocInfo.descriptorSetCount;

  VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
  mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);

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

  VkDescriptorImageInfo image_sampler_info = {};
  image_sampler_info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  image_sampler_info.imageView =
      p_vkrs->textures.samples[cmd->data.textured_rect_info.texture_uid - RESOURCE_UID_BEGIN].view;
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

  vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->texture_prog.pipeline_layout, 0, 1,
                          &desc_set, 0, NULL);

  vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->texture_prog.pipeline);

  const VkDeviceSize offsets[1] = {0};
  vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_vkrs->textured_shape_vertices.buf, offsets);

  vkCmdDraw(command_buffer, 2 * 3, 1, 0, 0);

  return res;
}

VkResult mrt_render_text(vk_render_state *p_vkrs, VkCommandBuffer *command_buffer, image_render_queue *sequence,
                         element_render_command *cmd, mrt_sequence_copy_buffer *copy_buffer)
{
  VkResult res;

  if (!cmd->data.print_text.text) {
    break;
  }
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

    // if (align_x > font_image->width) {
    //   break;
    // }

    char letter = cmd->data.print_text.text[c];
    if (letter < 32 || letter > 127) {
      printf("TODO character not supported.\n");
      return VK_SUCCESS;
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

    // printf("baked_quad: s0=%.2f s1==%.2f t0=%.2f t1=%.2f x0=%.2f x1=%.2f y0=%.2f y1=%.2f xoff=%.2f yoff=%.2f\n",
    // q.s0, q.s1,
    //        q.t0, q.t1, q.x0, q.x1, q.y0, q.y1, font->char_data->xoff, font->char_data->yoff);
    // printf("align_x=%.2f align_y=%.2f\n", align_x, align_y);

    // Vertex Uniform Buffer Object
    vert_data_scale_offset *vert_ubo_data = (vert_data_scale_offset *)&copy_buffer->data[copy_buffer->index];
    copy_buffer->index += sizeof(vert_data_scale_offset);

    float scale_multiplier =
        1.f / (float)(sequence->image_width < sequence->image_height ? sequence->image_width : sequence->image_height);
    vert_ubo_data->scale.x = 2.f * width * scale_multiplier;
    vert_ubo_data->scale.y = 2.f * height * scale_multiplier;
    vert_ubo_data->offset.x = -1.0f + 2.0f * (float)q.x0 / (float)(sequence->image_width) +
                              1.0f * (float)width / (float)(sequence->image_width);
    vert_ubo_data->offset.y = -1.0f + 2.0f * (float)q.y0 / (float)(sequence->image_height) +
                              1.0f * (float)height / (float)(sequence->image_height);

    // Fragment Data
    frag_ubo_tint_texcoordbounds *frag_ubo_data =
        (frag_ubo_tint_texcoordbounds *)&copy_buffer->data[copy_buffer->index];
    copy_buffer->index += sizeof(frag_ubo_tint_texcoordbounds);

    memcpy(&frag_ubo_data->tint, &cmd->data.print_text.color, sizeof(float) * 4);
    frag_ubo_data->tex_coord_bounds.s0 = q.s0;
    frag_ubo_data->tex_coord_bounds.s1 = q.s1;
    frag_ubo_data->tex_coord_bounds.t0 = q.t0;
    frag_ubo_data->tex_coord_bounds.t1 = q.t1;

    // Setup viewport and clip
    set_viewport_cmd(p_vkrs, 0, 0, (float)sequence->image_width, (float)sequence->image_height);
    set_scissor_cmd(p_vkrs, q.x0 < 0 ? 0 : q.x0, q.y0 < 0 ? 0 : q.y0, width, height);

    // Allocate the descriptor set from the pool.
    VkDescriptorSetAllocateInfo setAllocInfo = {};
    setAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocInfo.pNext = NULL;
    setAllocInfo.descriptorPool = p_vkrs->desc_pool;
    setAllocInfo.descriptorSetCount = 1;
    setAllocInfo.pSetLayouts = &p_vkrs->font_prog.desc_layout;

    unsigned int descriptor_set_index = p_vkrs->descriptor_sets_count;
    // printf("cmd->text:'%s' descriptor_set_index=%u\n", cmd->data.print_text.text, descriptor_set_index);
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

    VkDescriptorBufferInfo *vert_ubo_info = &buffer_infos[buffer_info_index++];
    mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(vert_data_scale_offset), vert_ubo_data, vert_ubo_info);

    VkDescriptorBufferInfo *frag_ubo_info = &buffer_infos[buffer_info_index++];
    mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(frag_ubo_tint_texcoordbounds), frag_ubo_data, frag_ubo_info);

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

    vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->font_prog.pipeline_layout, 0, 1,
                            &desc_set, 0, NULL);

    vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, p_vkrs->font_prog.pipeline);

    const VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(command_buffer, 0, 1, &p_vkrs->textured_shape_vertices.buf, offsets);

    vkCmdDraw(command_buffer, 2 * 3, 1, 0, 0);
  }

  free(cmd->data.print_text.text);

  return res;
}

VkResult render_sequence(vk_render_state *p_vkrs, VkCommandBuffer *command_buffer, image_render_queue *sequence)
{
  // Descriptor Writes
  VkResult res;

  coloured_rect_draw_data rect_draws[128];
  int rect_draws_index = 0;

  // TODO -- this still isn't very seamly no size checking on entry into the buffer, and keeps recreating it?
  // Donno...
  mrt_sequence_copy_buffer copy_buffer;
  copy_buffer->index = 0;

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

    mrt_write_desc_and_queue_render_data(p_vkrs, sizeof(mat4), &vpc, &vpc_desc_buffer_info);
  }

  for (int j = 0; j < sequence->command_count; ++j) {

    element_render_command *cmd = &sequence->commands[j];
    switch (cmd->type) {
    case RENDER_COMMAND_COLORED_QUAD: {
      res = mrt_render_colored_quad(p_vkrs, command_buffer, sequence, cmd, &copy_buffer->data);
      VK_CHECK(res, "mrt_render_colored_rectangle");
    } break;

    case RENDER_COMMAND_TEXTURED_QUAD: {
      res = mrt_render_textured_quad(p_vkrs, command_buffer, sequence, cmd, &copy_buffer->data);
      VK_CHECK(res, "mrt_render_textured_quad");
    } break;

    case RENDER_COMMAND_PRINT_TEXT: {
      res = mrt_render_text(p_vkrs, command_buffer, sequence, cmd, &copy_buffer->data);
      VK_CHECK(res, "mrt_render_text");
    } break;

    default:
      printf("COULD NOT HANDLE RENDER COMMAND TYPE=%i\n", cmd->type);
      continue;
    }
  }

  if (copy_buffer->index >= COPY_BUFFER_SIZE) {
    printf("ERROR Copy Buffer Allocation is insufficient!!\n");
    return VK_ERROR_UNKNOWN;
  }
  if (p_vkrs->render_data_buffer.queued_copies_count) {
    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->render_data_buffer.memory, 0,
                      p_vkrs->render_data_buffer.frame_utilized_amount, 0, (void **)&pData);
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

    vkUnmapMemory(p_vkrs->device, p_vkrs->render_data_buffer.memory);
  }

  return VK_SUCCESS;
}

VkResult render_through_queue(vk_render_state *p_vkrs, render_queue *render_queue)
{
  VkResult res;

  for (int i = 0; i < render_queue->count; ++i) {

    image_render_queue *sequence = &render_queue->image_renders[i];

    // if (sequence->command_count < 1) {
    //   return VK_ERROR_UNKNOWN;
    // }

    // printf("sequence: rt:%i cmd_count:%i\n", sequence->render_target, sequence->command_count);

    switch (sequence->render_target) {
    case NODE_RENDER_TARGET_PRESENT: {
      VkSemaphore imageAcquiredSemaphore;
      VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
      imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
      imageAcquiredSemaphoreCreateInfo.pNext = NULL;
      imageAcquiredSemaphoreCreateInfo.flags = 0;

      res = vkCreateSemaphore(p_vkrs->device, &imageAcquiredSemaphoreCreateInfo, NULL, &imageAcquiredSemaphore);
      VK_CHECK(res, "vkCreateSemaphore");

      // Get the index of the next available swapchain image:
      res = vkAcquireNextImageKHR(p_vkrs->device, p_vkrs->swap_chain, UINT64_MAX, imageAcquiredSemaphore,
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

      VkClearValue clear_values[2];
      clear_values[0].color.float32[0] = sequence->clear_color.r;
      clear_values[0].color.float32[1] = sequence->clear_color.g;
      clear_values[0].color.float32[2] = sequence->clear_color.b;
      clear_values[0].color.float32[3] = sequence->clear_color.a;
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

      vkCmdBeginRenderPass(command_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

      render_sequence(p_vkrs, sequence);

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
      present.pSwapchains = &p_vkrs->swap_chain;
      present.pImageIndices = &p_vkrs->current_buffer;
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
      VK_CHECK(res, "vkQueuePresentKHR");

      vkDestroySemaphore(p_vkrs->device, imageAcquiredSemaphore, NULL);
      vkDestroyFence(p_vkrs->device, drawFence, NULL);
    } break;
    case NODE_RENDER_TARGET_IMAGE: {
      // Obtain the target image
      sampled_image *target_image =
          &p_vkrs->textures.samples[sequence->data.target_image.image_uid - RESOURCE_UID_BEGIN];

      if (!target_image->framebuffer) {
        // Create?
        VkFramebufferCreateInfo framebuffer_create_info = {};
        framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_create_info.pNext = NULL;
        framebuffer_create_info.renderPass = p_vkrs->offscreen_render_pass;
        framebuffer_create_info.attachmentCount = 1;
        framebuffer_create_info.pAttachments = &target_image->view;
        framebuffer_create_info.width = target_image->width;
        framebuffer_create_info.height = target_image->height;
        framebuffer_create_info.layers = 1;

        res = vkCreateFramebuffer(p_vkrs->device, &framebuffer_create_info, NULL, &target_image->framebuffer);
        VK_CHECK(res, "vkCreateFramebuffer");
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

      VkClearValue clear_values[1];
      clear_values[0].color.float32[0] = sequence->clear_color.r;
      clear_values[0].color.float32[1] = sequence->clear_color.g;
      clear_values[0].color.float32[2] = sequence->clear_color.b;
      clear_values[0].color.float32[3] = sequence->clear_color.a;

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

      vkCmdBeginRenderPass(p_vkrs->headless.command_buffer, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

      res = render_sequence(p_vkrs, p_vkrs->headless.command_buffer, sequence);
      VK_CHECK(res, "vkEndCommandBuffer");

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
      return VK_ERROR_UNKNOWN;
    }
  }

  return VK_SUCCESS;
}

VkResult mrt_run_update_loop(render_thread_info *render_thread, vk_render_state *vkrs)
{
  mthread_info *thr = render_thread->thread_info;

  // -- Update
  mxcb_update_window(&vkrs->xcb_winfo, &render_thread->input_buffer);
  render_thread->render_thread_initialized = true;
  // printf("mrt-2: %p\n", thr);
  // printf("mrt-2: %p\n", &winfo);
  uint frame_updates = 0;
  while (!thr->should_exit && !vkrs->xcb_winfo.shouldExit) {
    // Resource Commands
    pthread_mutex_lock(&render_thread->resource_queue.mutex);
    if (render_thread->resource_queue.count) {
      // printf("Vulkan entered resources!\n");
      handle_resource_commands(&vkrs, &render_thread->resource_queue);
      render_thread->resource_queue.count = 0;
      printf("Vulkan loaded resources!\n");
    }
    pthread_mutex_unlock(&render_thread->resource_queue.mutex);

    // Render Commands
    pthread_mutex_lock(&render_thread->render_queue.mutex);
    if (render_thread->render_queue.count) {
      // {
      //   // DEBUG
      //   uint cmd_count = 0;
      //   for (int r = 0; r < render_thread->render_queue.count; ++r) {
      //     cmd_count += render_thread->render_queue.image_renders[r].command_count;
      //   }
      //   printf("Vulkan entered render_queue! %u sequences using %u draw-calls\n",
      //   render_thread->render_queue.count, cmd_count);
      // }
      render_through_queue(&vkrs, &render_thread->render_queue);
      render_thread->render_queue.count = 0;

      // printf("Vulkan rendered render_queue!\n");
      ++frame_updates;
    }
    pthread_mutex_unlock(&render_thread->render_queue.mutex);

    mxcb_update_window(&vkrs->xcb_winfo, &render_thread->input_buffer);
  }
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

  res = mvk_init_resources(&vkrs);

  // Update Loop
  res = mrt_run_update_loop(render_thread, &vkrs);

  // Vulkan Cleanup
  res = mvk_cleanup_resources(&vkrs);
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