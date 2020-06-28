/* mvk_init_util.c */

#include "rendering/mvk_init_util.h"

/*
 * Obtains the memory type from the available properties, returning false if no memory type was matched.
 */
bool get_memory_type_index_from_properties(vk_render_state *p_vkrs, uint32_t typeBits, VkFlags requirements_mask,
                                           uint32_t *typeIndex)
{
  // Search memtypes to find first index with those properties
  for (uint32_t i = 0; i < p_vkrs->memory_properties.memoryTypeCount; i++) {
    if ((typeBits & 1) == 1) {
      // Type is available, does it match user properties?
      if ((p_vkrs->memory_properties.memoryTypes[i].propertyFlags & requirements_mask) == requirements_mask) {
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

  do {
    res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, NULL);
    if (res)
      return res;

    if (instance_extension_count == 0) {
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
 *
      "create function mvk_init_global_layer_properties|"
      "@create_function_name|"
      "VkResult|"
      "void_collection *|"
      "vk_layers|"
      "finish|"
      "@create_function_name|"
      "dcl uint32_t instance_layer_count\n"
      "dcs VkLayerProperties * vk_props NULL\n"
      "dcl VkResult res\n"
      "whl 1\n"
      "nva res vkEnumerateInstanceLayerProperties &instance_layer_count NULL\n"
      "ifs res\n"
      "ret res\n"
      "end\n"
      "ifs instance_layer_count == 0\n"
      "ret VK_SUCCESS\n"
      "end\n"
      "nva vk_props (VkLayerProperties *) realloc vk_props 'instance_layer_count * sizeof(VkLayerProperties)'\n"
      "nva res vkEnumerateInstanceLayerProperties &instance_layer_count vk_props\n"
      "ifs res != VK_INCOMPLETE\n"
      "brk\n"
      "end\n"
      "end\n"
      "for i 0 instance_layer_count\n"
      "dcl layer_properties layer_props\n"
      "ass layer_props.properties vk_props[i]\n"
      "nva res mvk_init_global_extensions_properties\n"
      "TODO"
      "end for\n"
      "|"
 */
VkResult mvk_init_global_layer_properties(vk_render_state *p_vkrs)
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
  do {
    res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
    if (res)
      return res;

    if (instance_layer_count == 0) {
      return VK_SUCCESS;
    }

    vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

    res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
  } while (res == VK_INCOMPLETE);

  /*
   * Now gather the extension list for each instance layer.
   */
  for (uint32_t i = 0; i < instance_layer_count; i++) {
    layer_properties layer_props;
    layer_props.properties = vk_props[i];
    // printf("vk_props[%i]:%s-%s\n", i, vk_props[i].layerName, vk_props[i].description);
    res = mvk_init_global_extension_properties(layer_props);
    if (res)
      return res;
    p_vkrs->instance_layer_properties.push_back(layer_props);
  }
  free(vk_props);

  return res;
}

int mvk_checkLayerSupport(vk_render_state *p_vkrs)
{
  for (const char *layerName : p_vkrs->instance_layer_names) {
    bool layerFound = false;

    for (const auto &layerProperties : p_vkrs->instance_layer_properties) {
      if (strcmp(layerName, layerProperties.properties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) {
      printf("Could not find layer name='%s' in available layers!\n", layerName);
      return 124;
    }
  }
  return 0;
}

#define func void
VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT obj_type,
                                                   uint64_t src_obj, size_t location, int32_t msg_code, const char *layerPrefix,
                                                   const char *msg, void *user_data)
{
  printf("\n");
  printf("VKDBG: ");
  if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
    printf("INFO: ");
  }
  if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
    printf("WARNING: ");
  }
  if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
    printf("PERFORMANCE: ");
  }
  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
    printf("ERROR: ");
  }
  if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
    printf("DEBUG: ");
  }
  printf("@[%s]: ", layerPrefix);
  printf("%s\n", msg);

  return false;
}

// void mvk_setupDebug(vk_render_state *p_vkrs, VkDebugReportCallbackEXT *debugReport,
//                     VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo)
// {
//   debugCallbackCreateInfo->sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
//   debugCallbackCreateInfo->pfnCallback = &VulkanDebugCallback;
//   debugCallbackCreateInfo->flags = VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT |
//                                    VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_ERROR_BIT_EXT |
//                                    VK_DEBUG_REPORT_DEBUG_BIT_EXT | 0;
//   debugCallbackCreateInfo->pUserData = nullptr;
//   debugCallbackCreateInfo->pNext = nullptr;

/* Register the callback */
// VkDebugReportCallbackEXT callback;
// VkResult result = vkCreateDebugReportCallbackEXT(instance, &callbackCreateInfo, nullptr, &callback);

// p_vkrs->instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
/*
//	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_threading" );
      vulkanInstanceLayers.push_back( "VK_LAYER_GOOGLE_threading" );
      vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_draw_state" );
      vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_image" );
      vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
      vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_object_tracker" );
      vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_param_checker" );
      */

//	_device_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );				// depricated
/*
//	_device_layers.push_back( "VK_LAYER_LUNARG_threading" );
      _device_layers.push_back( "VK_LAYER_GOOGLE_threading" );
      _device_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
      _device_layers.push_back( "VK_LAYER_LUNARG_image" );
      _device_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
      _device_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
      _device_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
      */
// /* Load VK_EXT_debug_report entry points in debug builds */
// PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
//     reinterpret_cast(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
// PFN_vkDebugReportMessageEXT vkDebugReportMessageEXT =
//     reinterpret_cast(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));
// PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT =
//     reinterpret_cast(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
// }

func mvk_init_instance(VkResult *res, vk_render_state *p_vkrs, char const *const app_short_name)
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
  p_vkrs->instance_layer_names.push_back("VK_LAYER_KHRONOS_validation");
  // p_vkrs->instance_extension_names.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  // p_vkrs->instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

  mvk_checkLayerSupport(p_vkrs);

  // -- Debug --
  // VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
  // VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
  // mvk_setupDebug(p_vkrs, &debugReport, &debugCallbackCreateInfo);

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = p_vkrs->instance_layer_names.size();
  inst_info.ppEnabledLayerNames = p_vkrs->instance_layer_names.size() ? p_vkrs->instance_layer_names.data() : NULL;
  inst_info.enabledExtensionCount = p_vkrs->instance_extension_names.size();
  inst_info.ppEnabledExtensionNames = p_vkrs->instance_extension_names.data();
  // inst_info.pNext = (const void *)debugCallbackCreateInfo;

  printf("create VkInstance...");
  *res = vkCreateInstance(&inst_info, NULL, &p_vkrs->inst);
  assert(*res == VK_SUCCESS);
  printf("SUCCESS\n");

  // mvk_setupDebug(p_vkrs);
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

  do {
    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &device_extension_count, NULL);
    if (res)
      return res;

    if (device_extension_count == 0) {
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

  VkPhysicalDeviceFeatures deviceFeatures{};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = p_vkrs->device_extension_names.size();
  device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? p_vkrs->device_extension_names.data() : NULL;
  device_info.pEnabledFeatures = &deviceFeatures;

  res = vkCreateDevice(p_vkrs->gpus[0], &device_info, NULL, &p_vkrs->device);
  assert(res == VK_SUCCESS);

  return res;
}

/*
 * Enumerates through the available graphics devices.
 */
VkResult mvk_init_enumerate_device(vk_render_state *p_vkrs)
{
  uint32_t deviceCount = 0;
  VkResult res = vkEnumeratePhysicalDevices(p_vkrs->inst, &deviceCount, nullptr);
  assert(res == VK_SUCCESS);

  assert(deviceCount > 0 && "Must have at least one physical device that supports Vulkan!");

  // VkPhysicalDevice device

  p_vkrs->gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * deviceCount);

  res = vkEnumeratePhysicalDevices(p_vkrs->inst, &deviceCount, p_vkrs->gpus);
  assert(res == VK_SUCCESS);

  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, NULL);
  assert(p_vkrs->queue_family_count >= 1);

  p_vkrs->queue_props.resize(p_vkrs->queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, p_vkrs->queue_props.data());
  assert(p_vkrs->queue_family_count >= 1);

  /* This is as good a place as any to do this */
  vkGetPhysicalDeviceMemoryProperties(p_vkrs->gpus[0], &p_vkrs->memory_properties);
  vkGetPhysicalDeviceProperties(p_vkrs->gpus[0], &p_vkrs->gpu_props);
  /* query device extensions for enabled layers */
  for (auto &layer_props : p_vkrs->instance_layer_properties) {
    init_device_extension_properties(p_vkrs, layer_props);
  }

  return res;
}

// VkResult mvk_init_depth_buffer(vk_render_state *p_vkrs)
// {
//   // printf("mvk_init_depth_buffer\n");
//   VkResult res;
//   bool pass;
//   VkImageCreateInfo image_info = {};
//   VkFormatProperties props;

//   /* allow custom depth formats */
//   if (p_vkrs->depth.format == VK_FORMAT_UNDEFINED)
//     p_vkrs->depth.format = VK_FORMAT_D16_UNORM;

//   const VkFormat depth_format = p_vkrs->depth.format;
//   vkGetPhysicalDeviceFormatProperties(p_vkrs->gpus[0], depth_format, &props);
//   if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
//     image_info.tiling = VK_IMAGE_TILING_LINEAR;
//   }
//   else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
//     image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
//   }
//   else {
//     /* Try other depth formats? */
//     printf("depth_format:%i is unsupported\n", depth_format);
//     return (VkResult)MVK_ERROR_UNSUPPORTED_DEPTH_FORMAT;
//   }

//   image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//   image_info.pNext = NULL;
//   image_info.imageType = VK_IMAGE_TYPE_2D;
//   image_info.format = depth_format;
//   image_info.extent.width = p_vkrs->window_width;
//   image_info.extent.height = p_vkrs->window_height;
//   image_info.extent.depth = 1;
//   image_info.mipLevels = 1;
//   image_info.arrayLayers = 1;
//   image_info.samples = NUM_SAMPLES;
//   image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//   image_info.queueFamilyIndexCount = 0;
//   image_info.pQueueFamilyIndices = NULL;
//   image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//   image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
//   image_info.flags = 0;

//   VkMemoryAllocateInfo mem_alloc = {};
//   mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//   mem_alloc.pNext = NULL;
//   mem_alloc.allocationSize = 0;
//   mem_alloc.memoryTypeIndex = 0;

//   VkImageViewCreateInfo view_info = {};
//   view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
//   view_info.pNext = NULL;
//   view_info.image = VK_NULL_HANDLE;
//   view_info.format = depth_format;
//   view_info.components.r = VK_COMPONENT_SWIZZLE_R;
//   view_info.components.g = VK_COMPONENT_SWIZZLE_G;
//   view_info.components.b = VK_COMPONENT_SWIZZLE_B;
//   view_info.components.a = VK_COMPONENT_SWIZZLE_A;
//   view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//   view_info.subresourceRange.baseMipLevel = 0;
//   view_info.subresourceRange.levelCount = 1;
//   view_info.subresourceRange.baseArrayLayer = 0;
//   view_info.subresourceRange.layerCount = 1;
//   view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
//   view_info.flags = 0;

//   if (depth_format == VK_FORMAT_D16_UNORM_S8_UINT || depth_format == VK_FORMAT_D24_UNORM_S8_UINT ||
//       depth_format == VK_FORMAT_D32_SFLOAT_S8_UINT) {
//     view_info.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//   }

//   VkMemoryRequirements mem_reqs;

//   /* Create image */
//   res = vkCreateImage(p_vkrs->device, &image_info, NULL, &p_vkrs->depth.image);
//   assert(res == VK_SUCCESS);

//   vkGetImageMemoryRequirements(p_vkrs->device, p_vkrs->depth.image, &mem_reqs);

//   mem_alloc.allocationSize = mem_reqs.size;
//   /* Use the memory properties to determine the type of memory required */
//   pass = get_memory_type_index_from_properties(p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//                                                &mem_alloc.memoryTypeIndex);
//   assert(pass);

//   /* Allocate memory */
//   res = vkAllocateMemory(p_vkrs->device, &mem_alloc, NULL, &p_vkrs->depth.mem);
//   assert(res == VK_SUCCESS);

//   /* Bind memory */
//   res = vkBindImageMemory(p_vkrs->device, p_vkrs->depth.image, p_vkrs->depth.mem, 0);
//   assert(res == VK_SUCCESS);

//   /* Create image view */
//   view_info.image = p_vkrs->depth.image;
//   res = vkCreateImageView(p_vkrs->device, &view_info, NULL, &p_vkrs->depth.view);
//   assert(res == VK_SUCCESS);

//   return res;
// }

uint32_t getMemoryTypeIndex(vk_render_state *p_vkrs, uint32_t typeBits, VkMemoryPropertyFlags properties)
{
  VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
  vkGetPhysicalDeviceMemoryProperties(p_vkrs->gpus[0], &deviceMemoryProperties);
  for (uint32_t i = 0; i < deviceMemoryProperties.memoryTypeCount; i++) {
    if ((typeBits & 1) == 1) {
      if ((deviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
        return i;
      }
    }
    typeBits >>= 1;
  }
  return 0;
}

VkResult mvk_init_headless_image(vk_render_state *p_vkrs)
{
  /* allow custom depth formats */
  VkFormat colorFormat = p_vkrs->format;

  // Color attachment
  VkImageCreateInfo imageCreateInfo = {};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = colorFormat;
  imageCreateInfo.extent.width = p_vkrs->maximal_image_width;
  imageCreateInfo.extent.height = p_vkrs->maximal_image_height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

  // VkFormatProperties props;
  // vkGetPhysicalDeviceFormatProperties(p_vkrs->gpus[0], p_vkrs->depth.format, &props);
  // if (props.linearTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
  //   imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
  // }
  // else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
  //   imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  // }
  // else {
  //   /* Try other depth formats? */
  //   printf("depth_format:%i is unsupported\n", p_vkrs->depth.format);
  //   return (VkResult)MVK_ERROR_UNSUPPORTED_DEPTH_FORMAT;
  // }

  VkResult res = vkCreateImage(p_vkrs->device, &imageCreateInfo, nullptr, &p_vkrs->headless.image);
  assert(res == VK_SUCCESS);

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(p_vkrs->device, p_vkrs->headless.image, &memReqs);

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex = getMemoryTypeIndex(p_vkrs, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  res = vkAllocateMemory(p_vkrs->device, &memAlloc, nullptr, &p_vkrs->headless.memory);
  assert(res == VK_SUCCESS);
  res = vkBindImageMemory(p_vkrs->device, p_vkrs->headless.image, p_vkrs->headless.memory, 0);
  assert(res == VK_SUCCESS);

  VkImageViewCreateInfo colorImageView = {};
  colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
  colorImageView.format = colorFormat;
  colorImageView.subresourceRange = {};
  colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
  colorImageView.subresourceRange.baseMipLevel = 0;
  colorImageView.subresourceRange.levelCount = 1;
  colorImageView.subresourceRange.baseArrayLayer = 0;
  colorImageView.subresourceRange.layerCount = 1;
  colorImageView.image = p_vkrs->headless.image;
  res = vkCreateImageView(p_vkrs->device, &colorImageView, nullptr, &p_vkrs->headless.view);
  assert(res == VK_SUCCESS);

  return res;
}

void mvk_destroy_headless_image(vk_render_state *p_vkrs)
{

  vkDestroyImageView(p_vkrs->device, p_vkrs->headless.view, NULL);
  vkDestroyImage(p_vkrs->device, p_vkrs->headless.image, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->headless.memory, NULL);
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
  for (uint32_t i = 0; i < p_vkrs->queue_family_count; i++) {
    vkGetPhysicalDeviceSurfaceSupportKHR(p_vkrs->gpus[0], i, p_vkrs->surface, &pSupportsPresent[i]);
  }

  // Search for a graphics and a present queue in the array of queue
  // families, try to find one that supports both
  p_vkrs->graphics_queue_family_index = UINT32_MAX;
  p_vkrs->present_queue_family_index = UINT32_MAX;
  for (uint32_t i = 0; i < p_vkrs->queue_family_count; ++i) {
    if ((p_vkrs->queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
      if (p_vkrs->graphics_queue_family_index == UINT32_MAX)
        p_vkrs->graphics_queue_family_index = i;

      if (pSupportsPresent[i] == VK_TRUE) {
        p_vkrs->graphics_queue_family_index = i;
        p_vkrs->present_queue_family_index = i;
        break;
      }
    }
  }

  if (p_vkrs->present_queue_family_index == UINT32_MAX) {
    // If didn't find a queue that supports both graphics and present, then
    // find a separate present queue.
    for (size_t i = 0; i < p_vkrs->queue_family_count; ++i)
      if (pSupportsPresent[i] == VK_TRUE) {
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
  if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
    p_vkrs->format = VK_FORMAT_R8G8B8A8_SRGB;
  }
  else {
    assert(formatCount >= 1);
    p_vkrs->format = surfFormats[0].format;
  }
  printf("swapchain format = %i\n", p_vkrs->format);
  free(surfFormats);

  return VK_SUCCESS;
}

/*
 * Initializes the swap chain and the images it uses.
 */
VkResult mvk_init_swapchain(vk_render_state *p_vkrs)
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
  if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
    // If the surface size is undefined, the size is set to
    // the size of the images requested.
    swapchainExtent.width = p_vkrs->window_width;
    swapchainExtent.height = p_vkrs->window_height;
    if (swapchainExtent.width < surfCapabilities.minImageExtent.width) {
      swapchainExtent.width = surfCapabilities.minImageExtent.width;
    }
    else if (swapchainExtent.width > surfCapabilities.maxImageExtent.width) {
      swapchainExtent.width = surfCapabilities.maxImageExtent.width;
    }

    if (swapchainExtent.height < surfCapabilities.minImageExtent.height) {
      swapchainExtent.height = surfCapabilities.minImageExtent.height;
    }
    else if (swapchainExtent.height > surfCapabilities.maxImageExtent.height) {
      swapchainExtent.height = surfCapabilities.maxImageExtent.height;
    }
  }
  else {
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
  if (surfCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
    preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
  }
  else {
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
  for (uint32_t i = 0; i < sizeof(compositeAlphaFlags) / sizeof(compositeAlphaFlags[0]); i++) {
    if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
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
  swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
  swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
  swapchain_ci.queueFamilyIndexCount = 0;
  swapchain_ci.pQueueFamilyIndices = NULL;
  uint32_t queueFamilyIndices[2] = {(uint32_t)p_vkrs->graphics_queue_family_index, (uint32_t)p_vkrs->present_queue_family_index};
  if (p_vkrs->graphics_queue_family_index != p_vkrs->present_queue_family_index) {
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

  for (uint32_t i = 0; i < p_vkrs->swapchainImageCount; i++) {
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

  if (NULL != presentModes) {
    free(presentModes);
  }

  return VK_SUCCESS;
}

VkResult mvk_init_uniform_buffer(vk_render_state *p_vkrs)
{
  VkResult res;
  bool pass;

  float fov = glm_rad(45.0f);

  if (p_vkrs->window_width > p_vkrs->window_height) {
    fov *= static_cast<float>(p_vkrs->window_height) / static_cast<float>(p_vkrs->window_width);
  }

  glm_ortho_default((float)p_vkrs->window_width / p_vkrs->window_height, (vec4 *)&p_vkrs->Projection);
  // glm_perspective(fov, static_cast<float>(p_vkrs->window_width) / static_cast<float>(p_vkrs->window_height), 0.1f, 100.0f,
  //                 (vec4 *)&p_vkrs->Projection);
  glm_lookat((vec3){0, 0, -10}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)&p_vkrs->View);

  glm_mat4_copy(GLM_MAT4_IDENTITY, (vec4 *)&p_vkrs->Model);

  // Vulkan clip space has inverted Y and half Z.
  mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};
  glm_mat4_copy(clip, (vec4 *)&p_vkrs->Clip);

  glm_mat4_mul((vec4 *)&p_vkrs->View, (vec4 *)&p_vkrs->Model, (vec4 *)&p_vkrs->MVP);
  glm_mat4_mul((vec4 *)&p_vkrs->Projection, (vec4 *)&p_vkrs->MVP, (vec4 *)&p_vkrs->MVP);
  glm_mat4_mul((vec4 *)&p_vkrs->Clip, (vec4 *)&p_vkrs->MVP, (vec4 *)&p_vkrs->MVP);

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
  res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->global_vert_uniform_buffer.buf);
  assert(res == VK_SUCCESS);

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  pass = get_memory_type_index_from_properties(p_vkrs, mem_reqs.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               &alloc_info.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->global_vert_uniform_buffer.mem));
  assert(res == VK_SUCCESS);

  uint8_t *pData;
  res = vkMapMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem, 0, mem_reqs.size, 0, (void **)&pData);
  assert(res == VK_SUCCESS);

  memcpy(pData, &p_vkrs->MVP, sizeof(p_vkrs->MVP));

  vkUnmapMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem);

  res = vkBindBufferMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf, p_vkrs->global_vert_uniform_buffer.mem, 0);
  assert(res == VK_SUCCESS);

  p_vkrs->global_vert_uniform_buffer.buffer_info.buffer = p_vkrs->global_vert_uniform_buffer.buf;
  p_vkrs->global_vert_uniform_buffer.buffer_info.offset = 0;
  p_vkrs->global_vert_uniform_buffer.buffer_info.range = sizeof(p_vkrs->MVP);

  /* SHARED BUFFER */
  p_vkrs->render_data_buffer.allocated_size = 65536;
  buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  buf_info.size = p_vkrs->render_data_buffer.allocated_size;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->render_data_buffer.buffer);
  assert(res == VK_SUCCESS);

  vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->render_data_buffer.buffer, &mem_reqs);

  alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  pass = get_memory_type_index_from_properties(p_vkrs, mem_reqs.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               &alloc_info.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->render_data_buffer.memory));
  assert(res == VK_SUCCESS);

  res = vkBindBufferMemory(p_vkrs->device, p_vkrs->render_data_buffer.buffer, p_vkrs->render_data_buffer.memory, 0);
  assert(res == VK_SUCCESS);

  p_vkrs->render_data_buffer.frame_utilized_amount = 0;

  p_vkrs->render_data_buffer.queued_copies_alloc = 256U;
  p_vkrs->render_data_buffer.queued_copies_count = 0U;
  p_vkrs->render_data_buffer.queued_copies =
      (queued_copy_info *)malloc(sizeof(queued_copy_info) * p_vkrs->render_data_buffer.queued_copies_alloc);

  return res;
}

VkResult mvk_init_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs)
{
  VkResult res;

  // Render Vertex Color Primitives
  {
    VkDescriptorSetLayoutBinding layout_bindings[3];
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
    layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[2].descriptorCount = 1;
    layout_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_bindings[2].pImmutableSamplers = NULL;

    /* Next take layout bindings and use them to create a descriptor set layout
     */
    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.flags = 0;
    descriptor_layout.bindingCount = 3;
    descriptor_layout.pBindings = layout_bindings;

    p_vkrs->desc_layout.resize(1);
    res = vkCreateDescriptorSetLayout(p_vkrs->device, &descriptor_layout, NULL, p_vkrs->desc_layout.data());
    assert(res == VK_SUCCESS);

    /* Now use the descriptor layout to create a pipeline layout */
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pPipelineLayoutCreateInfo.pPushConstantRanges = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = p_vkrs->desc_layout.data();

    res = vkCreatePipelineLayout(p_vkrs->device, &pPipelineLayoutCreateInfo, NULL, &p_vkrs->pipeline_layout);
    assert(res == VK_SUCCESS);
  }

  return res;
}

VkResult mvk_init_textured_render_prog(vk_render_state *p_vkrs)
{
  VkResult res;

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

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = NULL;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = binding_count;
    layoutCreateInfo.pBindings = layout_bindings;

    p_vkrs->desc_layout.resize(1);
    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layoutCreateInfo, NULL, &p_vkrs->texture_prog.desc_layout);
    assert(res == VK_SUCCESS);
  }

  static glsl_shader texture_vertex_shader = {
      .text = "#version 450\n"
              "#extension GL_ARB_separate_shader_objects : enable\n"
              "\n"
              "layout (std140, binding = 0) uniform UBO0 {\n"
              "    mat4 mvp;\n"
              "} globalUI;\n"
              "layout (binding = 1) uniform UBO1 {\n"
              "    vec2 offset;\n"
              "    vec2 scale;\n"
              "} element;\n"
              "\n"
              "layout(location = 0) in vec2 inPosition;\n"
              "layout(location = 1) in vec2 inTexCoord;\n"
              "\n"
              "layout(location = 1) out vec2 fragTexCoord;\n"
              "\n"
              "void main() {\n"
              "   gl_Position = globalUI.mvp * vec4(inPosition, 0.0, 1.0);\n"
              "   gl_Position.xy *= element.scale.xy;\n"
              "   gl_Position.xy += element.offset.xy;\n"
              "   fragTexCoord = inTexCoord;\n"
              "}\n",
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
  };

  static glsl_shader texture_fragment_shader = {
      .text = "#version 450\n"
              "#extension GL_ARB_separate_shader_objects : enable\n"
              "\n"
              "layout(binding = 2) uniform sampler2D texSampler;\n"
              "\n"
              "layout(location = 1) in vec2 fragTexCoord;\n"
              "\n"
              "layout(location = 0) out vec4 outColor;\n"
              "\n"
              "void main() {\n"
              "\n"
              "   outColor = texture(texSampler, fragTexCoord);\n"
              "}\n",
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
    VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[0];
    shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStateCreateInfo->pNext = NULL;
    shaderStateCreateInfo->pSpecializationInfo = NULL;
    shaderStateCreateInfo->flags = 0;
    shaderStateCreateInfo->stage = texture_vertex_shader.stage;
    shaderStateCreateInfo->pName = "main";

    std::vector<unsigned int> vtx_spv;
    VkResult res = GLSLtoSPV(texture_vertex_shader.stage, texture_vertex_shader.text, vtx_spv);
    assert(res == VK_SUCCESS);

    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = vtx_spv.data();

    res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &shaderStateCreateInfo->module);
    assert(res == VK_SUCCESS);
  }
  {
    VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[1];
    shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStateCreateInfo->pNext = NULL;
    shaderStateCreateInfo->pSpecializationInfo = NULL;
    shaderStateCreateInfo->flags = 0;
    shaderStateCreateInfo->stage = texture_fragment_shader.stage;
    shaderStateCreateInfo->pName = "main";

    std::vector<unsigned int> vtx_spv;
    VkResult res = GLSLtoSPV(texture_fragment_shader.stage, texture_fragment_shader.text, vtx_spv);
    assert(res == VK_SUCCESS);

    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = vtx_spv.data();

    res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &shaderStateCreateInfo->module);
    assert(res == VK_SUCCESS);
  }

  // Vertex Bindings
  VkVertexInputBindingDescription bindingDescription{};
  const int VERTEX_ATTRIBUTE_COUNT = 2;
  VkVertexInputAttributeDescription attributeDescriptions[VERTEX_ATTRIBUTE_COUNT];
  {
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(textured_image_vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(textured_image_vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(textured_image_vertex, tex_coord);
  }

  {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = NULL;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = NULL;
    viewportState.flags = 0;
    viewportState.viewportCount = NUM_VIEWPORTS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    viewportState.scissorCount = NUM_SCISSORS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    viewportState.pScissors = NULL;
    viewportState.pViewports = NULL;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
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

    VkPipelineColorBlendStateCreateInfo colorBlending{};
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &p_vkrs->texture_prog.desc_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, nullptr, &p_vkrs->texture_prog.pipeline_layout);
    assert(res == VK_SUCCESS && "failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = p_vkrs->texture_prog.pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->present_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res = vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p_vkrs->texture_prog.pipeline);
    assert(res == VK_SUCCESS && "failed to create pipeline!");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, nullptr);
  }

  return VK_SUCCESS;
}

VkResult mvk_init_font_render_prog(vk_render_state *p_vkrs)
{
  VkResult res;

  // CreateDescriptorSetLayout
  {
    const int binding_count = 4;
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
    layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_bindings[2].descriptorCount = 1;
    layout_bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_bindings[2].pImmutableSamplers = NULL;

    layout_bindings[3].binding = 3;
    layout_bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_bindings[3].descriptorCount = 1;
    layout_bindings[3].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_bindings[3].pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo = {};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.pNext = NULL;
    layoutCreateInfo.flags = 0;
    layoutCreateInfo.bindingCount = binding_count;
    layoutCreateInfo.pBindings = layout_bindings;

    p_vkrs->desc_layout.resize(1);
    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layoutCreateInfo, NULL, &p_vkrs->font_prog.desc_layout);
    assert(res == VK_SUCCESS);
  }

  static glsl_shader vertex_shader = {
      .text = "#version 450\n"
              "#extension GL_ARB_separate_shader_objects : enable\n"
              "\n"
              "layout (std140, binding = 0) uniform UBO0 {\n"
              "    mat4 mvp;\n"
              "} globalUI;\n"
              "layout (binding = 1) uniform UBO1 {\n"
              "    vec2 offset;\n"
              "    vec2 scale;\n"
              "} element;\n"
              "\n"
              "layout(location = 0) in vec2 inPosition;\n"
              "layout(location = 1) in vec2 inTexCoord;\n"
              "\n"
              "layout(location = 1) out vec2 fragTexCoord;\n"
              "\n"
              "void main() {\n"
              "   gl_Position = globalUI.mvp * vec4(inPosition, 0.0, 1.0);\n"
              "   gl_Position.xy *= element.scale.xy;\n"
              "   gl_Position.xy += element.offset.xy;\n"
              "   fragTexCoord = inTexCoord;\n"
              "}\n",
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
  };

  static glsl_shader fragment_shader = {
      .text = "#version 450\n"
              "#extension GL_ARB_separate_shader_objects : enable\n"
              "\n"
              "layout (binding = 2) uniform UBO2 {\n"
              "    vec4 tint;\n"
              "    vec4 texCoordBounds;\n"
              "} element;\n"
              "\n"
              "layout(binding = 3) uniform sampler2D texSampler;\n"
              "\n"
              "layout(location = 1) in vec2 fragTexCoord;\n"
              "\n"
              "layout(location = 0) out vec4 outColor;\n"
              "\n"
              "void main() {\n"
              "\n"
              "   vec2 texCoords = vec2(\n"
              "       element.texCoordBounds.x + fragTexCoord.x * (element.texCoordBounds.y - element.texCoordBounds.x),\n"
              "       element.texCoordBounds.z + fragTexCoord.y * (element.texCoordBounds.w - element.texCoordBounds.z));\n"
              "   outColor = texture(texSampler, texCoords);\n"
              "   if(outColor.r < 0.01)\n"
              "      discard;\n"
              "   outColor.a = min(max(0, outColor.r - 0.2) * 0.2f + outColor.r * 1.5, 1.0);\n"
              "   outColor.rgb = element.tint.rgb;\n"
              "}\n",
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
  };

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
    VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[0];
    shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStateCreateInfo->pNext = NULL;
    shaderStateCreateInfo->pSpecializationInfo = NULL;
    shaderStateCreateInfo->flags = 0;
    shaderStateCreateInfo->stage = vertex_shader.stage;
    shaderStateCreateInfo->pName = "main";

    std::vector<unsigned int> vtx_spv;
    VkResult res = GLSLtoSPV(vertex_shader.stage, vertex_shader.text, vtx_spv);
    assert(res == VK_SUCCESS);

    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = vtx_spv.data();

    res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &shaderStateCreateInfo->module);
    assert(res == VK_SUCCESS);
  }
  {
    VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[1];
    shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStateCreateInfo->pNext = NULL;
    shaderStateCreateInfo->pSpecializationInfo = NULL;
    shaderStateCreateInfo->flags = 0;
    shaderStateCreateInfo->stage = fragment_shader.stage;
    shaderStateCreateInfo->pName = "main";

    std::vector<unsigned int> vtx_spv;
    VkResult res = GLSLtoSPV(fragment_shader.stage, fragment_shader.text, vtx_spv);
    assert(res == VK_SUCCESS);

    VkShaderModuleCreateInfo moduleCreateInfo;
    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.flags = 0;
    moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
    moduleCreateInfo.pCode = vtx_spv.data();

    res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &shaderStateCreateInfo->module);
    assert(res == VK_SUCCESS);
  }

  // Vertex Bindings
  VkVertexInputBindingDescription bindingDescription{};
  const int VERTEX_ATTRIBUTE_COUNT = 2;
  VkVertexInputAttributeDescription attributeDescriptions[VERTEX_ATTRIBUTE_COUNT];
  {
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(textured_image_vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(textured_image_vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(textured_image_vertex, tex_coord);
  }

  {
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = NULL;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
    VkPipelineDynamicStateCreateInfo dynamicState = {};
    memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.pNext = NULL;
    dynamicState.pDynamicStates = dynamicStateEnables;
    dynamicState.dynamicStateCount = 0;

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = NULL;
    viewportState.flags = 0;
    viewportState.viewportCount = NUM_VIEWPORTS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    viewportState.scissorCount = NUM_SCISSORS;
    dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
    viewportState.pScissors = NULL;
    viewportState.pViewports = NULL;

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState att_state[1];
    att_state[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT; // TODO
    att_state[0].blendEnable = VK_TRUE;
    att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
    // If this is true, then why is the render target blending when placed in another image
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
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

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &p_vkrs->font_prog.desc_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, nullptr, &p_vkrs->font_prog.pipeline_layout);
    assert(res == VK_SUCCESS && "failed to create pipeline layout!");

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = p_vkrs->font_prog.pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->present_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res = vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &p_vkrs->font_prog.pipeline);
    assert(res == VK_SUCCESS && "failed to create pipeline!");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, nullptr);
  }

  return VK_SUCCESS;
}

VkResult mvk_init_present_renderpass(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_swap_chain() and init_depth_buffer() */

  bool clear = true;

  VkResult res;
  /* Need attachments for render target and depth buffer */
  VkAttachmentDescription attachments[2];
  attachments[0].format = p_vkrs->format;
  attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
  attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  attachments[0].flags = 0;

  const bool include_depth = false;
  if (include_depth) {
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

  res = vkCreateRenderPass(p_vkrs->device, &rp_info, NULL, &p_vkrs->present_render_pass);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_offscreen_renderpass(vk_render_state *p_vkrs)
{
  VkResult res;
  /* Need attachments for render target and depth buffer */
  VkAttachmentDescription color_attachment;
  color_attachment.format = p_vkrs->format;
  color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
  color_attachment.flags = 0;

  VkAttachmentReference color_reference = {};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.flags = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = NULL;
  subpass.preserveAttachmentCount = 0;
  subpass.pPreserveAttachments = NULL;

  // Subpass dependency to wait for wsi image acquired semaphore before starting layout transition
  VkSubpassDependency subpass_dependencies[2];

  subpass_dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependencies[0].dstSubpass = 0;
  subpass_dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpass_dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpass_dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpass_dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  subpass_dependencies[1].srcSubpass = 0;
  subpass_dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
  subpass_dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  subpass_dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
  subpass_dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  subpass_dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
  subpass_dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.pNext = NULL;
  rp_info.attachmentCount = 1;
  rp_info.pAttachments = &color_attachment;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 2;
  rp_info.pDependencies = subpass_dependencies;

  res = vkCreateRenderPass(p_vkrs->device, &rp_info, NULL, &p_vkrs->offscreen_render_pass);
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
  cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

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

// VkResult mvk_execute_begin_command_buffer(vk_render_state *p_vkrs)
// {
//   /* DEPENDS on init_command_buffer() */
//   VkResult res;

//   VkCommandBufferBeginInfo cmd_buf_info = {};
//   cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//   cmd_buf_info.pNext = NULL;
//   cmd_buf_info.flags = 0;
//   cmd_buf_info.pInheritanceInfo = NULL;

//   res = vkBeginCommandBuffer(p_vkrs->cmd, &cmd_buf_info);
//   assert(res == VK_SUCCESS);
//   return res;
// }

// VkResult mvk_execute_end_command_buffer(vk_render_state *p_vkrs)
// {
//   VkResult res;

//   res = vkEndCommandBuffer(p_vkrs->cmd);
//   assert(res == VK_SUCCESS);
//   return res;
// }

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

  do {
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
  if (p_vkrs->graphics_queue_family_index == p_vkrs->present_queue_family_index) {
    p_vkrs->present_queue = p_vkrs->graphics_queue;
  }
  else {
    vkGetDeviceQueue(p_vkrs->device, p_vkrs->present_queue_family_index, 0, &p_vkrs->present_queue);
  }
}

VkResult GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *p_shader_text, std::vector<unsigned int> &spirv)
{
  // Use glslangValidator from file
  // Generate the shader file
  const char *ext = NULL;
  if ((shader_type & VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT) == VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT) {
    ext = "vert";
  }
  else if ((shader_type & VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT) ==
           VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT) {
    ext = "frag";
  }
  else {
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
  if (!fp) {
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
  char *argv[4]{exePath, execOutput, arg_V,
                // arg_H,
                NULL};
  int child_status;
  pid_t child_pid = fork();
  if (child_pid == 0) {
    /* This is done by the child process. */
    execv(argv[0], argv);

    printf("GLSLtoSPV: error occured during conversion.\n");
    perror("execv");
    return VK_ERROR_UNKNOWN;
  }
  else {
    pid_t tpid;
    do {
      tpid = wait(&child_status);
      if (tpid != child_pid)
        return (VkResult)child_status;
      // process_terminated(tpid);
    } while (tpid != child_pid);
  }

  // Load the generated file
  char shaderFile[512];
  strcpy(shaderFile, ext);
  strcat(shaderFile, ".spv");

  fp = fopen(shaderFile, "r");
  if (!fp) {
    printf("GLSLtoSPV: couldn't open file: %s", shaderFile);
    return VK_ERROR_UNKNOWN;
  }

  uint32_t code;
  while (fread(&code, sizeof code, 1, fp) == 1) {
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
  std::vector<unsigned int> vtx_spv;
  p_vkrs->shaderStages[stage_index].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
  p_vkrs->shaderStages[stage_index].pNext = NULL;
  p_vkrs->shaderStages[stage_index].pSpecializationInfo = NULL;
  p_vkrs->shaderStages[stage_index].flags = 0;
  p_vkrs->shaderStages[stage_index].stage = glsl_shader->stage; // VK_SHADER_STAGE_VERTEX_BIT; VK_SHADER_STAGE_FRAGMENT_BIT
  p_vkrs->shaderStages[stage_index].pName = "main";

  VkResult res = GLSLtoSPV(glsl_shader->stage, glsl_shader->text, vtx_spv);
  assert(res == VK_SUCCESS);

  VkShaderModuleCreateInfo moduleCreateInfo;
  moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
  moduleCreateInfo.pNext = NULL;
  moduleCreateInfo.flags = 0;
  moduleCreateInfo.codeSize = vtx_spv.size() * sizeof(unsigned int);
  moduleCreateInfo.pCode = vtx_spv.data();

  res = vkCreateShaderModule(p_vkrs->device, &moduleCreateInfo, NULL, &p_vkrs->shaderStages[stage_index].module);
  assert(res == VK_SUCCESS);

  return res;
}

VkResult mvk_init_framebuffers(vk_render_state *p_vkrs, bool include_depth)
{
  /* DEPENDS on init_depth_buffer(), init_renderpass() and
   * init_swapchain_extension() */

  VkResult res;
  VkImageView attachments[2];
  attachments[1] = p_vkrs->depth.view;

  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = p_vkrs->present_render_pass;
  fb_info.attachmentCount = include_depth ? 2 : 1;
  fb_info.pAttachments = attachments;
  fb_info.width = p_vkrs->window_width;
  fb_info.height = p_vkrs->window_height;
  fb_info.layers = 1;

  uint32_t i;

  p_vkrs->framebuffers = (VkFramebuffer *)malloc(p_vkrs->swapchainImageCount * sizeof(VkFramebuffer));

  for (i = 0; i < p_vkrs->swapchainImageCount; i++) {
    attachments[0] = p_vkrs->buffers[i].view;
    res = vkCreateFramebuffer(p_vkrs->device, &fb_info, NULL, &p_vkrs->framebuffers[i]);
    assert(res == VK_SUCCESS);
  }
  return res;
}

VkResult mvk_init_cube_vertices(vk_render_state *p_vkrs, const void *vertexData, uint32_t dataSize, uint32_t dataStride,
                                bool use_texture)
{
  VkResult res;
  bool pass;

  VkBufferCreateInfo buf_info = {};
  buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buf_info.pNext = NULL;
  buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
  buf_info.size = dataSize;
  buf_info.queueFamilyIndexCount = 0;
  buf_info.pQueueFamilyIndices = NULL;
  buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buf_info.flags = 0;
  res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->cube_vertices.buf);
  assert(res == VK_SUCCESS);

  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->cube_vertices.buf, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = mem_reqs.size;
  pass = get_memory_type_index_from_properties(p_vkrs, mem_reqs.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               &alloc_info.memoryTypeIndex);
  assert(pass && "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->cube_vertices.mem));
  assert(res == VK_SUCCESS);
  p_vkrs->cube_vertices.buffer_info.range = mem_reqs.size;
  p_vkrs->cube_vertices.buffer_info.offset = 0;

  uint8_t *pData;
  res = vkMapMemory(p_vkrs->device, p_vkrs->cube_vertices.mem, 0, mem_reqs.size, 0, (void **)&pData);
  assert(res == VK_SUCCESS);

  memcpy(pData, vertexData, dataSize);

  vkUnmapMemory(p_vkrs->device, p_vkrs->cube_vertices.mem);

  res = vkBindBufferMemory(p_vkrs->device, p_vkrs->cube_vertices.buf, p_vkrs->cube_vertices.mem, 0);
  assert(res == VK_SUCCESS);

  p_vkrs->cube_vertices.vi_desc = p_vkrs->pos_color_vertex_input_description;

  return res;
}

#define XY(X, Y) X, Y
#define UV(U, V) U, V
#define XYZW(X, Y, Z) X, Y, Z, 1.f
#define RGB(R, G, B) R, G, B
#define RGBA(R, G, B) R, G, B, 1.f
#define WHITE_RGBA 1.f, 1.f, 1.f, 1.f
#define WHITE_RGB 1.f, 1.f, 1.f
static const float g_vb_shape_data[] = { // Rectangle
    XYZW(-0.5f, -0.5f, 0), WHITE_RGBA, XYZW(-0.5f, 0.5f, 0), WHITE_RGBA, XYZW(0.5f, -0.5f, 0), WHITE_RGBA,
    XYZW(-0.5f, 0.5f, 0),  WHITE_RGBA, XYZW(0.5f, 0.5f, 0),  WHITE_RGBA, XYZW(0.5f, -0.5f, 0), WHITE_RGBA};
static const float g_vb_textured_shape_2D_data[] = { // Rectangle
    XY(-0.5f, -0.5f), UV(0.f, 0.f), XY(-0.5f, 0.5f), UV(0.f, 1.f), XY(0.5f, -0.5f), UV(1.f, 0.f),
    XY(-0.5f, 0.5f),  UV(0.f, 1.f), XY(0.5f, 0.5f),  UV(1.f, 1.f), XY(0.5f, -0.5f), UV(1.f, 0.f)};

VkResult mvk_init_shape_vertices(vk_render_state *p_vkrs)
{
  // const void *vertexData, uint32_t dataSize, uint32_t dataStride,
  //                               bool use_texture;

  VkResult res;
  bool pass;
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
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->shape_vertices.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    pass = get_memory_type_index_from_properties(p_vkrs, mem_reqs.memoryTypeBits,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                 &alloc_info.memoryTypeIndex);
    assert(pass && "No mappable, coherent memory");

    res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->shape_vertices.mem));
    assert(res == VK_SUCCESS);
    p_vkrs->shape_vertices.buffer_info.range = mem_reqs.size;
    p_vkrs->shape_vertices.buffer_info.offset = 0;

    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&pData);
    assert(res == VK_SUCCESS);

    memcpy(pData, g_vb_shape_data, data_size_in_bytes);

    vkUnmapMemory(p_vkrs->device, p_vkrs->shape_vertices.mem);

    res = vkBindBufferMemory(p_vkrs->device, p_vkrs->shape_vertices.buf, p_vkrs->shape_vertices.mem, 0);
    assert(res == VK_SUCCESS);

    p_vkrs->shape_vertices.vi_desc = p_vkrs->pos_color_vertex_input_description;
  }
  {
    // Shape Vertex-Colored Texture Data
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
    assert(res == VK_SUCCESS);

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->textured_shape_vertices.buf, &mem_reqs);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.memoryTypeIndex = 0;

    alloc_info.allocationSize = mem_reqs.size;
    pass = get_memory_type_index_from_properties(p_vkrs, mem_reqs.memoryTypeBits,
                                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                 &alloc_info.memoryTypeIndex);
    assert(pass && "No mappable, coherent memory");

    res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->textured_shape_vertices.mem));
    assert(res == VK_SUCCESS);
    p_vkrs->textured_shape_vertices.buffer_info.range = mem_reqs.size;
    p_vkrs->textured_shape_vertices.buffer_info.offset = 0;

    uint8_t *pData;
    res = vkMapMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem, 0, mem_reqs.size, 0, (void **)&pData);
    assert(res == VK_SUCCESS);

    memcpy(pData, g_vb_textured_shape_2D_data, data_size_in_bytes);

    vkUnmapMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.mem);

    res = vkBindBufferMemory(p_vkrs->device, p_vkrs->textured_shape_vertices.buf, p_vkrs->textured_shape_vertices.mem, 0);
    assert(res == VK_SUCCESS);
  }

  return res;
}

VkResult mvk_init_descriptor_pool(vk_render_state *p_vkrs, bool use_texture)
{
  /* DEPENDS on init_uniform_buffer() and
   * init_descriptor_and_pipeline_layouts() */

  VkResult res;

  const int DESCRIPTOR_POOL_COUNT = 2;
  VkDescriptorPoolSize type_count[DESCRIPTOR_POOL_COUNT];
  type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_count[0].descriptorCount = 256;
  type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  type_count[1].descriptorCount = 128;

  VkDescriptorPoolCreateInfo descriptor_pool = {};
  descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool.pNext = NULL;
  descriptor_pool.maxSets = 128;
  descriptor_pool.poolSizeCount = DESCRIPTOR_POOL_COUNT;
  descriptor_pool.pPoolSizes = type_count;

  res = vkCreateDescriptorPool(p_vkrs->device, &descriptor_pool, NULL, &p_vkrs->desc_pool);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_descriptor_set(vk_render_state *p_vkrs, bool use_texture)
{
  /* DEPENDS on init_descriptor_pool() */

  VkResult res = VK_SUCCESS;

  // VkDescriptorSetAllocateInfo alloc_info[1];
  // alloc_info[0].sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  // alloc_info[0].pNext = NULL;
  // alloc_info[0].descriptorPool = p_vkrs->desc_pool;
  // alloc_info[0].descriptorSetCount = MAX_DESCRIPTOR_SETS;
  // alloc_info[0].pSetLayouts = p_vkrs->desc_layout.data();

  p_vkrs->descriptor_sets_count = 0;

  // p_vkrs->descriptor_sets_allocated = 16;
  //   p_vkrs->descriptor_sets.resize(p_vkrs->descriptor_sets_allocated);
  //   res = vkAllocateDescriptorSets(p_vkrs->device, alloc_info, p_vkrs->desc_set.data());
  //   assert(res == VK_SUCCESS);

  // VkWriteDescriptorSet writes[3];

  // writes[0] = {};
  // writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  // writes[0].pNext = NULL;
  // writes[0].dstSet = p_vkrs->desc_set[0];
  // writes[0].descriptorCount = 1;
  // writes[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // writes[0].pBufferInfo = &p_vkrs->global_vert_uniform_buffer.buffer_info;
  // writes[0].dstArrayElement = 0;
  // writes[0].dstBinding = 0;

  // writes[1] = {};
  // writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  // writes[1].pNext = NULL;
  // writes[1].dstSet = p_vkrs->desc_set[0];
  // writes[1].descriptorCount = 1;
  // writes[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // writes[1].pBufferInfo = &p_vkrs->ui_element_data.buffer_info;
  // writes[1].dstArrayElement = 0;
  // writes[1].dstBinding = 1;

  // writes[2] = {};
  // writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  // writes[2].pNext = NULL;
  // writes[2].dstSet = p_vkrs->desc_set[0];
  // writes[2].descriptorCount = 1;
  // writes[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  // writes[2].pBufferInfo = &p_vkrs->ui_element_f.fragment_data.buffer_info;
  // writes[2].dstArrayElement = 0;
  // writes[2].dstBinding = 2;

  // if (use_texture) {
  //   writes[2] = {};
  //   writes[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  //   writes[2].dstSet = p_vkrs->desc_set[0];
  //   writes[2].descriptorCount = 1;
  //   writes[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  //   writes[2].pImageInfo = &p_vkrs->texture_data.image_info;
  //   writes[2].dstArrayElement = 0;
  //   writes[2].dstBinding = 2;
  // }

  // vkUpdateDescriptorSets(p_vkrs->device, 3, writes, 0, NULL);
  return res;
}

VkResult mvk_init_pipeline_cache(vk_render_state *p_vkrs)
{
  VkResult res;

  VkPipelineCacheCreateInfo pipelineCache;
  pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCache.pNext = NULL;
  pipelineCache.initialDataSize = 0;
  pipelineCache.pInitialData = NULL;
  pipelineCache.flags = 0;
  res = vkCreatePipelineCache(p_vkrs->device, &pipelineCache, NULL, &p_vkrs->pipelineCache);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_pipeline(vk_render_state *p_vkrs)
{
  VkResult res;

  p_vkrs->pos_color_vertex_input_description.binding.binding = 0;
  p_vkrs->pos_color_vertex_input_description.binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  p_vkrs->pos_color_vertex_input_description.binding.stride = 8 * sizeof(float);

  p_vkrs->pos_color_vertex_input_description.attribs[0].binding = 0;
  p_vkrs->pos_color_vertex_input_description.attribs[0].location = 0;
  p_vkrs->pos_color_vertex_input_description.attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  p_vkrs->pos_color_vertex_input_description.attribs[0].offset = 0;

  p_vkrs->pos_color_vertex_input_description.attribs[1].binding = 0;
  p_vkrs->pos_color_vertex_input_description.attribs[1].location = 1;
  p_vkrs->pos_color_vertex_input_description.attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
  // use_texture ? VK_FORMAT_R32G32_SFLOAT : VK_FORMAT_R32G32B32A32_SFLOAT;
  p_vkrs->pos_color_vertex_input_description.attribs[1].offset = 16;

  VkDynamicState dynamicStateEnables[VK_DYNAMIC_STATE_RANGE_SIZE];
  VkPipelineDynamicStateCreateInfo dynamicState = {};
  memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
  dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
  dynamicState.pNext = NULL;
  dynamicState.pDynamicStates = dynamicStateEnables;
  dynamicState.dynamicStateCount = 0;

  VkPipelineVertexInputStateCreateInfo vi;
  memset(&vi, 0, sizeof(vi));
  vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
  vi.pNext = NULL;
  vi.flags = 0;
  vi.vertexBindingDescriptionCount = 1;
  vi.pVertexBindingDescriptions = &p_vkrs->pos_color_vertex_input_description.binding;
  vi.vertexAttributeDescriptionCount = 2;
  vi.pVertexAttributeDescriptions = p_vkrs->pos_color_vertex_input_description.attribs;

  VkPipelineInputAssemblyStateCreateInfo ia;
  ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
  ia.pNext = NULL;
  ia.flags = 0;
  ia.primitiveRestartEnable = VK_FALSE;
  ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

  VkPipelineRasterizationStateCreateInfo rs;
  rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
  rs.pNext = NULL;
  rs.flags = 0;
  rs.polygonMode = VK_POLYGON_MODE_FILL;
  rs.cullMode = VK_CULL_MODE_BACK_BIT;
  rs.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
  rs.depthClampEnable = VK_FALSE;
  rs.rasterizerDiscardEnable = VK_FALSE;
  rs.depthBiasEnable = VK_FALSE;
  rs.depthBiasConstantFactor = 0;
  rs.depthBiasClamp = 0;
  rs.depthBiasSlopeFactor = 0;
  rs.lineWidth = 1.0f;

  VkPipelineColorBlendStateCreateInfo cb;
  cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
  cb.flags = 0;
  cb.pNext = NULL;
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
  cb.attachmentCount = 1;
  cb.pAttachments = att_state;
  cb.logicOpEnable = VK_FALSE;
  cb.logicOp = VK_LOGIC_OP_COPY;
  cb.blendConstants[0] = 1.0f;
  cb.blendConstants[1] = 1.0f;
  cb.blendConstants[2] = 1.0f;
  cb.blendConstants[3] = 1.0f;

  VkPipelineViewportStateCreateInfo vp = {};
  vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
  vp.pNext = NULL;
  vp.flags = 0;
  vp.viewportCount = NUM_VIEWPORTS;
  dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
  vp.scissorCount = NUM_SCISSORS;
  dynamicStateEnables[dynamicState.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
  vp.pScissors = NULL;
  vp.pViewports = NULL;

  VkPipelineDepthStencilStateCreateInfo ds;
  ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
  ds.pNext = NULL;
  ds.flags = 0;
  ds.depthTestEnable = false;
  ds.depthWriteEnable = false;
  ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
  ds.depthBoundsTestEnable = VK_FALSE;
  ds.stencilTestEnable = VK_FALSE;
  ds.back.failOp = VK_STENCIL_OP_KEEP;
  ds.back.passOp = VK_STENCIL_OP_KEEP;
  ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
  ds.back.compareMask = 0;
  ds.back.reference = 0;
  ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
  ds.back.writeMask = 0;
  ds.minDepthBounds = 0;
  ds.maxDepthBounds = 0;
  ds.stencilTestEnable = VK_FALSE;
  ds.front = ds.back;

  VkPipelineMultisampleStateCreateInfo ms;
  ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
  ms.pNext = NULL;
  ms.flags = 0;
  ms.pSampleMask = NULL;
  ms.rasterizationSamples = NUM_SAMPLES;
  ms.sampleShadingEnable = VK_FALSE;
  ms.alphaToCoverageEnable = VK_FALSE;
  ms.alphaToOneEnable = VK_FALSE;
  ms.minSampleShading = 0.0;

  VkGraphicsPipelineCreateInfo pipeline;
  pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
  pipeline.pNext = NULL;
  pipeline.layout = p_vkrs->pipeline_layout;
  pipeline.basePipelineHandle = VK_NULL_HANDLE;
  pipeline.basePipelineIndex = 0;
  pipeline.flags = 0;
  pipeline.pVertexInputState = &vi;
  pipeline.pInputAssemblyState = &ia;
  pipeline.pRasterizationState = &rs;
  pipeline.pColorBlendState = &cb;
  pipeline.pTessellationState = NULL;
  pipeline.pMultisampleState = &ms;
  pipeline.pDynamicState = &dynamicState;
  pipeline.pViewportState = &vp;
  pipeline.pDepthStencilState = &ds;
  pipeline.pStages = p_vkrs->shaderStages;
  pipeline.stageCount = 2;
  pipeline.renderPass = p_vkrs->present_render_pass;
  pipeline.subpass = 0;

  res = vkCreateGraphicsPipelines(p_vkrs->device, p_vkrs->pipelineCache, 1, &pipeline, NULL, &p_vkrs->pipeline);
  assert(res == VK_SUCCESS);
  return res;
}

void mvk_init_viewports(vk_render_state *p_vkrs, unsigned int width, unsigned int height)
{
  p_vkrs->viewport.width = (float)width;
  p_vkrs->viewport.height = (float)height;
  p_vkrs->viewport.minDepth = (float)0.0f;
  p_vkrs->viewport.maxDepth = (float)1.0f;
  p_vkrs->viewport.x = 0;
  p_vkrs->viewport.y = 0;
  vkCmdSetViewport(p_vkrs->cmd, 0, 1, &p_vkrs->viewport);
}

void mvk_init_scissors(vk_render_state *p_vkrs, unsigned int width, unsigned int height)
{
  p_vkrs->scissor.extent.width = width;
  p_vkrs->scissor.extent.height = height;
  p_vkrs->scissor.offset.x = 0;
  p_vkrs->scissor.offset.y = 0;
  vkCmdSetScissor(p_vkrs->cmd, 0, 1, &p_vkrs->scissor);
}

void mvk_destroy_textured_render_prog(vk_render_state *p_vkrs)
{
  vkDestroyPipeline(p_vkrs->device, p_vkrs->texture_prog.pipeline, NULL);
  vkDestroyDescriptorSetLayout(p_vkrs->device, p_vkrs->texture_prog.desc_layout, NULL);
  vkDestroyPipelineLayout(p_vkrs->device, p_vkrs->texture_prog.pipeline_layout, NULL);

  vkDestroyPipeline(p_vkrs->device, p_vkrs->font_prog.pipeline, NULL);
  vkDestroyDescriptorSetLayout(p_vkrs->device, p_vkrs->font_prog.desc_layout, NULL);
  vkDestroyPipelineLayout(p_vkrs->device, p_vkrs->font_prog.pipeline_layout, NULL);
}

void mvk_destroy_pipeline(vk_render_state *p_vkrs) { vkDestroyPipeline(p_vkrs->device, p_vkrs->pipeline, NULL); }

void mvk_destroy_pipeline_cache(vk_render_state *p_vkrs) { vkDestroyPipelineCache(p_vkrs->device, p_vkrs->pipelineCache, NULL); }

void mvk_destroy_descriptor_pool(vk_render_state *p_vkrs) { vkDestroyDescriptorPool(p_vkrs->device, p_vkrs->desc_pool, NULL); }

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

  vkDestroyBuffer(p_vkrs->device, p_vkrs->cube_vertices.buf, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->cube_vertices.mem, NULL);
}

void mvk_destroy_framebuffers(vk_render_state *p_vkrs)
{
  for (uint32_t i = 0; i < p_vkrs->swapchainImageCount; i++) {
    vkDestroyFramebuffer(p_vkrs->device, p_vkrs->framebuffers[i], NULL);
  }
  free(p_vkrs->framebuffers);
}

void mvk_destroy_shaders(vk_render_state *p_vkrs)
{
  vkDestroyShaderModule(p_vkrs->device, p_vkrs->shaderStages[0].module, NULL);
  vkDestroyShaderModule(p_vkrs->device, p_vkrs->shaderStages[1].module, NULL);
}

void mvk_destroy_uniform_buffer(vk_render_state *p_vkrs)
{
  vkDestroyBuffer(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem, NULL);

  vkDestroyBuffer(p_vkrs->device, p_vkrs->render_data_buffer.buffer, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->render_data_buffer.memory, NULL);
}

void mvk_destroy_descriptor_and_pipeline_layouts(vk_render_state *p_vkrs)
{
  for (int i = 0; i < 1; i++)
    vkDestroyDescriptorSetLayout(p_vkrs->device, p_vkrs->desc_layout[i], NULL);
  vkDestroyPipelineLayout(p_vkrs->device, p_vkrs->pipeline_layout, NULL);
}

void mvk_destroy_command_buffer(vk_render_state *p_vkrs)
{
  VkCommandBuffer cmd_bufs[1] = {p_vkrs->cmd};
  vkFreeCommandBuffers(p_vkrs->device, p_vkrs->cmd_pool, 1, cmd_bufs);
}

void mvk_destroy_command_pool(vk_render_state *p_vkrs) { vkDestroyCommandPool(p_vkrs->device, p_vkrs->cmd_pool, NULL); }

void mvk_destroy_depth_buffer(vk_render_state *p_vkrs)
{
  vkDestroyImageView(p_vkrs->device, p_vkrs->depth.view, NULL);
  vkDestroyImage(p_vkrs->device, p_vkrs->depth.image, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->depth.mem, NULL);
}

void mvk_destroy_swap_chain(vk_render_state *p_vkrs)
{
  for (uint32_t i = 0; i < p_vkrs->swapchainImageCount; i++) {
    vkDestroyImageView(p_vkrs->device, p_vkrs->buffers[i].view, NULL);
  }
  vkDestroySwapchainKHR(p_vkrs->device, p_vkrs->swap_chain, NULL);

  vkDestroySurfaceKHR(p_vkrs->inst, p_vkrs->surface, NULL);
}

void mvk_destroy_renderpass(vk_render_state *p_vkrs)
{
  vkDestroyRenderPass(p_vkrs->device, p_vkrs->present_render_pass, NULL);
  vkDestroyRenderPass(p_vkrs->device, p_vkrs->offscreen_render_pass, NULL);
}

void mvk_destroy_device(vk_render_state *p_vkrs)
{
  vkDeviceWaitIdle(p_vkrs->device);
  vkDestroyDevice(p_vkrs->device, NULL);
}

void mvk_destroy_instance(vk_render_state *p_vkrs) { vkDestroyInstance(p_vkrs->inst, NULL); }

VkResult createBuffer(vk_render_state *p_vkrs, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags mem_properties,
                      VkBuffer *buffer, VkDeviceMemory *bufferMemory)
{
  VkBufferCreateInfo bufferInfo{};
  bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  bufferInfo.size = size;
  bufferInfo.usage = usage;
  bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VkResult res = vkCreateBuffer(p_vkrs->device, &bufferInfo, nullptr, buffer);
  assert(res == VK_SUCCESS);

  VkMemoryRequirements memRequirements;
  vkGetBufferMemoryRequirements(p_vkrs->device, *buffer, &memRequirements);

  VkMemoryAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  allocInfo.allocationSize = memRequirements.size;
  assert(get_memory_type_index_from_properties(p_vkrs, memRequirements.memoryTypeBits, mem_properties,
                                               &allocInfo.memoryTypeIndex) == true);

  res = vkAllocateMemory(p_vkrs->device, &allocInfo, nullptr, bufferMemory);
  assert(res == VK_SUCCESS);

  res = vkBindBufferMemory(p_vkrs->device, *buffer, *bufferMemory, 0);
  assert(res == VK_SUCCESS);

  return res;
}
