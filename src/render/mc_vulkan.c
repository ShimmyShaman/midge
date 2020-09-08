
#include "render/mc_vulkan.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#define MC_RT_CALL(CALL)                                                \
  {                                                                     \
    VkResult mc_rt_vk_result = CALL;                                    \
    if (mc_rt_vk_result) {                                              \
      printf("VK-ERR[%i] :%i --" CALL "\n", mc_rt_vk_result, __LINE__); \
      return mc_rt_vk_result;                                           \
    }                                                                   \
  }

/*
 * Initializes a discovered global extension property.
 */
VkResult mvk_init_layer_instance_extension_properties(layer_properties *layer_props)
{
  VkResult res;
  char *layer_name = layer_props->properties.layerName;
  layer_props->instance_extensions.items = NULL;

  // Function has same format as mvk_init_global_layer_properties
  // for maybe the same reason??? Donno vulkan that well
  do {
    res = vkEnumerateInstanceExtensionProperties(layer_name, &layer_props->instance_extensions.size, NULL);
    VK_CHECK(res, "vkEnumerateInstanceExtensionProperties");

    if (layer_props->instance_extensions.size == 0) {
      return VK_SUCCESS;
    }

    layer_props->instance_extensions.items = (VkExtensionProperties *)realloc(
        layer_props->instance_extensions.items, layer_props->instance_extensions.size * sizeof(VkExtensionProperties));
    if (!layer_props->instance_extensions.items) {
      MCerror((VkResult)30, "realloc -- allocation error");
    }

    res = vkEnumerateInstanceExtensionProperties(layer_name, &layer_props->instance_extensions.size,
                                                 layer_props->instance_extensions.items);
  } while (res == VK_INCOMPLETE);

  return res;
}

VkResult mvk_init_global_layer_properties(vk_render_state *p_vkrs)
{
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
    res = vkEnumerateInstanceLayerProperties(&p_vkrs->instance_layer_properties.size, NULL);
    if (res) {
      printf("VK-ERR[%i] :%i --vkEnumerateInstanceLayerProperties\n", res, __LINE__);
      return res;
    }

    if (p_vkrs->instance_layer_properties.size == 0) {
      return VK_SUCCESS;
    }

    vk_props =
        (VkLayerProperties *)realloc(vk_props, p_vkrs->instance_layer_properties.size * sizeof(VkLayerProperties));
    if (!vk_props) {
      MCerror((VkResult)65, "realloc -- allocation error");
    }

    res = vkEnumerateInstanceLayerProperties(&p_vkrs->instance_layer_properties.size, vk_props);
  } while (res == VK_INCOMPLETE);

  // Allocate
  p_vkrs->instance_layer_properties.items =
      (layer_properties **)malloc(sizeof(layer_properties *) * p_vkrs->instance_layer_properties.size);
  /*
   * Now gather the extension list for each instance layer.
   */
  for (int i = 0; i < p_vkrs->instance_layer_properties.size; ++i) {
    // Initialize  new instance
    layer_properties *layer_props = (layer_properties *)malloc(sizeof(layer_properties));
    p_vkrs->instance_layer_properties.items[i] = layer_props;

    layer_props->device_extensions.size = 0;
    layer_props->instance_extensions.size = 0;
    layer_props->properties = vk_props[i];
    // printf("vk_props[%i]:%s-%s\n", i, vk_props[i].layerName, vk_props[i].description);
    res = mvk_init_layer_instance_extension_properties(layer_props);
    if (res) {
      printf("VK-ERR[%i] :%i --mvk_init_layer_instance_extension_properties\n", res, __LINE__);
      return res;
    }
  }
  free(vk_props);

  return res;
}

void mvk_init_device_extension_names(vk_render_state *p_vkrs)
{
  p_vkrs->device_extension_names.size = 1;
  p_vkrs->device_extension_names.items =
      (const char **)malloc(sizeof(const char *) * p_vkrs->device_extension_names.size);
  p_vkrs->device_extension_names.items[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
}

int mvk_check_layer_support(vk_render_state *p_vkrs)
{
  for (int a = 0; a < p_vkrs->instance_layer_names.size; ++a) {
    const char *layer_name = p_vkrs->instance_layer_names.items[a];
    bool layer_found = false;

    for (int b = 0; b < p_vkrs->instance_layer_properties.size; ++b) {
      layer_properties *layer_props = p_vkrs->instance_layer_properties.items[b];
      if (strcmp(layer_name, layer_props->properties.layerName) == 0) {
        layer_found = true;
        break;
      }
    }

    if (!layer_found) {
      printf("Could not find layer name='%s' in available layers!\n", layer_name);
      return 124;
    }
  }
  return 0;
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

  // -- Utilized Layers & Extensions --
  p_vkrs->instance_layer_names.size = 1;
  p_vkrs->instance_layer_names.items = (const char **)malloc(sizeof(const char *) * p_vkrs->instance_layer_names.size);
  p_vkrs->instance_layer_names.items[0] = "VK_LAYER_KHRONOS_validation";

  p_vkrs->instance_extension_names.size = 3;
  p_vkrs->instance_extension_names.items =
      (const char **)malloc(sizeof(const char *) * p_vkrs->instance_extension_names.size);
  //   p_vkrs->instance_extension_names.items[?] = VK_EXT_DEBUG_MARKER_EXTENSION_NAME;
  p_vkrs->instance_extension_names.items[0] = VK_KHR_XCB_SURFACE_EXTENSION_NAME;
  p_vkrs->instance_extension_names.items[1] = VK_KHR_SURFACE_EXTENSION_NAME;
  //   p_vkrs->instance_extension_names.items[?] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
  p_vkrs->instance_extension_names.items[2] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;

  int mc_res = mvk_check_layer_support(p_vkrs);
  if (mc_res) {
    MCerror((VkResult)156, "TODO");
  }

  // -- Debug --
  // VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
  // VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
  // mvk_setupDebug(p_vkrs, &debugReport, &debugCallbackCreateInfo);

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = p_vkrs->instance_layer_names.size;
  inst_info.ppEnabledLayerNames = p_vkrs->instance_layer_names.size ? p_vkrs->instance_layer_names.items : NULL;
  inst_info.enabledExtensionCount = p_vkrs->instance_extension_names.size;
  inst_info.ppEnabledExtensionNames = p_vkrs->instance_extension_names.items;
  // inst_info.pNext = (const void *)debugCallbackCreateInfo;

  // printf("create VkInstance...");
  VkResult res = vkCreateInstance(&inst_info, NULL, &p_vkrs->instance);
  VK_CHECK(res, "vkCreateInstance");
  // printf("SUCCESS %p\n", p_vkrs->instance);

  return res;
}

VkResult init_layer_device_extension_properties(vk_render_state *p_vkrs, layer_properties *layer_props)
{
  VkResult res;
  layer_props->device_extensions.items = NULL;
  char *layer_name = layer_props->properties.layerName;
  // printf()

  do {
    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &layer_props->device_extensions.size, NULL);
    VK_CHECK(res, "vkEnumerateDeviceExtensionProperties");

    if (layer_props->device_extensions.size == 0) {
      return VK_SUCCESS;
    }

    layer_props->device_extensions.items = (VkExtensionProperties *)realloc(
        layer_props->device_extensions.items, layer_props->device_extensions.size * sizeof(VkExtensionProperties));
    if (!layer_props->device_extensions.items) {
      MCerror((VkResult)196, "realloc -- allocation error");
    }

    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &layer_props->device_extensions.size,
                                               layer_props->device_extensions.items);
  } while (res == VK_INCOMPLETE);

  return res;
}

/*
 * Enumerates through the available graphics devices.
 */
VkResult mvk_init_physical_devices(vk_render_state *p_vkrs)
{
  // printf("mied-0 %p\n", p_vkrs->instance);
  p_vkrs->device_count = 0;
  VkResult res = vkEnumeratePhysicalDevices(p_vkrs->instance, &p_vkrs->device_count, NULL);
  VK_CHECK(res, "vkEnumeratePhysicalDevices");
  VK_ASSERT(p_vkrs->device_count > 0, "Must have at least one physical device that supports Vulkan!");
  // printf("mied-1a\n");

  // printf("mied-1\n");

  p_vkrs->gpus = (VkPhysicalDevice *)malloc(sizeof(VkPhysicalDevice) * p_vkrs->device_count);
  res = vkEnumeratePhysicalDevices(p_vkrs->instance, &p_vkrs->device_count, p_vkrs->gpus);
  VK_CHECK(res, "vkEnumeratePhysicalDevices");

  // printf("mied-2\n");
  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, NULL);
  VK_ASSERT(p_vkrs->queue_family_count > 0, "Must have at least one queue family!");

  // printf("before:%i\n", p_vkrs->queue_family_count);
  p_vkrs->queue_family_properties =
      (VkQueueFamilyProperties *)malloc(sizeof(VkQueueFamilyProperties) * p_vkrs->queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count,
                                           p_vkrs->queue_family_properties);
  VK_ASSERT(p_vkrs->queue_family_count > 0, "Must have at least one queue family!");
  // printf("after:%i\n", p_vkrs->queue_family_count);

  // printf("mied-3\n");
  /* This is as good a place as any to do this */
  vkGetPhysicalDeviceMemoryProperties(p_vkrs->gpus[0], &p_vkrs->memory_properties);
  vkGetPhysicalDeviceProperties(p_vkrs->gpus[0], &p_vkrs->gpu_props);
  /* query device extensions for enabled layers */
  for (int i = 0; i < p_vkrs->instance_layer_properties.size; ++i) {
    res = init_layer_device_extension_properties(p_vkrs, p_vkrs->instance_layer_properties.items[i]);
    VK_CHECK(res, "init_layer_device_extension_properties");
  }

  // printf("mied-4\n");
  return res;
}

/*
 * Constructs the surface, finds a graphics and present queue for it as well as a supported format.
 */
VkResult mvk_init_xcb_surface(vk_render_state *p_vkrs)
{
  VkResult res;

  // Construct the surface description
  VkXcbSurfaceCreateInfoKHR createInfo = {};
  createInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
  createInfo.pNext = NULL;
  createInfo.connection = p_vkrs->xcb_winfo->connection;
  createInfo.window = p_vkrs->xcb_winfo->window;
  res = vkCreateXcbSurfaceKHR(p_vkrs->instance, &createInfo, NULL, &p_vkrs->surface);
  // printf("vkCreateXcbSurfaceKHR:%p\n", p_vkrs->xcb_winfo->connection);
  // printf("vkCreateXcbSurfaceKHR:%p\n", p_vkrs->surface);
  VK_CHECK(res, "vkCreateXcbSurfaceKHR");

  // Iterate over each queue to learn whether it supports presenting:
  VkBool32 *pSupportsPresent = (VkBool32 *)malloc(p_vkrs->queue_family_count * sizeof(VkBool32));
  for (uint32_t i = 0; i < p_vkrs->queue_family_count; i++) {
    res = vkGetPhysicalDeviceSurfaceSupportKHR(p_vkrs->gpus[0], i, p_vkrs->surface, &pSupportsPresent[i]);
    VK_CHECK(res, "vkGetPhysicalDeviceSurfaceSupportKHR");
  }

  // Search for a graphics and a present queue in the array of queue
  // families, try to find one that supports both
  p_vkrs->graphics_queue_family_index = UINT32_MAX;
  p_vkrs->present_queue_family_index = UINT32_MAX;
  for (uint32_t i = 0; i < p_vkrs->queue_family_count; ++i) {
    if ((p_vkrs->queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) {
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
    for (uint32_t i = 0; i < p_vkrs->queue_family_count; ++i)
      if (pSupportsPresent[i] == VK_TRUE) {
        p_vkrs->present_queue_family_index = i;
        break;
      }
  }
  free(pSupportsPresent);

  // Generate error if could not find queues that support graphics
  // and present
  VK_ASSERT(p_vkrs->graphics_queue_family_index != UINT32_MAX, "Could not find queues that support graphics");
  VK_ASSERT(p_vkrs->present_queue_family_index != UINT32_MAX, "Could not find queues that support graphics");

  // Get the list of VkFormats that are supported:
  uint32_t formatCount;
  // printf("%p : %p\n", p_vkrs->gpus[0], p_vkrs->surface);
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(p_vkrs->gpus[0], p_vkrs->surface, &formatCount, NULL);
  // printf("formatCount:%i\n", formatCount);
  VK_CHECK(res, "vkGetPhysicalDeviceSurfaceFormatsKHR");
  VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
  res = vkGetPhysicalDeviceSurfaceFormatsKHR(p_vkrs->gpus[0], p_vkrs->surface, &formatCount, surfFormats);
  // printf("formatCount:%i\n", formatCount);
  VK_CHECK(res, "vkGetPhysicalDeviceSurfaceFormatsKHR");

  // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
  // the surface has no preferred format.  Otherwise, at least one
  // supported format will be returned.
  if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
    p_vkrs->format = VK_FORMAT_R8G8B8A8_SRGB;
  }
  else {
    VK_ASSERT(formatCount >= 1, "No supported formats?");
    p_vkrs->format = surfFormats[0].format;
  }
  // printf("swapchain format = %i\n", p_vkrs->format);
  free(surfFormats);

  return VK_SUCCESS;
}

/*
 * Initialize the graphics device.
 */
VkResult mvk_init_logical_device(vk_render_state *p_vkrs)
{
  // printf("mvk_init_device\n");
  VkResult res;
  VkDeviceQueueCreateInfo queue_info = {};

  float queue_priorities[1];
  queue_priorities[0] = 0.0;
  queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  queue_info.pNext = NULL;
  queue_info.queueCount = 1;
  queue_info.pQueuePriorities = queue_priorities;
  queue_info.queueFamilyIndex = p_vkrs->graphics_queue_family_index;

  VkPhysicalDeviceFeatures deviceFeatures = {};
  deviceFeatures.samplerAnisotropy = VK_TRUE;

  VkDeviceCreateInfo device_info = {};
  device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_info.pNext = NULL;
  device_info.queueCreateInfoCount = 1;
  device_info.pQueueCreateInfos = &queue_info;
  device_info.enabledExtensionCount = p_vkrs->device_extension_names.size;
  device_info.ppEnabledExtensionNames = device_info.enabledExtensionCount ? p_vkrs->device_extension_names.items : NULL;
  device_info.pEnabledFeatures = &deviceFeatures;

  // printf("requestedextensions %i\n", p_vkrs->device_extension_names.size);
  // for (int a = 0; a < p_vkrs->device_extension_names.size; ++a) {
  //   printf("--%s\n", p_vkrs->device_extension_names.items[a]);
  // }

  res = vkCreateDevice(p_vkrs->gpus[0], &device_info, NULL, &p_vkrs->device);
  VK_CHECK(res, "vkCreateDevice");

  // Obtain the device queue
  /* DEPENDS on init_swapchain_extension() */
  vkGetDeviceQueue(p_vkrs->device, p_vkrs->graphics_queue_family_index, 0, &p_vkrs->graphics_queue);
  if (p_vkrs->graphics_queue_family_index == p_vkrs->present_queue_family_index) {
    p_vkrs->present_queue = p_vkrs->graphics_queue;
  }
  else {
    vkGetDeviceQueue(p_vkrs->device, p_vkrs->present_queue_family_index, 0, &p_vkrs->present_queue);
  }

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

  res = vkCreateCommandPool(p_vkrs->device, &cmd_pool_info, NULL, &p_vkrs->command_pool);
  VK_CHECK(res, "vkCreateCommandPool");
  return res;
}

/*
 * Initializes the swap chain and the images it uses.
 */
VkResult mvk_init_swapchain_frame_buffers(vk_render_state *p_vkrs)
{
  VkResult res;

  /* DEPENDS on p_vkrs->cmd and p_vkrs->queue initialized */
  VkSurfaceCapabilitiesKHR surfCapabilities;

  res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(p_vkrs->gpus[0], p_vkrs->surface, &surfCapabilities);
  VK_CHECK(res, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR");

  uint32_t presentModeCount;
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(p_vkrs->gpus[0], p_vkrs->surface, &presentModeCount, NULL);
  VK_CHECK(res, "vkGetPhysicalDeviceSurfacePresentModesKHR");
  VkPresentModeKHR *presentModes = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
  VK_ASSERT(presentModes, "vk present modes");
  res = vkGetPhysicalDeviceSurfacePresentModesKHR(p_vkrs->gpus[0], p_vkrs->surface, &presentModeCount, presentModes);
  VK_CHECK(res, "vkGetPhysicalDeviceSurfacePresentModesKHR");

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
  const uint32_t compositeAlphaFlagCount = 4;
  VkCompositeAlphaFlagBitsKHR compositeAlphaFlags[compositeAlphaFlagCount] = {
      VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
      VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
  };
  for (uint32_t i = 0; i < compositeAlphaFlagCount; i++) {
    if (surfCapabilities.supportedCompositeAlpha & compositeAlphaFlags[i]) {
      compositeAlpha = compositeAlphaFlags[i];
      // printf("compositeAlpha set with '%u'\n", compositeAlpha);
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
  uint32_t queueFamilyIndices[2] = {(uint32_t)p_vkrs->graphics_queue_family_index,
                                    (uint32_t)p_vkrs->present_queue_family_index};
  if (p_vkrs->graphics_queue_family_index != p_vkrs->present_queue_family_index) {
    // If the graphics and present queues are from different queue families,
    // we either have to explicitly transfer ownership of images between the
    // queues, or we have to create the swapchain with imageSharingMode
    // as VK_SHARING_MODE_CONCURRENT
    swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    swapchain_ci.queueFamilyIndexCount = 2;
    swapchain_ci.pQueueFamilyIndices = queueFamilyIndices;
  }

  // Create the swap chain and its buffers
  // -- Size
  p_vkrs->swap_chain.size = 0;
  res = vkCreateSwapchainKHR(p_vkrs->device, &swapchain_ci, NULL, &p_vkrs->swap_chain.instance);
  VK_CHECK(res, "vkCreateSwapchainKHR");

  res = vkGetSwapchainImagesKHR(p_vkrs->device, p_vkrs->swap_chain.instance, &p_vkrs->swap_chain.size, NULL);
  VK_CHECK(res, "vkGetSwapchainImagesKHR");

  // -- Command Buffers
  VkCommandBufferAllocateInfo cmd = {};
  cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd.pNext = NULL;
  cmd.commandPool = p_vkrs->command_pool;
  cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd.commandBufferCount = p_vkrs->swap_chain.size;

  p_vkrs->swap_chain.command_buffers = (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * p_vkrs->swap_chain.size);
  VK_ASSERT(p_vkrs->swap_chain.command_buffers, "failed to allocate swap chain command buffers");
  res = vkAllocateCommandBuffers(p_vkrs->device, &cmd, p_vkrs->swap_chain.command_buffers);
  VK_CHECK(res, "vkAllocateCommandBuffers");

  // -- Images
  VkImage *images = (VkImage *)malloc(sizeof(VkImage) * p_vkrs->swap_chain.size);
  p_vkrs->swap_chain.images = images;
  VK_ASSERT(p_vkrs->swap_chain.images, "failed to allocate swap chain images");
  res = vkGetSwapchainImagesKHR(p_vkrs->device, p_vkrs->swap_chain.instance, &p_vkrs->swap_chain.size, images);
  VK_CHECK(res, "vkGetSwapchainImagesKHR");

  // -- Image Views
  p_vkrs->swap_chain.image_views = (VkImageView *)malloc(p_vkrs->swap_chain.size * sizeof(VkImageView));
  VK_ASSERT(p_vkrs->swap_chain.image_views, "failed to allocate swap chain image views");
  for (uint32_t i = 0; i < p_vkrs->swap_chain.size; i++) {
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

    color_image_view.image = p_vkrs->swap_chain.images[i];

    res = vkCreateImageView(p_vkrs->device, &color_image_view, NULL, &p_vkrs->swap_chain.image_views[i]);
    VK_CHECK(res, "vkCreateImageView");
  }
  p_vkrs->swap_chain.current_index = 0;

  if (presentModes) {
    free(presentModes);
  }

  return VK_SUCCESS;
}

uint32_t mvk_get_memory_type_index(vk_render_state *p_vkrs, uint32_t typeBits, VkMemoryPropertyFlags properties)
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

  VkResult res = vkCreateImage(p_vkrs->device, &imageCreateInfo, NULL, &p_vkrs->headless.image);
  VK_CHECK(res, "vkCreateImage");

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(p_vkrs->device, p_vkrs->headless.image, &memReqs);

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex =
      mvk_get_memory_type_index(p_vkrs, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  res = vkAllocateMemory(p_vkrs->device, &memAlloc, NULL, &p_vkrs->headless.memory);
  VK_CHECK(res, "vkAllocateMemory");
  res = vkBindImageMemory(p_vkrs->device, p_vkrs->headless.image, p_vkrs->headless.memory, 0);
  VK_CHECK(res, "vkBindImageMemory");

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
  res = vkCreateImageView(p_vkrs->device, &colorImageView, NULL, &p_vkrs->headless.view);
  VK_CHECK(res, "vkCreateImageView");

  return res;
}

VkResult mvk_init_uniform_buffer(vk_render_state *p_vkrs)
{
  VkResult res;

  float fov = glm_rad(45.0f);

  if (p_vkrs->window_width > p_vkrs->window_height) {
    fov *= (float)p_vkrs->window_height / (float)p_vkrs->window_width;
  }

  glm_ortho_default((float)p_vkrs->window_width / p_vkrs->window_height, (vec4 *)&p_vkrs->Projection);

  glm_lookat((vec3){0, 0, -10}, (vec3){0, 0, 0}, (vec3){0, -1, 0}, (vec4 *)&p_vkrs->View);

  glm_mat4_copy(GLM_MAT4_IDENTITY, (vec4 *)&p_vkrs->Model);

  // Vulkan clip space has inverted Y and half Z.
  mat4 clip = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.5f, 1.0f};
  glm_mat4_copy(clip, (vec4 *)&p_vkrs->Clip);

  glm_mat4_mul((vec4 *)&p_vkrs->View, (vec4 *)&p_vkrs->Model, (vec4 *)&p_vkrs->MVP);
  glm_mat4_mul((vec4 *)&p_vkrs->Projection, (vec4 *)&p_vkrs->MVP, (vec4 *)&p_vkrs->MVP);
  glm_mat4_mul((vec4 *)&p_vkrs->Clip, (vec4 *)&p_vkrs->MVP, (vec4 *)&p_vkrs->MVP);

  // /* VULKAN_KEY_START */
  // VkBufferCreateInfo buf_info = {};
  // buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  // buf_info.pNext = NULL;
  // buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  // buf_info.size = sizeof(p_vkrs->MVP);
  // buf_info.queueFamilyIndexCount = 0;
  // buf_info.pQueueFamilyIndices = NULL;
  // buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  // buf_info.flags = 0;
  // res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->global_vert_uniform_buffer.buf);
  // assert(res == VK_SUCCESS);

  // VkMemoryRequirements mem_reqs;
  // vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf, &mem_reqs);

  // VkMemoryAllocateInfo alloc_info = {};
  // alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  // alloc_info.pNext = NULL;
  // alloc_info.memoryTypeIndex = 0;

  // alloc_info.allocationSize = mem_reqs.size;
  // bool pass = get_memory_type_index_from_properties(
  //     p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //     &alloc_info.memoryTypeIndex);
  // assert(pass && "No mappable, coherent memory");

  // res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->global_vert_uniform_buffer.mem));
  // assert(res == VK_SUCCESS);

  // uint8_t *pData;
  // res = vkMapMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem, 0, mem_reqs.size, 0, (void **)&pData);
  // assert(res == VK_SUCCESS);

  // memcpy(pData, &p_vkrs->MVP, sizeof(p_vkrs->MVP));

  // vkUnmapMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem);

  // res = vkBindBufferMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf,
  //                          p_vkrs->global_vert_uniform_buffer.mem, 0);
  // assert(res == VK_SUCCESS);

  // p_vkrs->global_vert_uniform_buffer.buffer_info.buffer = p_vkrs->global_vert_uniform_buffer.buf;
  // p_vkrs->global_vert_uniform_buffer.buffer_info.offset = 0;
  // p_vkrs->global_vert_uniform_buffer.buffer_info.range = sizeof(p_vkrs->MVP);

  // /* SHARED BUFFER */
  // p_vkrs->render_data_buffer.allocated_size = 65536;
  // buf_info = {};
  // buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  // buf_info.pNext = NULL;
  // buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  // buf_info.size = p_vkrs->render_data_buffer.allocated_size;
  // buf_info.queueFamilyIndexCount = 0;
  // buf_info.pQueueFamilyIndices = NULL;
  // buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  // buf_info.flags = 0;
  // res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->render_data_buffer.buffer);
  // assert(res == VK_SUCCESS);

  // vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->render_data_buffer.buffer, &mem_reqs);

  // alloc_info = {};
  // alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  // alloc_info.pNext = NULL;
  // alloc_info.memoryTypeIndex = 0;

  // alloc_info.allocationSize = mem_reqs.size;
  // pass = get_memory_type_index_from_properties(
  //     p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //     &alloc_info.memoryTypeIndex);
  // assert(pass && "No mappable, coherent memory");

  // res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->render_data_buffer.memory));
  // assert(res == VK_SUCCESS);

  // res = vkBindBufferMemory(p_vkrs->device, p_vkrs->render_data_buffer.buffer, p_vkrs->render_data_buffer.memory, 0);
  // assert(res == VK_SUCCESS);

  // p_vkrs->render_data_buffer.frame_utilized_amount = 0;

  // p_vkrs->render_data_buffer.queued_copies_alloc = 256U;
  // p_vkrs->render_data_buffer.queued_copies_count = 0U;
  // p_vkrs->render_data_buffer.queued_copies =
  //     (queued_copy_info *)malloc(sizeof(queued_copy_info) * p_vkrs->render_data_buffer.queued_copies_alloc);

  return res;
}
/* ###################################################
   #               Vulkan Entry Point                #
   #               Initializes Vulkan                #
   ################################################### */
VkResult mvk_init_vulkan(vk_render_state *vkrs)
{
  VkResult res;
  res = mvk_init_global_layer_properties(vkrs);
  VK_CHECK(res, "mvk_init_global_layer_properties");
  res = mvk_init_device_extension_names(vkrs);
  VK_CHECK(res, "mvk_init_device_extension_names");
  res = mvk_init_instance(vkrs, "midge");
  VK_CHECK(res, "mvk_init_instance");
  res = mvk_init_physical_devices(vkrs);
  VK_CHECK(res, "mvk_init_physical_devices");

  // printf
  int init_window_res = mxcb_init_window(vkrs->xcb_winfo, vkrs->window_width, vkrs->window_height);
  VK_ASSERT(init_window_res == 0, "mxcb_init_window");
  VK_ASSERT(vkrs->xcb_winfo->connection != 0, "CHECK");

  res = mvk_init_xcb_surface(vkrs);
  VK_CHECK(res, "mvk_init_xcb_surface");
  res = mvk_init_logical_device(vkrs);
  VK_CHECK(res, "mvk_init_logical_device");

  res = mvk_init_command_pool(vkrs);
  VK_CHECK(res, "mvk_init_command_pool");
  res = mvk_init_swapchain_frame_buffers(vkrs);
  VK_CHECK(res, "mvk_init_swapchain_frame_buffers");
  res = mvk_init_headless_image(vkrs);
  VK_CHECK(res, "mvk_init_headless_image");

  return VK_SUCCESS;
}

void mvk_destroy_headless_image(vk_render_state *p_vkrs)
{
  vkDestroyImageView(p_vkrs->device, p_vkrs->headless.view, NULL);
  vkDestroyImage(p_vkrs->device, p_vkrs->headless.image, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->headless.memory, NULL);
}

void mvk_destroy_swapchain_frame_buffers(vk_render_state *p_vkrs)
{
  // Image Views
  if (p_vkrs->swap_chain.image_views) {
    for (uint32_t i = 0; i < p_vkrs->swap_chain.size; i++) {
      vkDestroyImageView(p_vkrs->device, p_vkrs->swap_chain.image_views[i], NULL);
    }
    free(p_vkrs->swap_chain.image_views);
    p_vkrs->swap_chain.image_views = NULL;
  }

  // Images
  if (p_vkrs->swap_chain.images) {
    // Don't destroy vulkan created swapchain images, it will be done in vkDestroySwapchainKHR
    free(p_vkrs->swap_chain.images);
    p_vkrs->swap_chain.images = NULL;
  }

  // Command-Buffers
  if (p_vkrs->swap_chain.command_buffers) {
    vkFreeCommandBuffers(p_vkrs->device, p_vkrs->command_pool, p_vkrs->swap_chain.size,
                         p_vkrs->swap_chain.command_buffers);
    p_vkrs->swap_chain.command_buffers = NULL;
  }

  vkDestroySwapchainKHR(p_vkrs->device, p_vkrs->swap_chain.instance, NULL);

  p_vkrs->swap_chain.size = 0;
}

void mvk_destroy_command_pool(vk_render_state *p_vkrs)
{
  vkDestroyCommandPool(p_vkrs->device, p_vkrs->command_pool, NULL);
}

void mvk_destroy_logical_device(vk_render_state *p_vkrs)
{
  vkDeviceWaitIdle(p_vkrs->device);
  vkDestroyDevice(p_vkrs->device, NULL);
}

void mvk_destroy_xcb_surface(vk_render_state *p_vkrs) { vkDestroySurfaceKHR(p_vkrs->instance, p_vkrs->surface, NULL); }

void mvk_destroy_physical_device_data(vk_render_state *p_vkrs)
{
  if (p_vkrs->gpus) {
    free(p_vkrs->gpus);
  }
  if (p_vkrs->queue_family_properties) {
    free(p_vkrs->queue_family_properties);
  }

  // instance_layer_properties.items[i]->device_extensions.items are freed in mvk_cleanup_global_layer_properties
}

void mvk_destroy_instance(vk_render_state *p_vkrs)
{
  if (p_vkrs->instance_layer_names.size && p_vkrs->instance_layer_names.items) {
    free(p_vkrs->instance_layer_names.items);
  }
  if (p_vkrs->instance_extension_names.size && p_vkrs->instance_extension_names.items) {
    free(p_vkrs->instance_extension_names.items);
  }

  // printf("instance @b destroy %p\n", p_vkrs->instance);
  vkDestroyInstance(p_vkrs->instance, NULL);
  // printf("instance @a destroy %p\n", p_vkrs->instance);
}

void mvk_cleanup_device_extension_names(vk_render_state *p_vkrs)
{
  if (p_vkrs->device_extension_names.size && p_vkrs->device_extension_names.items) {
    free(p_vkrs->device_extension_names.items);
  }
}

void mvk_cleanup_global_layer_properties(vk_render_state *p_vkrs)
{
  if (p_vkrs->instance_layer_properties.size && p_vkrs->instance_layer_properties.items) {
    // printf("instance_layer_properties.size:%i\n", p_vkrs->instance_layer_properties.size);
    for (int i = 0; i < p_vkrs->instance_layer_properties.size; ++i) {
      if (p_vkrs->instance_layer_properties.items[i]) {
        // printf("%i %s\n", i, p_vkrs->instance_layer_properties.items[i]->properties.layerName);
        // printf(" : %i\n", p_vkrs->instance_layer_properties.items[i]->instance_extensions.size);
        // printf(" %i\n", p_vkrs->instance_layer_properties.items[i]->device_extensions.size);
        if (p_vkrs->instance_layer_properties.items[i]->instance_extensions.size &&
            p_vkrs->instance_layer_properties.items[i]->instance_extensions.items) {
          free(p_vkrs->instance_layer_properties.items[i]->instance_extensions.items);
        }
        if (p_vkrs->instance_layer_properties.items[i]->device_extensions.size &&
            p_vkrs->instance_layer_properties.items[i]->device_extensions.items) {
          free(p_vkrs->instance_layer_properties.items[i]->device_extensions.items);
        }

        free(p_vkrs->instance_layer_properties.items[i]);
      }
    }
    free(p_vkrs->instance_layer_properties.items);
  }
}

/* ###################################################
   #                 Vulkan CLEANUP                  #
   #               Initializes Vulkan                #
   ################################################### */
VkResult mvk_cleanup_vulkan(vk_render_state *vkrs)
{
  mvk_destroy_headless_image(vkrs);
  mvk_destroy_swapchain_frame_buffers(vkrs);

  mvk_destroy_command_pool(vkrs);
  mvk_destroy_xcb_surface(vkrs);
  mvk_destroy_logical_device(vkrs);

  mxcb_destroy_window(vkrs->xcb_winfo);

  mvk_destroy_physical_device_data(vkrs);
  mvk_destroy_instance(vkrs);
  mvk_cleanup_device_extension_names(vkrs);
  mvk_cleanup_global_layer_properties(vkrs);

  return VK_SUCCESS;
}