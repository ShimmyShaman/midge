
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

    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&pData);
    VK_CHECK(res, "vkMapMemory");

    memcpy(pData, g_vb_shape_data, data_size_in_bytes);

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

    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&pData);
    VK_CHECK(res, "vkMapMemory");

    memcpy(pData, g_vb_textured_shape_2D_data, data_size_in_bytes);

    vkUnmapMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem);

    res =
        vkBindBufferMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.buf, p_vkrs->textured_shape_vertices.mem, 0);
    VK_CHECK(res, "vkBindBufferMemory");
  }
  {
    vec3 mesh_data[] = {{0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 1.f},
                        {1.f, 0.f, 0.f}, {1.f, 0.f, 1.f}, {1.f, 1.f, 0.f}, {1.f, 1.f, 1.f}};

    // Cube vertices TEMP DEBUG
    const int data_size_in_bytes = sizeof(mesh_data);

    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.size = data_size_in_bytes;
    buf_info.queueFamilyIndexCount = 0;
    buf_info.pQueueFamilyIndices = NULL;
    buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    buf_info.flags = 0;
    res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->cube_shape_vertices.buf);
    VK_CHECK(res, "vkCreateBuffer");

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->cube_shape_vertices.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    bool pass = mvk_get_properties_memory_type_index(
        p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &alloc_info.memoryTypeIndex);
    VK_ASSERT(pass, "No mappable, coherent memory");

    res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->cube_shape_vertices.mem));
    VK_CHECK(res, "vkAllocateMemory");
    p_vkrs->cube_shape_vertices.buffer_info.range = mem_reqs.size;
    p_vkrs->cube_shape_vertices.buffer_info.offset = 0;

    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->cube_shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&pData);
    VK_CHECK(res, "vkMapMemory");

    memcpy(pData, g_vb_textured_shape_2D_data, data_size_in_bytes);

    vkUnmapMemory(p_vkrs->device, p_vkrs->cube_shape_vertices.mem);

    res = vkBindBufferMemory(p_vkrs->device, p_vkrs->cube_shape_vertices.buf, p_vkrs->cube_shape_vertices.mem, 0);
    VK_CHECK(res, "vkBindBufferMemory");
  }

  return res;
}

VkResult mvk_init_resources(vk_render_state *p_vkrs)
{
  VkResult res;

  res = mvk_init_shape_vertices(p_vkrs);
  VK_CHECK(res, "mvk_init_shape_vertices");

  return res;
}

void mvk_destroy_sampled_image(vk_render_state *p_vkrs, sampled_image *sampled_image)
{
  vkDestroySampler(p_vkrs->device, sampled_image->sampler, NULL);
  vkDestroyImageView(p_vkrs->device, sampled_image->view, NULL);
  vkDestroyImage(p_vkrs->device, sampled_image->image, NULL);
  vkFreeMemory(p_vkrs->device, sampled_image->memory, NULL);
  if (sampled_image->framebuffer)
    vkDestroyFramebuffer(p_vkrs->device, sampled_image->framebuffer, NULL);
}

void mvk_destroy_resources(vk_render_state *p_vkrs)
{
  for (int i = 0; i < p_vkrs->textures.count; ++i) {
    mvk_destroy_sampled_image(p_vkrs, &p_vkrs->textures.samples[i]);
  }
  free(p_vkrs->textures.samples);

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
                                bool use_as_render_target, const unsigned char *const pixels,
                                sampled_image *image_sampler)
{
  VkResult res;

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
  VkImageUsageFlags image_usage;
  if (use_as_render_target) {
    image_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    image_sampler->format = p_vkrs->format;
  }
  else {
    image_usage = 0;
    image_sampler->format = VK_IMAGE_FORMAT;
  }
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
  imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | image_usage;
  imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageInfo.flags = 0; // Optional

  res = vkCreateImage(p_vkrs->device, &imageInfo, NULL, &image_sampler->image);
  VK_CHECK(res, "vkCreateImage");

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

  // TODO ??
  image_sampler->framebuffer = NULL;

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

  res = mvk_load_image_sampler(p_vkrs, texWidth, texHeight, texChannels, false, pixels,
                               &p_vkrs->textures.samples[p_vkrs->textures.count]);
  VK_CHECK(res, "mvk_load_image_sampler");
  *resource_uid = RESOURCE_UID_BEGIN + p_vkrs->textures.count;
  ++p_vkrs->textures.count;

  stbi_image_free(pixels);

  printf("loaded %s> width:%i height:%i channels:%i\n", filepath, texWidth, texHeight, texChannels);

  return res;
}

VkResult mvk_create_empty_render_target(vk_render_state *p_vkrs, const uint width, const uint height,
                                        bool use_as_render_target, uint *resource_uid)
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

  res = mvk_load_image_sampler(p_vkrs, width, height, texChannels, use_as_render_target, pixels,
                               &p_vkrs->textures.samples[p_vkrs->textures.count]);
  VK_CHECK(res, "mvk_load_image_sampler");

  // printf("p_vkrs->textures:%u  %u\n", p_vkrs->textures.count, p_vkrs->textures.allocated);

  *resource_uid = RESOURCE_UID_BEGIN + p_vkrs->textures.count;
  ++p_vkrs->textures.count;

  stbi_image_free(pixels);

  printf("generated empty texture> width:%i height:%i channels:%i\n", width, height, texChannels);

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

  res = mvk_load_image_sampler(p_vkrs, texWidth, texHeight, texChannels, false, pixels,
                               &p_vkrs->textures.samples[p_vkrs->textures.count]);
  VK_CHECK(res, "mvk_load_image_sampler");
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
  // global_root_data *global_data;
  // obtain_midge_global_root(&global_data);
  // printf("generated font texture> resource_uid:%u\n", global_data->ui_state->default_font_resource);

  return res;
}