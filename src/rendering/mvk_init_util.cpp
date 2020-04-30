/* mvk_init_util.c */

#include "rendering/mvk_init_util.h"

/*
 * Obtains the memory type from the available properties, returning false if no memory type was matched.
 */
bool memory_type_from_properties(vk_render_state *p_vkrs, uint32_t typeBits, VkFlags requirements_mask, uint32_t *typeIndex)
{
  // Search memtypes to find first index with those properties
  for (uint32_t i = 0; i < p_vkrs->memory_properties.memoryTypeCount; i++)
  {
    if ((typeBits & 1) == 1)
    {
      // Type is available, does it match user properties?
      if ((p_vkrs->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask)
      {
        *typeIndex = i;
        return true;
      }
    }
    typeBits >>= 1;
  }
  // No memory types matched, return failure
  return false;
}

/*
 * Initializes a discovered global extension property.
 */
VkResult mvk_init_global_extension_properties(layer_properties &layer_props)
{
  VkExtensionProperties *instance_extensions;
  uint32_t instance_extension_count;
  VkResult res;
  char *layer_name = NULL;

  layer_name = layer_props.properties.layerName;

  do
  {
    res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, NULL);
    if (res)
      return res;

    if (instance_extension_count == 0)
    {
      return VK_SUCCESS;
    }

    layer_props.instance_extensions.resize(instance_extension_count);
    instance_extensions = layer_props.instance_extensions.data();
    res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, instance_extensions);
  } while (res == VK_INCOMPLETE);

  return res;
}

/*
 * Enumerates through all globally accessible instance layers and fills a vector with their retrieved
 * properties.
 */
VkResult mvk_init_global_layer_properties(std::vector<layer_properties> *p_vk_layers)
{
  uint32_t instance_layer_count;
  VkLayerProperties *vk_props = NULL;
  VkResult res;

  /*
     * It's possible, though very rare, that the number of
     * instance layers could change. For example, installing something
     * could include new layers that the loader would pick up
     * between the initial query for the count and the
     * request for VkLayerProperties. The loader indicates that
     * by returning a VK_INCOMPLETE status and will update the
     * the count parameter.
     * The count parameter will be updated with the number of
     * entries loaded into the data pointer - in case the number
     * of layers went down or is smaller than the size given.
     */
  do
  {
    res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
    if (res)
      return res;

    if (instance_layer_count == 0)
    {
      return VK_SUCCESS;
    }

    vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

    res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
  } while (res == VK_INCOMPLETE);

  /*
     * Now gather the extension list for each instance layer.
     */
  for (uint32_t i = 0; i < instance_layer_count; i++)
  {
    layer_properties layer_props;
    layer_props.properties = vk_props[i];
    res = mvk_init_global_extension_properties(layer_props);
    if (res)
      return res;
    p_vk_layers->push_back(layer_props);
  }
  free(vk_props);

  return res;
}

VkResult mvk_init_instance(vk_render_state *p_vkrs, char const *const app_short_name)
{
  VkApplicationInfo app_info = {};
  app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  app_info.pNext = NULL;
  app_info.pApplicationName = app_short_name;
  app_info.applicationVersion = 1;
  app_info.pEngineName = app_short_name;
  app_info.engineVersion = 1;
  app_info.apiVersion = VK_API_VERSION_1_0;

  // -- Layers & Extensions --
  p_vkrs->instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = p_vkrs->instance_layer_names.size();
  inst_info.ppEnabledLayerNames = p_vkrs->instance_layer_names.size() ? p_vkrs->instance_layer_names.data() : NULL;
  inst_info.enabledExtensionCount = p_vkrs->instance_extension_names.size();
  inst_info.ppEnabledExtensionNames = p_vkrs->instance_extension_names.data();

  printf("create VkInstance...");
  VkResult res = vkCreateInstance(&inst_info, NULL, &p_vkrs->inst);
  assert(res == VK_SUCCESS);
  printf("SUCCESS\n");

  return res;
}

void mvk_init_device_extension_names(vk_render_state *p_vkrs)
{
  p_vkrs->device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

VkResult init_device_extension_properties(vk_render_state *p_vkrs, layer_properties &layer_props)
{
  VkExtensionProperties *device_extensions;
  uint32_t device_extension_count;
  VkResult res;
  char *layer_name = NULL;

  layer_name = layer_props.properties.layerName;

  do
  {
    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &device_extension_count, NULL);
    if (res)
      return res;

    if (device_extension_count == 0)
    {
      return VK_SUCCESS;
    }

    layer_props.device_extensions.resize(device_extension_count);
    device_extensions = layer_props.device_extensions.data();
    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &device_extension_count, device_extensions);
  } while (res == VK_INCOMPLETE);

  return res;
}

/*
 * Initialize the graphics device.
 */
VkResult mvk_init_device(vk_render_state *p_vkrs)
{
  // printf("mvk_init_device\n");
  VkResult res;
  VkDeviceQueueCreateInfo queue_info = {};

  float queue_priorities[1] = {0.0};
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;
  queue_info.queueFamilyIndex = p_vkrs->graphics_queue_family_index;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = p_vkrs->device_extension_names.size();
  device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? p_vkrs->device_extension_names.data() : NULL;
  device_info.pEnabledFeatures = NULL;

  res = vkCreateDevice(p_vkrs->gpus[0], &device_info, NULL, &p_vkrs->device);
  assert(res == VK_SUCCESS);

  return res;
}

/*
 * Enumerates through the available graphics devices.
 */
VkResult mvk_init_enumerate_device(vk_render_state *p_vkrs, const uint32_t required_gpu_count)
{
  uint32_t gpu_count = required_gpu_count;
  VkResult res = vkEnumeratePhysicalDevices(p_vkrs->inst, &gpu_count, NULL);
  assert(gpu_count);
  p_vkrs->gpus.resize(gpu_count);

  res = vkEnumeratePhysicalDevices(p_vkrs->inst, &gpu_count, p_vkrs->gpus.data());
  assert(!res && gpu_count >= required_gpu_count);

  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, NULL);
  assert(p_vkrs->queue_family_count >= 1);

  p_vkrs->queue_props.resize(p_vkrs->queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, p_vkrs->queue_props.data());
  assert(p_vkrs->queue_family_count >= 1);

  /* This is as good a place as any to do this */
  vkGetPhysicalDeviceMemoryProperties(p_vkrs->gpus[0], &p_vkrs->memory_properties);
  vkGetPhysicalDeviceProperties(p_vkrs->gpus[0], &p_vkrs->gpu_props);
  /* query device extensions for enabled layers */
  for (auto &layer_props : p_vkrs->instance_layer_properties)
  {
    init_device_extension_properties(p_vkrs, layer_props);
  }

  return res;
}

VkResult mvk_init_depth_buffer(vk_render_state *p_vkrs)
{
  // printf("mvk_init_depth_buffer\n");
  VkResult res;
  bool pass;
  VkImageCreateInfo image_info = {};
  VkFormatProperties props;

  /* allow custom depth formats */
  if (p_vkrs->depth.format == VK_FORMAT_UNDEFINED)
    p_vkrs->depth.format = VK_FORMAT_D16_UNORM;

  const VkFormat depth_format = p_vkrs->depth.format;
  vkGetPhysicalDeviceFormatProperties(p_vkrs->gpus[0], depth_format, &props);
  if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
  {
    image_info.tiling = VK_IMAGE_TILING_LINEAR;
  }
  else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
  {
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
  }
  else
  {
    /* Try other depth formats? */
    printf("depth_format:%i is unsupported\n", depth_format);
    return (VkResult)MVK_ERROR_UNSUPPORTED_DEPTH_FORMAT;
  }

  image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  image_info.pNext = NULL;
  image_info.imageType = VK_IMAGE_TYPE_2D;
  image_info.format = depth_format;
  image_info.extent.width = p_vkrs->window_width;
  image_info.extent.height = p_vkrs->window_height;
  image_info.extent.depth = 1;
  image_info.mipLevels = 1;
  image_info.arrayLayers = 1;
  image_info.samples = NUM_SAMPLES;
  image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  image_info.queueFamilyIndexCount = 0;
  image_info.pQueueFamilyIndices = NULL;
  image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
  image_info.flags = 0;

  VkMemoryAllocateInfo mem_alloc = {};
  mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  mem_alloc.pNext = NULL;
  mem_alloc.allocationSize = 0;
  mem_alloc.memoryTypeIndex = 0;

  VkImageViewCreateInfo view_info = {};
  view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  view_info.pNext = NULL;
  view_info.image = VK_NULL_HANDLE;
  view_info.format = depth_format;
  view_info.components.r = VK_COMPONENT_SWIZZLE_R;
  view_info.components.g = VK_COMPONENT_SWIZZLE_G;
  view_info.components.b = VK_COMPONENT_SWIZZLE_B;
  view_info.components.a = VK_COMPONENT_SWIZZLE_A;
  view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  view_info.subresourceRange.baseMipLevel = 0;
  view_info.subresourceRange.levelCount = 1;
  view_info.subresourceRange.baseArrayLayer = 0;
  view_info.subresourceRange.layerCount = 1;
  view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
  view_info.flags = 0;

  if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
      depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT)
  {
    view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
  }

  VkMemoryRequirements mem_reqs;

  /* Create image */
  res = vkCreateImage(p_vkrs->device, &image_info, NULL, &p_vkrs->depth.image);
  assert(res == VK_SUCCESS);

  vkGetImageMemoryRequirements(p_vkrs->device, p_vkrs->depth.image, &mem_reqs);

  mem_alloc.allocationSize = mem_reqs.size;
  /* Use the memory properties to determine the type of memory required */
  pass = memory_type_from_properties(p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &mem_alloc.memoryTypeIndex);
  assert(pass);

  /* Allocate memory */
  res = vkAllocateMemory(p_vkrs->device, &mem_alloc, NULL, &p_vkrs->depth.mem);
  assert(res == VK_SUCCESS);

  /* Bind memory */
  res = vkBindImageMemory(p_vkrs->device, p_vkrs->depth.image, p_vkrs->depth.mem, 0);
  assert(res == VK_SUCCESS);

  /* Create image view */
  view_info.image = p_vkrs->depth.image;
  res = vkCreateImageView(p_vkrs->device, &view_info, NULL, &p_vkrs->depth.view);
  assert(res == VK_SUCCESS);

  return res;
}

/*
 * Constructs the surface, finds a graphics and present queue for it as well as a supported format.
*/
VkResult mvk_init_swapchain_extension(vk_render_state *p_vkrs)
{
  VkResult res;

  // Construct the surface description
  VkXcbSurfaceCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.connection = p_vkrs->xcb_winfo->connection;
  createInfo.window = p_vkrs->xcb_winfo->window;
  res = vkCreateXcbSurfaceKHR(p_vkrs->inst, &createInfo, NULL, &p_vkrs->surface);
  assert(res == VK_SUCCESS);

  // Iterate over each queue to learn whether it supports presenting:
  VkBool32 *pSupportsPresent = (VkBool32 *)malloc(p_vkrs->queue_family_count * sizeof(VkBool32));
  for (uint32_t i = 0; i < p_vkrs->queue_family_count; i++)
  {
    vkGetPhysicalDeviceSurfaceSupportKHR(p_vkrs->gpus[0], i, p_vkrs->surface, &pSupportsPresent[i]);
  }

  // Search for a graphics and a present queue in the array of queue
  // families, try to find one that supports both
  p_vkrs->graphics_queue_family_index = UINT32_MAX;
  p_vkrs->present_queue_family_index = UINT32_MAX;
  for (uint32_t i = 0; i < p_vkrs->queue_family_count; ++i)
  {
    if ((p_vkrs->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
    {
      if (p_vkrs->graphics_queue_family_index == UINT32_MAX)
        p_vkrs->graphics_queue_family_index = i;

      if (pSupportsPresent[i] == VK_TRUE)
      {
        p_vkrs->graphics_queue_family_index = i;
        p_vkrs->present_queue_family_index = i;
        break;
      }
    }
  }

  if (p_vkrs->present_queue_family_index == UINT32_MAX)
  {
    // If didn't find a queue that supports both graphics and present, then
    // find a separate present queue.
    for (size_t i = 0; i < p_vkrs->queue_family_count; ++i)
      if (pSupportsPresent[i] == VK_TRUE)
      {
        p_vkrs->present_queue_family_index = i;
        break;
      }
  }
  free(pSupportsPresent);

  // Generate error if could not find queues that support graphics
  // and present
  if (p_vkrs->graphics_queue_family_index == UINT32_MAX)
    return (VkResult)MVK_ERROR_GRAPHIC_QUEUE_NOT_FOUND;
  if (p_vkrs->present_queue_family_index == UINT32_MAX)
    return (VkResult)MVK_ERROR_PRESENT_QUEUE_NOT_FOUND;

  // Get the list of VkFormats that are supported:
  uint32_t formatCount;
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(p_vkrs->gpus[0], p_vkrs->surface, &formatCount, NULL);
  assert(res == VK_SUCCESS);
  VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(p_vkrs->gpus[0], p_vkrs->surface, &formatCount, surfFormats);
  assert(res == VK_SUCCESS);

  // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
  // the surface has no preferred format.  Otherwise, at least one
  // supported format will be returned.
  if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
  {
    p_vkrs->format = VK_FORMAT_B8G8R8A8_UNORM;
  }
  else
  {
    assert(formatCount >= 1);
    p_vkrs->format = surfFormats[0].format;
  }
  free(surfFormats);

  return VK_SUCCESS;
}

/*
 * Initializes the swap chain and the images it uses.
 */
VkResult mvk_init_swapchain(vk_render_state *p_vkrs, VkImageUsageFlags default_image_usage_flags)
{
  /* DEPENDS on p_vkrs->cmd and p_vkrs->queue initialized */

  VkResult res;
  VkSurfaceCapabilitiesKHR surfCapabilities;

  res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_vkrs->gpus[0], p_vkrs->surface, &surfCapabilities);
  assert(res == VK_SUCCESS);

  uint32_t presentModeCount;
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(p_vkrs->gpus[0], p_vkrs->surface, &presentModeCount, NULL);
  assert(res == VK_SUCCESS);
  VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
  assert(presentModes);
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(p_vkrs->gpus[0], p_vkrs->surface, &presentModeCount, presentModes);
  assert(res == VK_SUCCESS);

  VkExtent2D swapchainExtent;
  // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
  if (surfCapabilities.currentExtent.width == 0xFFFFFFFF)
  {
    // If the surface size is undefined, the size is set to
    // the size of the images requested.
    swapchainExtent.width = p_vkrs->window_width;
    swapchainExtent.height = p_vkrs->window_height;
    if (swapchainExtent.width < surfCapabilities.minImageExtent.width)
    {
      swapchainExtent.width = surfCapabilities.minImageExtent.width;
    }
    else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width)
    {
      swapchainExtent.width = surfCapabilities.maxImageExtent.width;
    }

    if (swapchainExtent.height < surfCapabilities.minImageExtent.height)
    {
      swapchainExtent.height = surfCapabilities.minImageExtent.height;
    }
    else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height)
    {
      swapchainExtent.height = surfCapabilities.maxImageExtent.height;
    }
  }
  else
  {
    // If the surface size is defined, the swap chain size must match
    swapchainExtent = surfCapabilities.currentExtent;
  }

  // The FIFO present mode is guaranteed by the spec to be supported
  // Also note that current Android driver only supports FIFO
  VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

  // Determine the number of VkImage's to use in the swap chain.
  // We need to acquire only 1 presentable image at at time.
  // Asking for minImageCount images ensures that we can acquire
  // 1 presentable image as long as we present it before attempting
  // to acquire another.
  uint32_t desiredNumberOfSwapChainImages = surfCapabilities.minImageCount;

  VkSurfaceTransformFlagBitsKHR preTransform;
  if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
  {
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }
  else
  {
    preTransform = surfCapabilities.currentTransform;
  }

  // Find a supported composite alpha mode - one of these is guaranteed to be set
  VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
  VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[4] = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
  };
  for (uint32_t i = 0; i < sizeof(compositeAlphaFlags) / sizeof(compositeAlphaFlags[0]); i++)
  {
    if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i])
    {
      compositeAlpha = compositeAlphaFlags[i];
      break;
    }
  }

  VkSwapchainCreateInfoKHR swapchain_ci = {};
  swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
  swapchain_ci.pNext = NULL;
  swapchain_ci.surface = p_vkrs->surface;
  swapchain_ci.minImageCount = desiredNumberOfSwapChainImages;
  swapchain_ci.imageFormat = p_vkrs->format;
  swapchain_ci.imageExtent.width = swapchainExtent.width;
  swapchain_ci.imageExtent.height = swapchainExtent.height;
  swapchain_ci.preTransform = preTransform;
  swapchain_ci.compositeAlpha = compositeAlpha;
  swapchain_ci.imageArrayLayers = 1;
  swapchain_ci.presentMode = swapchainPresentMode;
  swapchain_ci.oldSwapchain = VK_NULL_HANDLE;
  swapchain_ci.clipped = true;
  swapchain_ci.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
  swapchain_ci.imageUsage = default_image_usage_flags;
  swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_ci.queueFamilyIndexCount = 0;
  swapchain_ci.pQueueFamilyIndices = NULL;
  uint32_t queueFamilyIndices[2] = {(uint32_t)p_vkrs->graphics_queue_family_index, (uint32_t)p_vkrs->present_queue_family_index};
  if (p_vkrs->graphics_queue_family_index != p_vkrs->present_queue_family_index)
  {
    // If the graphics and present queues are from different queue families,
    // we either have to explicitly transfer ownership of images between the
    // queues, or we have to create the swapchain with imageSharingMode
    // as VK_SHARING_MODE_CONCURRENT
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
  }

  res = vkCreateSwapchainKHR(p_vkrs->device, &swapchain_ci, NULL, &p_vkrs->swap_chain);
  assert(res == VK_SUCCESS);

  res = vkGetSwapchainImagesKHR(p_vkrs->device, p_vkrs->swap_chain, &p_vkrs->swapchainImageCount, NULL);
  assert(res == VK_SUCCESS);

  VkImage *swapchainImages = (VkImage *)malloc(p_vkrs->swapchainImageCount * sizeof(VkImage));
  assert(swapchainImages);
  res = vkGetSwapchainImagesKHR(p_vkrs->device, p_vkrs->swap_chain, &p_vkrs->swapchainImageCount, swapchainImages);
  assert(res == VK_SUCCESS);

  for (uint32_t i = 0; i < p_vkrs->swapchainImageCount; i++)
  {
    swap_chain_buffer sc_buffer;

    VkImageViewCreateInfo color_image_view = {};
    color_image_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    color_image_view.pNext = NULL;
    color_image_view.format = p_vkrs->format;
    color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
    color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
    color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
    color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
    color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    color_image_view.subresourceRange.baseMipLevel = 0;
    color_image_view.subresourceRange.levelCount = 1;
    color_image_view.subresourceRange.baseArrayLayer = 0;
    color_image_view.subresourceRange.layerCount = 1;
    color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    color_image_view.flags = 0;

    sc_buffer.image = swapchainImages[i];

    color_image_view.image = sc_buffer.image;

    res = vkCreateImageView(p_vkrs->device, &color_image_view, NULL, &sc_buffer.view);
    p_vkrs->buffers.push_back(sc_buffer);
    assert(res == VK_SUCCESS);
  }
  free(swapchainImages);
  p_vkrs->current_buffer = 0;

  if (NULL != presentModes)
  {
    free(presentModes);
  }

  return VK_SUCCESS;
}

VkResult mvk_init_uniform_buffer(vk_render_state *p_vkrs)
{
  VkResult res;
  bool pass;
  float fov = glm::radians(45.0f);
  if (p_vkrs->window_width > p_vkrs->window_height)
  {
    fov *= static_cast<float>(p_vkrs->window_height) / static_cast<float>(p_vkrs->window_width);
  }
  p_vkrs->Projection = glm::perspective(fov, static_cast<float>(p_vkrs->window_width) / static_cast<float>(p_vkrs->window_height), 0.1f, 100.0f);
  p_vkrs->View = glm::lookAt(glm::vec3(-5, 3, -10), // Camera is at (-5,3,-10), in World Space
                             glm::vec3(0, 0, 0),    // and looks at the origin
                             glm::vec3(0, -1, 0)    // Head is up (set to 0,-1,0 to look upside-down)
  );
  p_vkrs->Model = glm::mat4(1.0f);
  // Vulkan clip space has inverted Y and half Z.
  p_vkrs->Clip = glm::mat4(1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f);

  p_vkrs->MVP = p_vkrs->Clip * p_vkrs->Projection * p_vkrs->View * p_vkrs->Model;

  /* VULKAN_KEY_START */
  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  buf_info.size = sizeof(p_vkrs->MVP);
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->uniform_data.buf);
  assert(res == VK_SUCCESS);

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->uniform_data.buf, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  pass = memory_type_from_properties(p_vkrs, mem_reqs.memoryTypeBits,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                     &alloc_info.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->uniform_data.mem));
  assert(res == VK_SUCCESS);

  uint8_t *pData;
  res = vkMapMemory(p_vkrs->device, p_vkrs->uniform_data.mem, 0, mem_reqs.size, 0, (void **)&pData);
  assert(res == VK_SUCCESS);

  memcpy(pData, &p_vkrs->MVP, sizeof(p_vkrs->MVP));

  vkUnmapMemory(p_vkrs->device, p_vkrs->uniform_data.mem);

  res = vkBindBufferMemory(p_vkrs->device, p_vkrs->uniform_data.buf, p_vkrs->uniform_data.mem, 0);
  assert(res == VK_SUCCESS);

  p_vkrs->uniform_data.buffer_info.buffer = p_vkrs->uniform_data.buf;
  p_vkrs->uniform_data.buffer_info.offset = 0;
  p_vkrs->uniform_data.buffer_info.range = sizeof(p_vkrs->MVP);

  return res;
}

VkResult mvk_init_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs, bool use_texture,
                                                  VkDescriptorSetLayoutCreateFlags descSetLayoutCreateFlags)
{
  VkDescriptorSetLayoutBinding layout_bindings[2];
  layout_bindings[0].binding = 0;
  layout_bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  layout_bindings[0].descriptorCount = 1;
  layout_bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
  layout_bindings[0].pImmutableSamplers = NULL;

  if (use_texture)
  {
    layout_bindings[1].binding = 1;
    layout_bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_bindings[1].descriptorCount = 1;
    layout_bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_bindings[1].pImmutableSamplers = NULL;
  }

  /* Next take layout bindings and use them to create a descriptor set layout
     */
  VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
  descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptor_layout.pNext = NULL;
  descriptor_layout.flags = descSetLayoutCreateFlags;
  descriptor_layout.bindingCount = use_texture ? 2 : 1;
  descriptor_layout.pBindings = layout_bindings;

  VkResult res;

  p_vkrs->desc_layout.resize(NUM_DESCRIPTOR_SETS);
  res = vkCreateDescriptorSetLayout(p_vkrs->device, &descriptor_layout, NULL, p_vkrs->desc_layout.data());
  assert(res == VK_SUCCESS);

  /* Now use the descriptor layout to create a pipeline layout */
  VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
  pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pPipelineLayoutCreateInfo.pNext = NULL;
  pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
  pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
  pPipelineLayoutCreateInfo.setLayoutCount = NUM_DESCRIPTOR_SETS;
  pPipelineLayoutCreateInfo.pSetLayouts = p_vkrs->desc_layout.data();

  res = vkCreatePipelineLayout(p_vkrs->device, &pPipelineLayoutCreateInfo, NULL, &p_vkrs->pipeline_layout);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_renderpass(vk_render_state *p_vkrs, bool include_depth, bool clear, VkImageLayout finalLayout,
                             VkImageLayout initialLayout)
{
  /* DEPENDS on init_swap_chain() and init_depth_buffer() */

  assert(clear || (initialLayout != VK_IMAGE_LAYOUT_UNDEFINED));

  VkResult res;
  /* Need attachments for render target and depth buffer */
  VkAttachmentDescription attachments[2];
  attachments[0].format = p_vkrs->format;
  attachments[0].samples = NUM_SAMPLES;
  attachments[0].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = initialLayout;
  attachments[0].finalLayout = finalLayout;
  attachments[0].flags = 0;

  if (include_depth)
  {
    attachments[1].format = p_vkrs->depth.format;
    attachments[1].samples = NUM_SAMPLES;
    attachments[1].loadOp = clear ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].flags = 0;
  }

  VkAttachmentReference color_reference = {};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkAttachmentReference depth_reference = {};
  depth_reference.attachment = 1;
  depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.flags = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = include_depth ? &depth_reference : NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  // Subpass dependency to wait for wsi image acquired semaphore before starting layout transition
  VkSubpassDependency subpass_dependency = {};
  subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependency.dstSubpass = 0;
  subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependency.srcAccessMask = 0;
  subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpass_dependency.dependencyFlags = 0;

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.pNext = NULL;
  rp_info.attachmentCount = include_depth ? 2 : 1;
  rp_info.pAttachments = attachments;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 1;
  rp_info.pDependencies = &subpass_dependency;

  res = vkCreateRenderPass(p_vkrs->device, &rp_info, NULL, &p_vkrs->render_pass);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_command_pool(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_swapchain_extension() */
  VkResult res;

  VkCommandPoolCreateInfo cmd_pool_info = {};
  cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  cmd_pool_info.pNext = NULL;
  cmd_pool_info.queueFamilyIndex = p_vkrs->graphics_queue_family_index;
  cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  res = vkCreateCommandPool(p_vkrs->device, &cmd_pool_info, NULL, &p_vkrs->cmd_pool);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_command_buffer(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_swapchain_extension() and init_command_pool() */
  VkResult res;

  VkCommandBufferAllocateInfo cmd = {};
  cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd.pNext = NULL;
  cmd.commandPool = p_vkrs->cmd_pool;
  cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd.commandBufferCount = 1;

  res = vkAllocateCommandBuffers(p_vkrs->device, &cmd, &p_vkrs->cmd);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_execute_begin_command_buffer(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_command_buffer() */
  VkResult res;

  VkCommandBufferBeginInfo cmd_buf_info = {};
  cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  cmd_buf_info.pNext = NULL;
  cmd_buf_info.flags = 0;
  cmd_buf_info.pInheritanceInfo = NULL;

  res = vkBeginCommandBuffer(p_vkrs->cmd, &cmd_buf_info);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_execute_end_command_buffer(vk_render_state *p_vkrs)
{
  VkResult res;

  res = vkEndCommandBuffer(p_vkrs->cmd);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_execute_queue_command_buffer(vk_render_state *p_vkrs)
{
  VkResult res;

  /* Queue the command buffer for execution */
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
  submit_info[0].waitSemaphoreCount = 0;
  submit_info[0].pWaitSemaphores = NULL;
  submit_info[0].pWaitDstStageMask = &pipe_stage_flags;
  submit_info[0].commandBufferCount = 1;
  submit_info[0].pCommandBuffers = cmd_bufs;
  submit_info[0].signalSemaphoreCount = 0;
  submit_info[0].pSignalSemaphores = NULL;

  res = vkQueueSubmit(p_vkrs->graphics_queue, 1, submit_info, drawFence);
  assert(res == VK_SUCCESS);

  do
  {
    res = vkWaitForFences(p_vkrs->device, 1, &drawFence, VK_TRUE, FENCE_TIMEOUT);
  } while (res == VK_TIMEOUT);
  assert(res == VK_SUCCESS);

  vkDestroyFence(p_vkrs->device, drawFence, NULL);
  return res;
}

void mvk_init_device_queue(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_swapchain_extension() */

  vkGetDeviceQueue(p_vkrs->device, p_vkrs->graphics_queue_family_index, 0, &p_vkrs->graphics_queue);
  if (p_vkrs->graphics_queue_family_index == p_vkrs->present_queue_family_index)
  {
    p_vkrs->present_queue = p_vkrs->graphics_queue;
  }
  else
  {
    vkGetDeviceQueue(p_vkrs->device, p_vkrs->present_queue_family_index, 0, &p_vkrs->present_queue);
  }
}

VkResult GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *p_shader_text, std::vector<unsigned int> &spirv)
{
  // Use glslangValidator from file
  // Generate the shader file
  const char *ext = NULL;
  if ((shader_type & VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT) == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT)
  {
    ext = "vert";
  } else if ((shader_type & VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT) == VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT)
  {
    ext = "frag";
  }
  else
  {
    printf("GLSLtoSPV: unhandled bit type: %i", shader_type);
    return VK_ERROR_UNKNOWN;
  }

  const char *wd = "/home/jason/midge/dep/glslang/bin/";
  char execOutput[1024];
  strcpy(execOutput, wd);
  strcat(execOutput, "shader.");
  strcat(execOutput, ext);

  FILE *fp;
  fp = fopen(execOutput, "w");
  if (!fp)
  {
    printf("GLSLtoSPV: couldn't open file: %s", execOutput);
    return VK_ERROR_UNKNOWN;
  }

  fprintf(fp, "%s", p_shader_text);
  fclose(fp);
  // printf("%s written\n", shaderFile);

  // Execute the SPIRV gen
  char *exePath = strdup("/home/jason/midge/dep/glslang/bin/glslangValidator");
  char *arg_V = strdup("-V");
  char *arg_H = strdup("-H");
  char *argv[4]{
      exePath,
      execOutput,
      arg_V,
      // arg_H,
      NULL};
  int child_status;
  pid_t child_pid = fork();
  if (child_pid == 0)
  {
    /* This is done by the child process. */
    execv(argv[0], argv);

    printf("GLSLtoSPV: error occured during conversion.\n");
    perror("execv");
    return VK_ERROR_UNKNOWN;
  }
  else
  {
    pid_t tpid;
    do
    {
      tpid = wait(&child_status);
      if (tpid != child_pid)
        return (VkResult)child_status;
      //process_terminated(tpid);
    } while (tpid != child_pid);
  }

  // Load the generated file
  char shaderFile[512];
  strcpy(shaderFile, ext);
  strcat(shaderFile, ".spv");

  fp = fopen(shaderFile, "r");
  if (!fp)
  {
    printf("GLSLtoSPV: couldn't open file: %s", shaderFile);
    return VK_ERROR_UNKNOWN;
  }

  uint32_t code;
  while (fread(&code, sizeof code, 1, fp) == 1)
  {
    spirv.push_back(code);
  }
  fclose(fp);
  remove(execOutput);
  remove(shaderFile);
  printf("->%s: sizeof=4*%zu\n", shaderFile, spirv.size());

  // TEST READ
  // fp = fopen("/home/jason/midge/test.spv", "w");
  // if (!fp)
  // {
  //   printf("GLSLtoSPV: couldn't open file: %s", "/home/jason/midge/test.spv");
  //   return VK_ERROR_UNKNOWN;
  // }
  // // printf("test.spv opened: ");
  // for (int i = 0; i < spirv.size(); ++i)
  // {
  //   // printf("%i", spirv[i]);
  //   fwrite(&spirv[i], sizeof(uint32_t), 1, fp);
  // }
  // fclose(fp);

  return VK_SUCCESS;
}

VkResult mvk_init_shader(vk_render_state *p_vkrs, struct glsl_shader *glsl_shader, int stage_index)
{
  VkShaderModuleCreateInfo moduleCreateInfo;
  std::vector<unsigned int> vtx_spv;
  p_vkrs->shaderStages[stage_index].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  p_vkrs->shaderStages[stage_index].pNext = NULL;
  p_vkrs->shaderStages[stage_index].pSpecializationInfo = NULL;
  p_vkrs->shaderStages[stage_index].flags = 0;
  p_vkrs->shaderStages[stage_index].stage = glsl_shader->stage; // VK_SHADER_STAGE_VERTEX_BIT; VK_SHADER_STAGE_FRAGMENT_BIT
  p_vkrs->shaderStages[stage_index].pName = "main";

  VkResult res = GLSLtoSPV(glsl_shader->stage, glsl_shader->text, vtx_spv);
  assert(res == VK_SUCCESS);

  moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  moduleCreateInfo.pNext = NULL;
  moduleCreateInfo.flags = 0;
  moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
  moduleCreateInfo.pCode = vtx_spv.data();

  res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &p_vkrs->shaderStages[stage_index].module);
  assert(res == VK_SUCCESS);

  return res;
}

void mvk_destroy_shaders(vk_render_state *p_vkrs)
{
  vkDestroyShaderModule(p_vkrs->device, p_vkrs->shaderStages[0].module, NULL);
  vkDestroyShaderModule(p_vkrs->device, p_vkrs->shaderStages[1].module, NULL);
}

void mvk_destroy_uniform_buffer(vk_render_state *p_vkrs)
{
  vkDestroyBuffer(p_vkrs->device, p_vkrs->uniform_data.buf, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->uniform_data.mem, NULL);
}

void mvk_destroy_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs)
{
  for (int i = 0; i < NUM_DESCRIPTOR_SETS; i++)
    vkDestroyDescriptorSetLayout(p_vkrs->device, p_vkrs->desc_layout[i], NULL);
  vkDestroyPipelineLayout(p_vkrs->device, p_vkrs->pipeline_layout, NULL);
}

void mvk_destroy_command_buffer(vk_render_state *p_vkrs)
{
  VkCommandBuffer cmd_bufs[1] = {p_vkrs->cmd};
  vkFreeCommandBuffers(p_vkrs->device, p_vkrs->cmd_pool, 1, cmd_bufs);
}

void mvk_destroy_command_pool(vk_render_state *p_vkrs)
{
  vkDestroyCommandPool(p_vkrs->device, p_vkrs->cmd_pool, NULL);
}

void mvk_destroy_depth_buffer(vk_render_state *p_vkrs)
{
  vkDestroyImageView(p_vkrs->device, p_vkrs->depth.view, NULL);
  vkDestroyImage(p_vkrs->device, p_vkrs->depth.image, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->depth.mem, NULL);
}

void mvk_destroy_swap_chain(vk_render_state *p_vkrs)
{
  for (uint32_t i = 0; i < p_vkrs->swapchainImageCount; i++)
  {
    vkDestroyImageView(p_vkrs->device, p_vkrs->buffers[i].view, NULL);
  }
  vkDestroySwapchainKHR(p_vkrs->device, p_vkrs->swap_chain, NULL);
}

void mvk_destroy_renderpass(vk_render_state *p_vkrs) { vkDestroyRenderPass(p_vkrs->device, p_vkrs->render_pass, NULL); }

void mvk_destroy_device(vk_render_state *p_vkrs)
{
  vkDeviceWaitIdle(p_vkrs->device);
  vkDestroyDevice(p_vkrs->device, NULL);
}

void mvk_destroy_instance(vk_render_state *p_vkrs)
{
  vkDestroyInstance(p_vkrs->inst, NULL);
}