
#include "cglm/cglm.h"
#include "m_threads.h"
#include "midge_common.h"
#include "render/mc_vulkan.h"
#include <vulkan/vulkan.h>

#include "stb_truetype.h"

#define XY(X, Y) X, Y
#define UV(U, V) U, V
#define XYZW(X, Y, Z) X, Y, Z, 1.f
#define RGB(R, G, B) R, G, B
#define RGBA(R, G, B) R, G, B, 1.f
#define WHITE_RGBA 1.f, 1.f, 1.f, 1.f
#define WHITE_RGB 1.f, 1.f, 1.f

VkResult mvk_init_shape_vertices(vk_render_state *p_vkrs)
{
  const float g_vb_shape_data[] = {// Rectangle
                                   XY(-0.5f, -0.5f), XY(-0.5f, 0.5f), XY(0.5f, -0.5f),
                                   XY(-0.5f, 0.5f),  XY(0.5f, 0.5f),  XY(0.5f, -0.5f)};
  //  XYZW(-0.5f, -0.5f, 0), WHITE_RGBA, XYZW(-0.5f, 0.5f, 0), WHITE_RGBA,
  //  XYZW(0.5f, -0.5f, 0),  WHITE_RGBA, XYZW(-0.5f, 0.5f, 0), WHITE_RGBA,
  //  XYZW(0.5f, 0.5f, 0),   WHITE_RGBA, XYZW(0.5f, -0.5f, 0), WHITE_RGBA};
  const float g_vb_textured_shape_2D_data[] = {// Rectangle
                                               XY(-0.5f, -0.5f), UV(0.f, 0.f), XY(-0.5f, 0.5f), UV(0.f, 1.f),
                                               XY(0.5f, -0.5f),  UV(1.f, 0.f), XY(-0.5f, 0.5f), UV(0.f, 1.f),
                                               XY(0.5f, 0.5f),   UV(1.f, 1.f), XY(0.5f, -0.5f), UV(1.f, 0.f)};

  VkResult res;
  {
    // Shape Colored Vertices Data
    const int data_size_in_bytes = sizeof(g_vb_shape_data);

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.size = data_size_in_bytes;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->shape_vertices.buf);
    VK_CHECK(res, "vkCreateBuffer");

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->shape_vertices.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    bool pass = mvk_get_properties_memory_type_index(
        p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &alloc_info.memoryTypeIndex);
    VK_ASSERT(pass, "No mappable, coherent memory");

    res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->shape_vertices.mem));
    VK_CHECK(res, "vkAllocateMemory");
    p_vkrs->shape_vertices.buffer_info.range = mem_reqs.size;
    p_vkrs->shape_vertices.buffer_info.offset = 0;

    uint8_t *p_mapped_mem;
    res = vkMapMemory(p_vkrs->device, p_vkrs->shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&p_mapped_mem);
    VK_CHECK(res, "vkMapMemory");

    memcpy(p_mapped_mem, g_vb_shape_data, data_size_in_bytes);

    vkUnmapMemory(p_vkrs->device, p_vkrs->shape_vertices.mem);

    res = vkBindBufferMemory(p_vkrs->device, p_vkrs->shape_vertices.buf, p_vkrs->shape_vertices.mem, 0);
    VK_CHECK(res, "vkBindBufferMemory");

    // p_vkrs->shape_vertices.vi_desc = p_vkrs->pos_color_vertex_input_description;
  }
  {
    // Shaped Texture Data
    const int data_size_in_bytes = sizeof(g_vb_textured_shape_2D_data);

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.size = data_size_in_bytes;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->textured_shape_vertices.buf);
    VK_CHECK(res, "vkCreateBuffer");

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->textured_shape_vertices.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    bool pass = mvk_get_properties_memory_type_index(
        p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &alloc_info.memoryTypeIndex);
    VK_ASSERT(pass, "No mappable, coherent memory");

    res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->textured_shape_vertices.mem));
    VK_CHECK(res, "vkAllocateMemory");
    p_vkrs->textured_shape_vertices.buffer_info.range = mem_reqs.size;
    p_vkrs->textured_shape_vertices.buffer_info.offset = 0;

    uint8_t *p_mapped_mem;
    res = vkMapMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&p_mapped_mem);
    VK_CHECK(res, "vkMapMemory");

    memcpy(p_mapped_mem, g_vb_textured_shape_2D_data, data_size_in_bytes);

    vkUnmapMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem);

    res =
        vkBindBufferMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.buf, p_vkrs->textured_shape_vertices.mem, 0);
    VK_CHECK(res, "vkBindBufferMemory");
  }
  // {
  //   vec3 mesh_data[] = {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f},
  //                       {0.5f, -0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, -0.5f},  {0.5f, 0.5f, 0.5f}};

  //   // Cube vertices TEMP DEBUG
  //   const int data_size_in_bytes = sizeof(mesh_data);

  //   VkBufferCreateInfo buf_info = {};
  //   buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //   buf_info.pNext = NULL;
  //   buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  //   buf_info.size = data_size_in_bytes;
  //   buf_info.queueFamilyIndexCount = 0;
  //   buf_info.pQueueFamilyIndices = NULL;
  //   buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  //   buf_info.flags = 0;
  //   res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->cube_shape_vertices.buf);
  //   VK_CHECK(res, "vkCreateBuffer");

  //   VkMemoryRequirements mem_reqs;
  //   vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->cube_shape_vertices.buf, &mem_reqs);

  //   VkMemoryAllocateInfo alloc_info = {};
  //   alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //   alloc_info.pNext = NULL;
  //   alloc_info.memoryTypeIndex = 0;

  //   alloc_info.allocationSize = mem_reqs.size;
  //   bool pass = mvk_get_properties_memory_type_index(
  //       p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //       &alloc_info.memoryTypeIndex);
  //   VK_ASSERT(pass, "No mappable, coherent memory");

  //   res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->cube_shape_vertices.mem));
  //   VK_CHECK(res, "vkAllocateMemory");
  //   p_vkrs->cube_shape_vertices.buffer_info.range = mem_reqs.size;
  //   p_vkrs->cube_shape_vertices.buffer_info.offset = 0;

  //   uint8_t *p_mapped_mem;
  //   res = vkMapMemory(p_vkrs->device, p_vkrs->cube_shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&p_mapped_mem);
  //   VK_CHECK(res, "vkMapMemory");

  //   memcpy(p_mapped_mem, mesh_data, data_size_in_bytes);

  //   vkUnmapMemory(p_vkrs->device, p_vkrs->cube_shape_vertices.mem);

  //   res = vkBindBufferMemory(p_vkrs->device, p_vkrs->cube_shape_vertices.buf, p_vkrs->cube_shape_vertices.mem, 0);
  //   VK_CHECK(res, "vkBindBufferMemory");
  // }
  // {
  //   unsigned int indices[] = {
  //       0, 1, 2, 2, 3, 1, 4, 5, 6, 6, 7, 5, 0, 1, 4, 4, 5, 1, 2, 3, 6, 6, 7, 3, 0, 2, 4, 4, 6, 2, 1, 3, 5, 5, 7, 3,
  //   };

  //   // Cube vertices TEMP DEBUG
  //   const int data_size_in_bytes = sizeof(indices);

  //   VkBufferCreateInfo buf_info = {};
  //   buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  //   buf_info.pNext = NULL;
  //   buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  //   buf_info.size = data_size_in_bytes;
  //   buf_info.queueFamilyIndexCount = 0;
  //   buf_info.pQueueFamilyIndices = NULL;
  //   buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  //   buf_info.flags = 0;
  //   res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->cube_shape_indices.buf);
  //   VK_CHECK(res, "vkCreateBuffer");

  //   VkMemoryRequirements mem_reqs;
  //   vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->cube_shape_indices.buf, &mem_reqs);

  //   VkMemoryAllocateInfo alloc_info = {};
  //   alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  //   alloc_info.pNext = NULL;
  //   alloc_info.memoryTypeIndex = 0;

  //   alloc_info.allocationSize = mem_reqs.size;
  //   bool pass = mvk_get_properties_memory_type_index(
  //       p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //       &alloc_info.memoryTypeIndex);
  //   VK_ASSERT(pass, "No mappable, coherent memory");

  //   res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->cube_shape_indices.mem));
  //   VK_CHECK(res, "vkAllocateMemory");
  //   p_vkrs->cube_shape_indices.buffer_info.range = mem_reqs.size;
  //   p_vkrs->cube_shape_indices.buffer_info.offset = 0;

  //   uint8_t *p_mapped_mem;
  //   res = vkMapMemory(p_vkrs->device, p_vkrs->cube_shape_indices.mem, 0, mem_reqs.size, 0, (void **)&p_mapped_mem);
  //   VK_CHECK(res, "vkMapMemory");

  //   memcpy(p_mapped_mem, indices, data_size_in_bytes);

  //   vkUnmapMemory(p_vkrs->device, p_vkrs->cube_shape_indices.mem);

  //   res = vkBindBufferMemory(p_vkrs->device, p_vkrs->cube_shape_indices.buf, p_vkrs->cube_shape_indices.mem, 0);
  //   VK_CHECK(res, "vkBindBufferMemory");
  // }

  return res;
}

VkResult mvk_init_resources(vk_render_state *p_vkrs)
{
  VkResult res;

  res = mvk_init_shape_vertices(p_vkrs);
  VK_CHECK(res, "mvk_init_shape_vertices");

  return res;
}

void mvk_destroy_sampled_image(vk_render_state *p_vkrs, texture_image *texture_image)
{
  vkDestroySampler(p_vkrs->device, texture_image->sampler, NULL);
  vkDestroyImageView(p_vkrs->device, texture_image->view, NULL);
  vkDestroyImage(p_vkrs->device, texture_image->image, NULL);
  vkFreeMemory(p_vkrs->device, texture_image->memory, NULL);
  if (texture_image->framebuffer)
    vkDestroyFramebuffer(p_vkrs->device, texture_image->framebuffer, NULL);
}

void mvk_destroy_resources(vk_render_state *p_vkrs)
{
  for (int i = 0; i < p_vkrs->textures.count; ++i) {
    mvk_destroy_sampled_image(p_vkrs, p_vkrs->textures.items[i]);
  }
  free(p_vkrs->textures.items);
  for (int i = 0; i < p_vkrs->loaded_vertex_data.count; ++i) {
    vkDestroyBuffer(p_vkrs->device, p_vkrs->loaded_vertex_data.items[i]->buf, NULL);
    vkFreeMemory(p_vkrs->device, p_vkrs->loaded_vertex_data.items[i]->mem, NULL);
  }
  free(p_vkrs->loaded_vertex_data.items);
  for (int i = 0; i < p_vkrs->loaded_index_data.count; ++i) {
    vkDestroyBuffer(p_vkrs->device, p_vkrs->loaded_index_data.items[i]->buf, NULL);
    vkFreeMemory(p_vkrs->device, p_vkrs->loaded_index_data.items[i]->mem, NULL);
  }
  free(p_vkrs->loaded_index_data.items);

  vkDestroyBuffer(p_vkrs->device, p_vkrs->shape_vertices.buf, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->shape_vertices.mem, NULL);

  vkDestroyBuffer(p_vkrs->device, p_vkrs->textured_shape_vertices.buf, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem, NULL);
}

VkResult mvk_beginSingleTimeCommands(vk_render_state *p_vkrs, VkCommandBuffer *command_buffer)
{
  VkResult res;

  VkCommandBufferAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = p_vkrs->command_pool;
  allocInfo.commandBufferCount = 1;

  res = vkAllocateCommandBuffers(p_vkrs->device, &allocInfo, command_buffer);
  VK_CHECK(res, "vkAllocateCommandBuffers");

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

  res = vkBeginCommandBuffer(*command_buffer, &beginInfo);
  VK_CHECK(res, "vkBeginCommandBuffer");

  return res;
}

VkResult mvk_transitionImageLayout(vk_render_state *p_vkrs, VkImage image, VkFormat format, VkImageLayout oldLayout,
                                   VkImageLayout newLayout)
{
  VkResult res;

  VkCommandBuffer commandBuffer;
  res = mvk_beginSingleTimeCommands(p_vkrs, &commandBuffer);
  VK_CHECK(res, "mvk_beginSingleTimeCommands");

  VkImageMemoryBarrier barrier = {};
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

  vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, NULL, 0, NULL, 1, &barrier);

  // endSingleTimeCommands(p_vkrs, commandBuffer);
  {
    res = vkEndCommandBuffer(commandBuffer);
    VK_CHECK(res, "vkEndCommandBuffer");

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    res = vkQueueSubmit(p_vkrs->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    VK_CHECK(res, "vkQueueSubmit");
    res = vkQueueWaitIdle(p_vkrs->graphics_queue);
    VK_CHECK(res, "vkQueueWaitIdle");

    vkFreeCommandBuffers(p_vkrs->device, p_vkrs->command_pool, 1, &commandBuffer);
  }
  return res;
}

VkResult mvk_copyBufferToImage(vk_render_state *p_vkrs, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
  VkResult res;

  VkCommandBuffer commandBuffer;
  res = mvk_beginSingleTimeCommands(p_vkrs, &commandBuffer);
  VK_CHECK(res, "mvk_beginSingleTimeCommands");

  VkBufferImageCopy region = {};
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
    res = vkEndCommandBuffer(commandBuffer);
    VK_CHECK(res, "vkEndCommandBuffer");

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    res = vkQueueSubmit(p_vkrs->graphics_queue, 1, &submitInfo, VK_NULL_HANDLE);
    VK_CHECK(res, "vkQueueSubmit");
    res = vkQueueWaitIdle(p_vkrs->graphics_queue);
    VK_CHECK(res, "vkQueueWaitIdle");

    vkFreeCommandBuffers(p_vkrs->device, p_vkrs->command_pool, 1, &commandBuffer);
  }

  return res;
}

VkResult mvk_load_image_sampler(vk_render_state *p_vkrs, const int texWidth, const int texHeight, const int texChannels,
                                mvk_image_sampler_usage image_usage, const unsigned char *const pixels,
                                texture_image **out_image)
{
  VkResult res;

  texture_image *image_sampler = (texture_image *)malloc(sizeof(texture_image));
  VK_ASSERT(image_sampler, "malloc error 5407");

  image_sampler->resource_uid = p_vkrs->resource_uid_counter++;
  image_sampler->sampler_usage = image_usage;
  image_sampler->width = texWidth;
  image_sampler->height = texHeight;
  image_sampler->size = texWidth * texHeight * 4; // TODO

  // Copy to buffer
  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  VkBufferCreateInfo bufferInfo = {};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = image_sampler->size;
  bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  res = vkCreateBuffer(p_vkrs->device, &bufferInfo, NULL, &stagingBuffer);
  VK_CHECK(res, "vkCreateBuffer");

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(p_vkrs->device, stagingBuffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  bool pass = mvk_get_properties_memory_type_index(
      p_vkrs, memRequirements.memoryTypeBits,
      VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &allocInfo.memoryTypeIndex);
  VK_ASSERT(pass, "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &allocInfo, NULL, &stagingBufferMemory);
  VK_CHECK(res, "vkAllocateMemory");

  res = vkBindBufferMemory(p_vkrs->device, stagingBuffer, stagingBufferMemory, 0);
  VK_CHECK(res, "vkBindBufferMemory");

  void *data;
  res = vkMapMemory(p_vkrs->device, stagingBufferMemory, 0, image_sampler->size, 0, &data);
  VK_CHECK(res, "vkMapMemory");
  memcpy(data, pixels, (size_t)image_sampler->size);
  vkUnmapMemory(p_vkrs->device, stagingBufferMemory);

  // Create Image
  VkImageUsageFlags vk_image_usage_flags;
  switch (image_usage) {
  case MVK_IMAGE_USAGE_READ_ONLY: {
    vk_image_usage_flags = 0;
    image_sampler->format = VK_IMAGE_FORMAT;
  } break;
  case MVK_IMAGE_USAGE_RENDER_TARGET_2D:
  case MVK_IMAGE_USAGE_RENDER_TARGET_3D: {
    vk_image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_sampler->format = p_vkrs->format;
    vk_image_usage_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_sampler->format = p_vkrs->format;
  } break;
  default:
    MCerror(536, "Unsupported image usage:%i", image_usage);
  }

  // Image
  VkImageCreateInfo imageInfo = {};
  imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageInfo.imageType = VK_IMAGE_TYPE_2D;
  imageInfo.extent.width = (uint32_t)texWidth;
  imageInfo.extent.height = (uint32_t)texHeight;
  imageInfo.extent.depth = 1;
  imageInfo.mipLevels = 1;
  imageInfo.arrayLayers = 1;
  imageInfo.format = image_sampler->format;
  imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | vk_image_usage_flags;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  res = vkCreateImage(p_vkrs->device, &imageInfo, NULL, &image_sampler->image);
  VK_CHECK(res, "vkCreateImage");

  // Memory
  vkGetImageMemoryRequirements(p_vkrs->device, image_sampler->image, &memRequirements);

  allocInfo = {};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  pass = mvk_get_properties_memory_type_index(p_vkrs, memRequirements.memoryTypeBits,
                                              VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &allocInfo.memoryTypeIndex);
  VK_ASSERT(pass, "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &allocInfo, NULL, &image_sampler->memory);
  VK_CHECK(res, "vkAllocateMemory");

  res = vkBindImageMemory(p_vkrs->device, image_sampler->image, image_sampler->memory, 0);
  VK_CHECK(res, "vkBindImageMemory");

  res = mvk_transitionImageLayout(p_vkrs, image_sampler->image, image_sampler->format, VK_IMAGE_LAYOUT_UNDEFINED,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
  VK_CHECK(res, "mvk_transitionImageLayout");
  mvk_copyBufferToImage(p_vkrs, stagingBuffer, image_sampler->image, (uint32_t)texWidth, (uint32_t)texHeight);
  VK_CHECK(res, "mvk_copyBufferToImage");
  res = mvk_transitionImageLayout(p_vkrs, image_sampler->image, image_sampler->format,
                                  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
  VK_CHECK(res, "mvk_transitionImageLayout");

  // Destroy staging resources
  vkDestroyBuffer(p_vkrs->device, stagingBuffer, NULL);
  vkFreeMemory(p_vkrs->device, stagingBufferMemory, NULL);

  // Image View
  VkImageViewCreateInfo viewInfo = {};
  viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  viewInfo.image = image_sampler->image;
  viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
  viewInfo.format = image_sampler->format;
  viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  viewInfo.subresourceRange.baseMipLevel = 0;
  viewInfo.subresourceRange.levelCount = 1;
  viewInfo.subresourceRange.baseArrayLayer = 0;
  viewInfo.subresourceRange.layerCount = 1;

  res = vkCreateImageView(p_vkrs->device, &viewInfo, NULL, &image_sampler->view);
  VK_CHECK(res, "vkCreateImageView");

  switch (image_usage) {
  case MVK_IMAGE_USAGE_READ_ONLY: {
    // printf("MVK_IMAGE_USAGE_READ_ONLY\n");
    image_sampler->framebuffer = NULL;
  } break;
  case MVK_IMAGE_USAGE_RENDER_TARGET_2D: {
    // printf("MVK_IMAGE_USAGE_RENDER_TARGET_2D\n");
    // Create Framebuffer
    VkImageView attachments[1] = {image_sampler->view};

    VkFramebufferCreateInfo framebuffer_create_info = {};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.pNext = NULL;
    framebuffer_create_info.renderPass = p_vkrs->offscreen_render_pass_2d;
    framebuffer_create_info.attachmentCount = 1;
    framebuffer_create_info.pAttachments = attachments;
    framebuffer_create_info.width = texWidth;
    framebuffer_create_info.height = texHeight;
    framebuffer_create_info.layers = 1;

    res = vkCreateFramebuffer(p_vkrs->device, &framebuffer_create_info, NULL, &image_sampler->framebuffer);
    VK_CHECK(res, "vkCreateFramebuffer");

  } break;
  case MVK_IMAGE_USAGE_RENDER_TARGET_3D: {
    // printf("MVK_IMAGE_USAGE_RENDER_TARGET_3D\n");
    // Create Framebuffer
    VkImageView attachments[2] = {image_sampler->view, p_vkrs->depth_buffer.view};

    VkFramebufferCreateInfo framebuffer_create_info = {};
    framebuffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.pNext = NULL;
    framebuffer_create_info.renderPass = p_vkrs->offscreen_render_pass_3d;
    framebuffer_create_info.attachmentCount = 2;
    framebuffer_create_info.pAttachments = attachments;
    framebuffer_create_info.width = texWidth;
    framebuffer_create_info.height = texHeight;
    framebuffer_create_info.layers = 1;

    res = vkCreateFramebuffer(p_vkrs->device, &framebuffer_create_info, NULL, &image_sampler->framebuffer);
    VK_CHECK(res, "vkCreateFramebuffer");
  } break;
  default:
    MCerror(536, "Unsupported image usage:%i", image_usage);
  }

  // Sampler
  VkSamplerCreateInfo samplerInfo = {};
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

  res = vkCreateSampler(p_vkrs->device, &samplerInfo, NULL, &image_sampler->sampler);
  VK_CHECK(res, "vkCreateSampler");

  *out_image = image_sampler;

  return res;
}

VkResult mvk_load_texture_from_file(vk_render_state *p_vkrs, const char *const filepath, uint *resource_uid)
{
  VkResult res;

  int texWidth, texHeight, texChannels;
  stbi_uc *pixels = stbi_load(filepath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  VkDeviceSize imageSize = texWidth * texHeight * 4;

  if (!pixels) {
    return VK_ERROR_UNKNOWN;
  }

  texture_image *texture;
  res = mvk_load_image_sampler(p_vkrs, texWidth, texHeight, texChannels, MVK_IMAGE_USAGE_READ_ONLY, pixels, &texture);
  VK_CHECK(res, "mvk_load_image_sampler");

  *resource_uid = texture->resource_uid;
  append_to_collection((void ***)&p_vkrs->textures.items, &p_vkrs->textures.alloc, &p_vkrs->textures.count, texture);

  stbi_image_free(pixels);

  printf("loaded %s> width:%i height:%i channels:%i\n", filepath, texWidth, texHeight, texChannels);

  return res;
}

VkResult mvk_create_empty_render_target(vk_render_state *p_vkrs, const uint width, const uint height,
                                        mvk_image_sampler_usage image_usage, uint *resource_uid)
{
  VkResult res;

  int texChannels = 4;
  stbi_uc *pixels = (stbi_uc *)malloc(sizeof(stbi_uc) * width * height * texChannels);
  VkDeviceSize imageSize = width * height * 4;
  for (int i = 0; i < (int)imageSize; ++i) {
    pixels[i] = 255;
  }

  if (!pixels) {
    return VK_ERROR_UNKNOWN;
  }

  texture_image *texture;
  res = mvk_load_image_sampler(p_vkrs, width, height, texChannels, image_usage, pixels, &texture);
  VK_CHECK(res, "mvk_load_image_sampler");

  *resource_uid = texture->resource_uid;
  append_to_collection((void ***)&p_vkrs->textures.items, &p_vkrs->textures.alloc, &p_vkrs->textures.count, texture);

  stbi_image_free(pixels);

  // printf("generated empty texture> width:%i height:%i channels:%i\n", width, height, texChannels);

  return res;
}

VkResult mvk_load_font(vk_render_state *p_vkrs, const char *const filepath, float font_height, uint *resource_uid)
{
  VkResult res;

  // printf("mvk_load_font:resource_uid=%p\n", resource_uid);

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
      if (p_vkrs->loaded_fonts.fonts[i].height == font_height &&
          !strcmp(p_vkrs->loaded_fonts.fonts[i].name, font_name)) {
        *resource_uid = p_vkrs->loaded_fonts.fonts[i].resource_uid;

        printf("using cached font texture> name:%s height:%.2f resource_uid:%u\n", font_name, font_height,
               *resource_uid);
        free(font_name);

        return VK_SUCCESS;
      }
    }
  }

  stbi_uc ttf_buffer[1 << 20];
  fread(ttf_buffer, 1, 1 << 20, fopen(filepath, "rb"));

  const int texWidth = 256, texHeight = 256, texChannels = 4;
  stbi_uc temp_bitmap[texWidth * texHeight];
  stbtt_bakedchar *cdata = (stbtt_bakedchar *)malloc(sizeof(stbtt_bakedchar) * 96); // ASCII 32..126 is 95 glyphs
  stbtt_BakeFontBitmap(ttf_buffer, 0, font_height, temp_bitmap, texWidth, texHeight, 32, 96,
                       cdata); // no guarantee this fits!

  // printf("garbagein: font_height:%f\n", font_height);
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

  texture_image *texture;
  res = mvk_load_image_sampler(p_vkrs, texWidth, texHeight, texChannels, MVK_IMAGE_USAGE_READ_ONLY, pixels, &texture);
  VK_CHECK(res, "mvk_load_image_sampler");

  *resource_uid = texture->resource_uid;
  append_to_collection((void ***)&p_vkrs->textures.items, &p_vkrs->textures.alloc, &p_vkrs->textures.count, texture);

  // Font is a common resource -- cache so multiple loads reference the same resource uid
  {
    if (p_vkrs->loaded_fonts.allocated < p_vkrs->loaded_fonts.count + 1) {
      int new_allocated = p_vkrs->loaded_fonts.allocated + 4 + p_vkrs->loaded_fonts.allocated / 4;
      font_resource *new_ary = (font_resource *)malloc(sizeof(font_resource) * new_allocated);

      if (p_vkrs->loaded_fonts.allocated) {
        memcpy(new_ary, p_vkrs->loaded_fonts.fonts, sizeof(font_resource) * p_vkrs->loaded_fonts.allocated);
        free(p_vkrs->loaded_fonts.fonts);
      }
      p_vkrs->loaded_fonts.fonts = new_ary;
      p_vkrs->loaded_fonts.allocated = new_allocated;
    }

    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].name = font_name;
    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].height = font_height;
    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].resource_uid = *resource_uid;
    p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].char_data = cdata;
    {
      float lowest = 500;
      for (int ci = 0; ci < 96; ++ci) {
        stbtt_aligned_quad q;

        // printf("garbagein: %i %i %f %f %i\n", (int)font_image->width, (int)font_image->height, align_x, align_y,
        // letter
        // - 32);
        float ax = 100, ay = 300;
        stbtt_GetBakedQuad(cdata, (int)texWidth, (int)texHeight, ci, &ax, &ay, &q, 1);
        if (q.y0 < lowest)
          lowest = q.y0;
        // printf("baked_quad: s0=%.2f s1==%.2f t0=%.2f t1=%.2f x0=%.2f x1=%.2f y0=%.2f y1=%.2f lowest=%.3f\n", q.s0,
        // q.s1,
        //        q.t0, q.t1, q.x0, q.x1, q.y0, q.y1, lowest);
      }
      p_vkrs->loaded_fonts.fonts[p_vkrs->loaded_fonts.count].draw_vertical_offset = 300 - lowest;
    }
    ++p_vkrs->loaded_fonts.count;
  }

  printf("generated font texture> name:%s height:%.2f resource_uid:%u\n", font_name, font_height, *resource_uid);

  return res;
}

VkResult mvk_load_vertex_data(vk_render_state *p_vkrs, float *p_data, unsigned int data_count,
                              bool release_original_data_on_copy, uint *resource_uid)
{
  // printf("mvk_load_vertex_data:%p (%u)\n", p_data, data_count);

  VkResult res = VK_SUCCESS;

  // vec3 mesh_data[] = {{-0.5f, -0.5f, -0.5f}, {-0.5f, -0.5f, 0.5f}, {-0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, 0.5f},
  //                     {0.5f, -0.5f, -0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, -0.5f},  {0.5f, 0.5f, 0.5f}};

  mrt_vertex_data *mesh = (mrt_vertex_data *)malloc(sizeof(mrt_vertex_data));
  mesh->resource_uid = p_vkrs->resource_uid_counter++;

  const int data_size_in_bytes = sizeof(float) * data_count;

  // Buffer
  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buf_info.size = data_size_in_bytes;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &mesh->buf);
  VK_CHECK(res, "vkCreateBuffer");

  // Memory
  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(p_vkrs->device, mesh->buf, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  bool pass = mvk_get_properties_memory_type_index(
      p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &alloc_info.memoryTypeIndex);
  VK_ASSERT(pass, "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(mesh->mem));
  VK_CHECK(res, "vkAllocateMemory");
  mesh->buffer_info.range = mem_reqs.size;
  mesh->buffer_info.offset = 0;

  // Bind
  uint8_t *p_mapped_mem;
  res = vkMapMemory(p_vkrs->device, mesh->mem, 0, mem_reqs.size, 0, (void **)&p_mapped_mem);
  VK_CHECK(res, "vkMapMemory");

  memcpy(p_mapped_mem, p_data, data_size_in_bytes);

  vkUnmapMemory(p_vkrs->device, mesh->mem);

  res = vkBindBufferMemory(p_vkrs->device, mesh->buf, mesh->mem, 0);
  VK_CHECK(res, "vkBindBufferMemory");

  // Register the mesh
  append_to_collection((void ***)&p_vkrs->loaded_vertex_data.items, &p_vkrs->loaded_vertex_data.alloc,
                       &p_vkrs->loaded_vertex_data.count, mesh);

  *resource_uid = mesh->resource_uid;
  printf("vertex_data resource %u loaded\n", *resource_uid);

  if (release_original_data_on_copy) {
    free(p_data);
  }

  return res;
}

VkResult mvk_load_index_buffer(vk_render_state *p_vkrs, unsigned int *p_data, unsigned int data_count,
                               bool release_original_data_on_copy, unsigned int *resource_uid)
{
  // printf("mvk_load_index_buffer:%p (%u)\n", p_data, data_count);

  VkResult res = VK_SUCCESS;

  mrt_index_data *index_buffer = (mrt_index_data *)malloc(sizeof(mrt_index_data));
  index_buffer->resource_uid = p_vkrs->resource_uid_counter++;
  index_buffer->count = data_count;

  const int data_size_in_bytes = sizeof(unsigned int) * data_count;

  // Buffer
  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
  buf_info.size = data_size_in_bytes;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &index_buffer->buf);
  VK_CHECK(res, "vkCreateBuffer");

  // Memory
  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(p_vkrs->device, index_buffer->buf, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  bool pass = mvk_get_properties_memory_type_index(
      p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &alloc_info.memoryTypeIndex);
  VK_ASSERT(pass, "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(index_buffer->mem));
  VK_CHECK(res, "vkAllocateMemory");
  index_buffer->buffer_info.range = mem_reqs.size;
  index_buffer->buffer_info.offset = 0;

  // Bind
  uint8_t *p_mapped_mem;
  res = vkMapMemory(p_vkrs->device, index_buffer->mem, 0, mem_reqs.size, 0, (void **)&p_mapped_mem);
  VK_CHECK(res, "vkMapMemory");

  memcpy(p_mapped_mem, p_data, data_size_in_bytes);

  vkUnmapMemory(p_vkrs->device, index_buffer->mem);

  res = vkBindBufferMemory(p_vkrs->device, index_buffer->buf, index_buffer->mem, 0);
  VK_CHECK(res, "vkBindBufferMemory");

  // Register the index_buffer
  append_to_collection((void ***)&p_vkrs->loaded_index_data.items, &p_vkrs->loaded_index_data.alloc,
                       &p_vkrs->loaded_index_data.count, index_buffer);

  *resource_uid = index_buffer->resource_uid;
  printf("index_data resource %u loaded\n", *resource_uid);

  if (release_original_data_on_copy) {
    free(p_data);
  }

  return res;
}

VkResult mvk_create_render_program(vk_render_state *p_vkrs, mcr_render_program_create_info *create_info,
                                   unsigned int *resource_uid)
{
  VkResult res = VK_SUCCESS;

  render_program *render_prog = (render_program *)malloc(sizeof(render_program));
  render_prog->resource_uid = p_vkrs->resource_uid_counter++;

  // CreateDescriptorSetLayout
  {
    const int binding_count = 3;
    VkDescriptorSetLayoutBinding layout_bindings[binding_count];
    layout_bindings[0].binding = 0;
    layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[0].descriptorCount = 1;
    layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[0].pImmutableSamplers = NULL;

    layout_bindings[1].binding = 1;
    layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[1].descriptorCount = 1;
    layout_bindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    layout_bindings[1].pImmutableSamplers = NULL;

    layout_bindings[2].binding = 2;
    layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_bindings[2].descriptorCount = 1;
    layout_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_bindings[2].pImmutableSamplers = NULL;

    // Next take layout bindings and use them to create a descriptor set layout
    VkDescriptorSetLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.pNext = NULL;
    layout_create_info.flags = 0;
    layout_create_info.bindingCount = 3;
    layout_create_info.pBindings = layout_bindings;

    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layout_create_info, NULL, &render_prog->descriptor_layout);
    VK_CHECK(res, "vkCreateDescriptorSetLayout");
  }

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
    {
      char *vertex_shader_code;
      read_file_text(create_info->vertex_shader_filepath, &vertex_shader_code);

      VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[0];
      shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shaderStateCreateInfo->pNext = NULL;
      shaderStateCreateInfo->pSpecializationInfo = NULL;
      shaderStateCreateInfo->flags = 0;
      shaderStateCreateInfo->stage = VK_SHADER_STAGE_VERTEX_BIT;
      shaderStateCreateInfo->pName = "main";

      unsigned int *vtx_spv, vtx_spv_size;
      VkResult res = GLSLtoSPV(VK_SHADER_STAGE_VERTEX_BIT, vertex_shader_code, &vtx_spv, &vtx_spv_size);
      VK_CHECK(res, "GLSLtoSPV");
      free(vertex_shader_code);

      VkShaderModuleCreateInfo moduleCreateInfo;
      moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      moduleCreateInfo.pNext = NULL;
      moduleCreateInfo.flags = 0;
      moduleCreateInfo.codeSize = vtx_spv_size * sizeof(unsigned int);
      moduleCreateInfo.pCode = vtx_spv;

      res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &shaderStateCreateInfo->module);
      VK_CHECK(res, "vkCreateShaderModule");

      free(vtx_spv);
    }
    {
      char *fragment_shader_code;
      read_file_text(create_info->fragment_shader_filepath, &fragment_shader_code);

      VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[1];
      shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
      shaderStateCreateInfo->pNext = NULL;
      shaderStateCreateInfo->pSpecializationInfo = NULL;
      shaderStateCreateInfo->flags = 0;
      shaderStateCreateInfo->stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      shaderStateCreateInfo->pName = "main";

      unsigned int *vtx_spv, vtx_spv_size;
      VkResult res = GLSLtoSPV(VK_SHADER_STAGE_FRAGMENT_BIT, fragment_shader_code, &vtx_spv, &vtx_spv_size);
      VK_CHECK(res, "GLSLtoSPV");
      free(fragment_shader_code);

      VkShaderModuleCreateInfo moduleCreateInfo;
      moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
      moduleCreateInfo.pNext = NULL;
      moduleCreateInfo.flags = 0;
      moduleCreateInfo.codeSize = vtx_spv_size * sizeof(unsigned int);
      moduleCreateInfo.pCode = vtx_spv;

      res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &shaderStateCreateInfo->module);
      VK_CHECK(res, "vkCreateShaderModule");

      free(vtx_spv);
    }
  }

  // Vertex Bindings
  VkVertexInputBindingDescription bindingDescription = {};
  const int VERTEX_ATTRIBUTE_COUNT = 2;
  VkVertexInputAttributeDescription attributeDescriptions[VERTEX_ATTRIBUTE_COUNT];
  {
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(float) * 5;
    // printf("sizeof(vec2)=%zu\n", sizeof(vec2));
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // p_vkrs->format; // VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;                          // offsetof(textured_image_vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT; // p_vkrs->format; // VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = sizeof(float) * 3;       // offsetof(textured_image_vertex, position);
  }

  {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = NULL;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof(dynamicStateEnables));
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = NULL;
    viewportState.flags = 0;
    viewportState.viewportCount = 1; // NUM_VIEWPORTS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    viewportState.scissorCount = 1; // NUM_SCISSORS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    viewportState.pScissors = NULL;
    viewportState.pViewports = NULL;

    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; // VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState att_state[1];
    att_state[0].colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    att_state[0].blendEnable = VK_TRUE;
    att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.flags = 0;
    colorBlending.pNext = NULL;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = att_state;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.blendConstants[0] = 1.0f;
    colorBlending.blendConstants[1] = 1.0f;
    colorBlending.blendConstants[2] = 1.0f;
    colorBlending.blendConstants[3] = 1.0f;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &render_prog->descriptor_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, NULL, &render_prog->pipeline_layout);
    VK_CHECK(res, "vkCreatePipelineLayout :: Failed to create pipeline layout!");

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {};  // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = render_prog->pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->offscreen_render_pass_3d;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res =
        vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &render_prog->pipeline);
    VK_CHECK(res, "vkCreateGraphicsPipelines :: Failed to create pipeline");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, NULL);
  }

  // Register the render program
  append_to_collection((void ***)&p_vkrs->loaded_render_programs.items, &p_vkrs->loaded_render_programs.alloc,
                       &p_vkrs->loaded_render_programs.count, render_prog);

  *resource_uid = render_prog->resource_uid;
  printf("render_prog resource %u loaded\n", *resource_uid);

  return res;
}