
#include "render/mc_vulkan.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// Code modified from vulkan-tutorial.com / other sites too (TODO -- attributation)

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
    p_vkrs->format = VK_IMAGE_FORMAT;
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

  VkCommandPoolCreateInfo command_pool_info = {};
  command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  command_pool_info.pNext = NULL;
  command_pool_info.queueFamilyIndex = p_vkrs->graphics_queue_family_index;
  command_pool_info.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

  res = vkCreateCommandPool(p_vkrs->device, &command_pool_info, NULL, &p_vkrs->command_pool);
  VK_CHECK(res, "vkCreateCommandPool");
  return res;
}

/*
 * Initializes the swap chain and the images it uses.
 */
VkResult mvk_init_swapchain_data(vk_render_state *p_vkrs)
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

  // width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
  if (surfCapabilities.currentExtent.width == 0xFFFFFFFF) {
    // If the surface size is undefined, the size is set to
    // the size of the images requested.
    p_vkrs->swap_chain.extents.width = p_vkrs->window_width;
    p_vkrs->swap_chain.extents.height = p_vkrs->window_height;
    if (p_vkrs->swap_chain.extents.width < surfCapabilities.minImageExtent.width) {
      p_vkrs->swap_chain.extents.width = surfCapabilities.minImageExtent.width;
    }
    else if (p_vkrs->swap_chain.extents.width > surfCapabilities.maxImageExtent.width) {
      p_vkrs->swap_chain.extents.width = surfCapabilities.maxImageExtent.width;
    }

    if (p_vkrs->swap_chain.extents.height < surfCapabilities.minImageExtent.height) {
      p_vkrs->swap_chain.extents.height = surfCapabilities.minImageExtent.height;
    }
    else if (p_vkrs->swap_chain.extents.height > surfCapabilities.maxImageExtent.height) {
      p_vkrs->swap_chain.extents.height = surfCapabilities.maxImageExtent.height;
    }
  }
  else {
    // If the surface size is defined, the swap chain size must match
    p_vkrs->swap_chain.extents = surfCapabilities.currentExtent;
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
  swapchain_ci.imageExtent.width = p_vkrs->swap_chain.extents.width;
  swapchain_ci.imageExtent.height = p_vkrs->swap_chain.extents.height;
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
  p_vkrs->swap_chain.size_count = 0;
  res = vkCreateSwapchainKHR(p_vkrs->device, &swapchain_ci, NULL, &p_vkrs->swap_chain.instance);
  VK_CHECK(res, "vkCreateSwapchainKHR");

  res = vkGetSwapchainImagesKHR(p_vkrs->device, p_vkrs->swap_chain.instance, &p_vkrs->swap_chain.size_count, NULL);
  VK_CHECK(res, "vkGetSwapchainImagesKHR");

  // -- Command Buffers
  VkCommandBufferAllocateInfo cmd = {};
  cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd.pNext = NULL;
  cmd.commandPool = p_vkrs->command_pool;
  cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd.commandBufferCount = p_vkrs->swap_chain.size_count;

  p_vkrs->swap_chain.command_buffers =
      (VkCommandBuffer *)malloc(sizeof(VkCommandBuffer) * p_vkrs->swap_chain.size_count);
  VK_ASSERT(p_vkrs->swap_chain.command_buffers, "failed to allocate swap chain command buffers");
  res = vkAllocateCommandBuffers(p_vkrs->device, &cmd, p_vkrs->swap_chain.command_buffers);
  VK_CHECK(res, "vkAllocateCommandBuffers");

  // -- Images
  VkImage *images = (VkImage *)malloc(sizeof(VkImage) * p_vkrs->swap_chain.size_count);
  p_vkrs->swap_chain.images = images;
  VK_ASSERT(p_vkrs->swap_chain.images, "failed to allocate swap chain images");
  res = vkGetSwapchainImagesKHR(p_vkrs->device, p_vkrs->swap_chain.instance, &p_vkrs->swap_chain.size_count, images);
  VK_CHECK(res, "vkGetSwapchainImagesKHR");

  // -- Image Views
  p_vkrs->swap_chain.image_views = (VkImageView *)malloc(p_vkrs->swap_chain.size_count * sizeof(VkImageView));
  VK_ASSERT(p_vkrs->swap_chain.image_views, "failed to allocate swap chain image views");
  for (uint32_t i = 0; i < p_vkrs->swap_chain.size_count; i++) {
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

uint32_t mvk_get_physical_memory_type_index(vk_render_state *p_vkrs, uint32_t typeBits,
                                            VkMemoryPropertyFlags properties)
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
  VkResult res;

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

  // -- Command Buffers
  VkCommandBufferAllocateInfo cmd = {};
  cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  cmd.pNext = NULL;
  cmd.commandPool = p_vkrs->command_pool;
  cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  cmd.commandBufferCount = 1;

  res = vkAllocateCommandBuffers(p_vkrs->device, &cmd, &p_vkrs->headless.command_buffer);
  VK_CHECK(res, "vkAllocateCommandBuffers");

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

  res = vkCreateImage(p_vkrs->device, &imageCreateInfo, NULL, &p_vkrs->headless.image);
  VK_CHECK(res, "vkCreateImage");

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(p_vkrs->device, p_vkrs->headless.image, &memReqs);

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex =
      mvk_get_physical_memory_type_index(p_vkrs, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
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

/*
 * Obtains the memory type from the available properties, returning false if no memory type was matched.
 */
bool mvk_get_properties_memory_type_index(vk_render_state *p_vkrs, uint32_t typeBits, VkFlags requirements_mask,
                                          uint32_t *typeIndex)
{
  // printf("mvk_get_properties_memory_type_index: requirements_mask=%i\n", (int)requirements_mask);

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
  VkBufferCreateInfo buf_info = {};
  // buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  // buf_info.pNext = NULL;
  // buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  // buf_info.size = sizeof(float) * 16;
  // buf_info.queueFamilyIndexCount = 0;
  // buf_info.pQueueFamilyIndices = NULL;
  // buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  // buf_info.flags = 0;
  // res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->global_vert_uniform_buffer.buf);
  // VK_CHECK(res, "vkCreateBuffer");

  VkMemoryRequirements mem_reqs;
  // vkGetBufferMemoryRequirements(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf, &mem_reqs);

  // alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  // alloc_info.pNext = NULL;
  // alloc_info.memoryTypeIndex = 0;

  // alloc_info.allocationSize = mem_reqs.size;
  bool pass;
  // bool pass = mvk_get_properties_memory_type_index(
  //     p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
  //     &alloc_info.memoryTypeIndex);
  // VK_ASSERT(pass, "No mappable, coherent memory");

  // res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &(p_vkrs->global_vert_uniform_buffer.mem));
  // VK_CHECK(res, "vkAllocateMemory");

  uint8_t *pData;
  // res = vkMapMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem, 0, mem_reqs.size, 0, (void **)&pData);
  // VK_CHECK(res, "vkMapMemory");

  // memcpy(pData, &p_vkrs->MVP, sizeof(float) * 16);

  // vkUnmapMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem);

  // res = vkBindBufferMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf,
  //                          p_vkrs->global_vert_uniform_buffer.mem, 0);
  // VK_CHECK(res, "vkBindBufferMemory");

  // p_vkrs->global_vert_uniform_buffer.buffer_info.buffer = p_vkrs->global_vert_uniform_buffer.buf;
  // p_vkrs->global_vert_uniform_buffer.buffer_info.offset = 0;
  // p_vkrs->global_vert_uniform_buffer.buffer_info.range = sizeof(float) * 16;

  /* SHARED BUFFER */
  // p_vkrs->render_data_buffer.allocated_size = 262144 + 262144 + 262144; // TODO reduce/refactor etc
  // buf_info = {};
  // buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  // buf_info.pNext = NULL;
  // buf_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  // buf_info.size = 262144; // p_vkrs->render_data_buffer.allocated_size;
  // buf_info.queueFamilyIndexCount = 0;
  // buf_info.pQueueFamilyIndices = NULL;
  // buf_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  // buf_info.flags = 0;
  // res = vkCreateBuffer(p_vkrs->device, &buf_info, NULL, &p_vkrs->render_data_buffer.buffer);
  // VK_CHECK(res, "vkCreateBuffer");

  // p_vkrs->render_data_buffer.buffer_offset = 262144;
  // res = vkBindBufferMemory(p_vkrs->device, p_vkrs->render_data_buffer.buffer, p_vkrs->render_data_buffer.memory,
  //                          p_vkrs->render_data_buffer.buffer_offset);
  // VK_CHECK(res, "vkBindBufferMemory");

  // p_vkrs->render_data_buffer.frame_utilized_amount = 0;

  p_vkrs->render_data_buffer.dynamic_buffers.size = 0;
  p_vkrs->render_data_buffer.dynamic_buffers.activated = 0;
  p_vkrs->render_data_buffer.dynamic_buffers.min_memory_allocation = 32768;
  p_vkrs->render_data_buffer.queued_copies_alloc = 2048U;
  p_vkrs->render_data_buffer.queued_copies_count = 0U;
  p_vkrs->render_data_buffer.queued_copies =
      (queued_copy_info *)malloc(sizeof(queued_copy_info) * p_vkrs->render_data_buffer.queued_copies_alloc);

  // int res = mvk_allocate_dynamic_render_data_memory(p_vkrs, 0);
  VK_CHECK(res, "Dynamic Render Data Buffer Allocation");

  return res;
}

VkResult mvk_allocate_dynamic_render_data_memory(vk_render_state *p_vkrs, int min_buffer_size)
{
  VkDeviceSize next_mem_block_size = p_vkrs->render_data_buffer.dynamic_buffers.min_memory_allocation;
  {
    for (int m = 0; m < (int)p_vkrs->render_data_buffer.dynamic_buffers.size / 2; ++m) {
      next_mem_block_size *= 2;
    }
    while (next_mem_block_size < min_buffer_size) {
      next_mem_block_size *= 2;
    }
  }

  // Attach a new block onto the current collection
  mvk_dynamic_buffer_block *block = (mvk_dynamic_buffer_block *)malloc(sizeof(mvk_dynamic_buffer_block));
  block->allocated_size = next_mem_block_size;
  block->utilized = 0;

  // Attach it
  reallocate_collection((void ***)&p_vkrs->render_data_buffer.dynamic_buffers.blocks,
                        &p_vkrs->render_data_buffer.dynamic_buffers.size,
                        p_vkrs->render_data_buffer.dynamic_buffers.size + 1, 0);
  p_vkrs->render_data_buffer.dynamic_buffers.blocks[p_vkrs->render_data_buffer.dynamic_buffers.size - 1] = block;

  // Create the buffer
  VkBufferCreateInfo buffer_create_info = {};
  buffer_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
  buffer_create_info.pNext = NULL;
  buffer_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
  buffer_create_info.size = block->allocated_size;
  buffer_create_info.queueFamilyIndexCount = 0;
  buffer_create_info.pQueueFamilyIndices = NULL;
  buffer_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
  buffer_create_info.flags = 0;
  VkResult res = vkCreateBuffer(p_vkrs->device, &buffer_create_info, NULL, &block->buffer);
  VK_CHECK(res, "vkCreateBuffer");

  // Create & allocate the memory block
  VkMemoryRequirements mem_reqs;
  vkGetBufferMemoryRequirements(p_vkrs->device, block->buffer, &mem_reqs);

  VkMemoryAllocateInfo alloc_info = {};
  alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  alloc_info.pNext = NULL;
  alloc_info.memoryTypeIndex = 0;

  alloc_info.allocationSize = next_mem_block_size;
  bool pass = mvk_get_properties_memory_type_index(
      p_vkrs, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
      &alloc_info.memoryTypeIndex);
  VK_ASSERT(pass, "No mappable, coherent memory");

  res = vkAllocateMemory(p_vkrs->device, &alloc_info, NULL, &block->memory);
  VK_CHECK(res, "vkAllocateMemory");

  // Bind together
  res = vkBindBufferMemory(p_vkrs->device, block->buffer, block->memory, 0);
  VK_CHECK(res, "vkBindBufferMemory");

  // printf("Created block with memory=%lu\n", next_mem_block_size);
}

VkResult mvk_init_present_renderpass(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_swap_chain() and init_depth_buffer() */

  bool clear = true;

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
  color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
  color_attachment.flags = 0;

  VkAttachmentReference color_reference = {};
  color_reference.attachment = 0;
  color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

  // VkAttachmentDescription depth_attachment = {};
  // depth_attachment.format = p_vkrs->depth_buffer.format;
  // depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  // depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  // depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  // depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  // depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  // depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  // depth_attachment.flags = 0;

  // VkAttachmentReference depth_reference = {};
  // depth_reference.attachment = 1;
  // depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

  VkSubpassDescription subpass = {};
  subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
  subpass.flags = 0;
  subpass.inputAttachmentCount = 0;
  subpass.pInputAttachments = NULL;
  subpass.colorAttachmentCount = 1;
  subpass.pColorAttachments = &color_reference;
  subpass.pResolveAttachments = NULL;
  subpass.pDepthStencilAttachment = NULL; //&depth_reference;
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

  VkAttachmentDescription attachment_descriptions[] = {color_attachment}; //, depth_attachment
  // printf("mvk_init_present_renderpass:color_attachment:%i depth_attachment:%i\n", color_attachment.format,
  //        depth_attachment.format);

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.pNext = NULL;
  rp_info.attachmentCount = 1;
  rp_info.pAttachments = attachment_descriptions;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 1;
  rp_info.pDependencies = &subpass_dependency;

  res = vkCreateRenderPass(p_vkrs->device, &rp_info, NULL, &p_vkrs->present_render_pass);
  assert(res == VK_SUCCESS);
  return res;
}

VkResult mvk_init_offscreen_renderpass_3d(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_depth_buffer() */

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

  VkAttachmentDescription depth_attachment = {};
  depth_attachment.format = p_vkrs->depth_buffer.format;
  depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
  depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
  depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
  depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
  depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
  depth_attachment.flags = 0;

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
  subpass.pDepthStencilAttachment = &depth_reference;
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

  VkAttachmentDescription attachment_descriptions[] = {color_attachment, depth_attachment};
  printf("mvk_init_offscreen_renderpass:color_attachment:%i depth_attachment:%i\n", color_attachment.format,
         depth_attachment.format);

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.pNext = NULL;
  rp_info.attachmentCount = 2;
  rp_info.pAttachments = attachment_descriptions;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 2;
  rp_info.pDependencies = subpass_dependencies;

  res = vkCreateRenderPass(p_vkrs->device, &rp_info, NULL, &p_vkrs->offscreen_render_pass_3d);
  assert(res == VK_SUCCESS);

  return res;
}

VkResult mvk_init_offscreen_renderpass_2d(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_depth_buffer() */

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

  VkAttachmentDescription attachment_descriptions[] = {color_attachment};

  VkRenderPassCreateInfo rp_info = {};
  rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
  rp_info.pNext = NULL;
  rp_info.attachmentCount = 1;
  rp_info.pAttachments = attachment_descriptions;
  rp_info.subpassCount = 1;
  rp_info.pSubpasses = &subpass;
  rp_info.dependencyCount = 2;
  rp_info.pDependencies = subpass_dependencies;

  res = vkCreateRenderPass(p_vkrs->device, &rp_info, NULL, &p_vkrs->offscreen_render_pass_2d);
  assert(res == VK_SUCCESS);

  return res;
}

VkResult GLSLtoSPV(const VkShaderStageFlagBits shader_type, const char *p_shader_text, unsigned int **spirv,
                   unsigned int *spirv_size)
{
  // Use glslangValidator from file
  // Generate the shader file
  const char *ext = NULL;
  if ((shader_type & VK_SHADER_STAGE_VERTEX_BIT) == VK_SHADER_STAGE_VERTEX_BIT) {
    ext = "vert";
  }
  else if ((shader_type & VK_SHADER_STAGE_FRAGMENT_BIT) == VK_SHADER_STAGE_FRAGMENT_BIT) {
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
  char *argv[4] = {exePath, execOutput, arg_V,
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

  {
    uint32_t spirv_alloc = strlen(p_shader_text);
    *spirv = (uint32_t *)malloc(sizeof(uint32_t) * spirv_alloc);
    VK_ASSERT(*spirv, "malloc error spirv 1120");
    *spirv_size = 0;

    uint32_t code;
    while (fread(&code, sizeof(uint32_t), 1, fp) == 1) {
      if (*spirv_size + 1 >= spirv_alloc) {
        spirv_alloc = spirv_alloc + 16 + spirv_alloc / 2;
        // printf("reallocing to %u\n", spirv_alloc);

        *spirv = (uint32_t *)realloc(*spirv, sizeof(uint32_t) * spirv_alloc);
        VK_ASSERT(*spirv, "realloc error spirv 1087");
      }
      (*spirv)[*spirv_size] = code;
      ++*spirv_size;
    }
  }
  fclose(fp);
  remove(execOutput);
  remove(shaderFile);
  // printf("->%s: sizeof=4*%u\n", shaderFile, *spirv_size);

  // TEST READ
  // fp = fopen("/home/jason/midge/test.spv", "w");
  // if (!fp)
  // {
  //   printf("GLSLtoSPV: couldn't open file: %s", "/home/jason/midge/test.spv");
  //   return VK_ERROR_UNKNOWN;
  // }
  // printf("test.spv opened: ");
  // for (int i = 0; i < spirv.size(); ++i)
  // {
  // printf("%i", spirv[i]);
  //   fwrite(&spirv[i], sizeof(uint32_t), 1, fp);
  // }
  // fclose(fp);

  return VK_SUCCESS;
}

VkResult mvk_init_tint_render_prog(vk_render_state *p_vkrs)
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
    layout_bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
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

    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layout_create_info, NULL, &p_vkrs->tint_prog.descriptor_layout);
    VK_CHECK(res, "vkCreateDescriptorSetLayout");
  }

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
    const char *vertex_shader_code = "#version 450\n"
                                     "#extension GL_ARB_separate_shader_objects : enable\n"
                                     //  "#extension GL_ARB_shading_language_420pack : enable\n"
                                     "layout (std140, binding = 0) uniform UBO0 {\n"
                                     "    mat4 mvp;\n"
                                     "} globalUI;\n"
                                     "layout (binding = 1) uniform UBO1 {\n"
                                     "    vec2 offset;\n"
                                     "    vec2 scale;\n"
                                     "} element;\n"
                                     "\n"
                                     "layout(location = 0) in vec2 inPosition;\n"
                                     "\n"
                                     "void main() {\n"
                                     "   gl_Position = globalUI.mvp * vec4(inPosition, 0.0, 1.0);\n"
                                     "   gl_Position.xy *= element.scale.xy;\n"
                                     "   gl_Position.xy += element.offset.xy;\n"
                                     "}\n";

    const char *fragment_shader_code = "#version 450\n"
                                       "#extension GL_ARB_separate_shader_objects : enable\n"
                                       //  "#extension GL_ARB_shading_language_420pack : enable\n"
                                       "layout (binding = 2) uniform UBO2 {\n"
                                       "    vec4 tint;\n"
                                       "} element;\n"
                                       "layout (location = 0) out vec4 outColor;\n"
                                       "void main() {\n"
                                       "   outColor = element.tint;\n"
                                       "}\n";

    {
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
  const int VERTEX_ATTRIBUTE_COUNT = 1;
  VkVertexInputAttributeDescription attributeDescriptions[VERTEX_ATTRIBUTE_COUNT];
  {
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(vec2);
    // printf("sizeof(vec2)=%zu\n", sizeof(vec2));
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // p_vkrs->format; // VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = 0;                       // offsetof(textured_image_vertex, position);
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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
    pipelineLayoutInfo.pSetLayouts = &p_vkrs->tint_prog.descriptor_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, NULL, &p_vkrs->tint_prog.pipeline_layout);
    VK_CHECK(res, "vkCreatePipelineLayout :: Failed to create pipeline layout!");

    // VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_TRUE;
    // depthStencil.depthWriteEnable = VK_TRUE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // depthStencil.depthBoundsTestEnable = VK_FALSE;
    // depthStencil.minDepthBounds = 0.0f; // Optional
    // depthStencil.maxDepthBounds = 1.0f; // Optional
    // depthStencil.stencilTestEnable = VK_FALSE;
    // depthStencil.front = {}; // Optional
    // depthStencil.back = {};  // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = NULL; // &depthStencil;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = p_vkrs->tint_prog.pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->present_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res =
        vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &p_vkrs->tint_prog.pipeline);
    VK_CHECK(res, "vkCreateGraphicsPipelines :: Failed to create pipeline");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, NULL);
  }

  VkPipelineCacheCreateInfo pipelineCache;
  pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
  pipelineCache.pNext = NULL;
  pipelineCache.initialDataSize = 0;
  pipelineCache.pInitialData = NULL;
  pipelineCache.flags = 0;
  res = vkCreatePipelineCache(p_vkrs->device, &pipelineCache, NULL, &p_vkrs->pipelineCache);
  assert(res == VK_SUCCESS);

  return VK_SUCCESS;
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

    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layoutCreateInfo, NULL, &p_vkrs->texture_prog.descriptor_layout);
    VK_CHECK(res, "vkCreateDescriptorSetLayout");
  }

  const char *texture_vertex_shader_code = "#version 450\n"
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
                                           "}\n";
  VkShaderStageFlagBits texture_vertex_shader_stage = VK_SHADER_STAGE_VERTEX_BIT;

  const char *texture_fragment_shader_code = "#version 450\n"
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
                                             "}\n";
  VkShaderStageFlagBits texture_fragment_shader_stage = VK_SHADER_STAGE_FRAGMENT_BIT;

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
    VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[0];
    shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStateCreateInfo->pNext = NULL;
    shaderStateCreateInfo->pSpecializationInfo = NULL;
    shaderStateCreateInfo->flags = 0;
    shaderStateCreateInfo->stage = texture_vertex_shader_stage;
    shaderStateCreateInfo->pName = "main";

    unsigned int *vtx_spv, vtx_spv_size;
    VkResult res = GLSLtoSPV(texture_vertex_shader_stage, texture_vertex_shader_code, &vtx_spv, &vtx_spv_size);
    VK_CHECK(res, "GLSLtoSPV");

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
    VkPipelineShaderStageCreateInfo *shaderStateCreateInfo = &shaderStages[1];
    shaderStateCreateInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStateCreateInfo->pNext = NULL;
    shaderStateCreateInfo->pSpecializationInfo = NULL;
    shaderStateCreateInfo->flags = 0;
    shaderStateCreateInfo->stage = texture_fragment_shader_stage;
    shaderStateCreateInfo->pName = "main";

    unsigned int *vtx_spv, vtx_spv_size;
    VkResult res = GLSLtoSPV(texture_fragment_shader_stage, texture_fragment_shader_code, &vtx_spv, &vtx_spv_size);
    VK_CHECK(res, "GLSLtoSPV");

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

  // Vertex Bindings
  VkVertexInputBindingDescription bindingDescription = {};
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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
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
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    // att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    // att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
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
    pipelineLayoutInfo.pSetLayouts = &p_vkrs->texture_prog.descriptor_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, NULL, &p_vkrs->texture_prog.pipeline_layout);
    VK_CHECK(res, "vkCreatePipelineLayout :: Failed to create pipeline layout!");

    // VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_TRUE;
    // depthStencil.depthWriteEnable = VK_TRUE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // depthStencil.depthBoundsTestEnable = VK_FALSE;
    // depthStencil.minDepthBounds = 0.0f; // Optional
    // depthStencil.maxDepthBounds = 1.0f; // Optional
    // depthStencil.stencilTestEnable = VK_FALSE;
    // depthStencil.front = {}; // Optional
    // depthStencil.back = {};  // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = NULL; //&depthStencil;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = p_vkrs->texture_prog.pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->present_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res = vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL,
                                    &p_vkrs->texture_prog.pipeline);
    VK_CHECK(res, "vkCreateGraphicsPipelines :: Failed to create pipeline");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, NULL);
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

    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layoutCreateInfo, NULL, &p_vkrs->font_prog.descriptor_layout);
    assert(res == VK_SUCCESS);
  }

  const char *vertex_shader_code = "#version 450\n"
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
                                   "}\n";

  const char *fragment_shader_code =
      "#version 450\n"
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
      // "   outColor.a = 0.3 + 0.7 * outColor.r;\n"
      "   outColor.a = min(max(0, outColor.r - 0.2) * 0.2f + outColor.r * 1.5, 1.0);\n"
      "   outColor.rgb = element.tint.rgb * outColor.a;\n"
      "   outColor.a = 1.0f;"
      "}\n";

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
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

  // Vertex Bindings
  VkVertexInputBindingDescription bindingDescription = {};
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
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = VERTEX_ATTRIBUTE_COUNT;
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
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState att_state[1];
    att_state[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
                                  VK_COLOR_COMPONENT_A_BIT; // TODO
    att_state[0].blendEnable = VK_TRUE;
    att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
    // If this is true, then why is the render target blending when placed in another image
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    // att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    // att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
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
    pipelineLayoutInfo.pSetLayouts = &p_vkrs->font_prog.descriptor_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, NULL, &p_vkrs->font_prog.pipeline_layout);
    VK_CHECK(res, "failed to create pipeline layout!");

    // VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    // depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    // depthStencil.depthTestEnable = VK_TRUE;
    // depthStencil.depthWriteEnable = VK_TRUE;
    // depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    // depthStencil.depthBoundsTestEnable = VK_FALSE;
    // depthStencil.minDepthBounds = 0.0f; // Optional
    // depthStencil.maxDepthBounds = 1.0f; // Optional
    // depthStencil.stencilTestEnable = VK_FALSE;
    // depthStencil.front = {}; // Optional
    // depthStencil.back = {};  // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.pNext = NULL;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pDepthStencilState = NULL; //&depthStencil;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = p_vkrs->font_prog.pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->present_render_pass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res =
        vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &p_vkrs->font_prog.pipeline);
    VK_CHECK(res, "failed to create pipeline!");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, NULL);
  }

  return VK_SUCCESS;
}

VkResult mvk_init_mesh_render_prog(vk_render_state *p_vkrs)
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

    // Next take layout bindings and use them to create a descriptor set layout
    VkDescriptorSetLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout_create_info.pNext = NULL;
    layout_create_info.flags = 0;
    layout_create_info.bindingCount = 3;
    layout_create_info.pBindings = layout_bindings;

    res = vkCreateDescriptorSetLayout(p_vkrs->device, &layout_create_info, NULL, &p_vkrs->mesh_prog.descriptor_layout);
    VK_CHECK(res, "vkCreateDescriptorSetLayout");
  }

  const int SHADER_STAGE_MODULES = 2;
  VkPipelineShaderStageCreateInfo shaderStages[SHADER_STAGE_MODULES];
  {
    const char *vertex_shader_code = "#version 450\n"
                                     "#extension GL_ARB_separate_shader_objects : enable\n"
                                     //  "#extension GL_ARB_shading_language_420pack : enable\n"
                                     "layout (std140, binding = 0) uniform UBO0 {\n"
                                     "    mat4 mvp;\n"
                                     "} world;\n"
                                     "layout (binding = 1) uniform UBO1 {\n"
                                     "    vec2 offset;\n"
                                     "    vec2 scale;\n"
                                     "} element;\n"
                                     "\n"
                                     "layout(location = 0) in vec3 inPosition;\n"
                                     "layout(location = 1) in vec2 inTexCoord;\n"
                                     "\n"
                                     "layout(location = 1) out vec2 fragTexCoord;\n"
                                     "\n"
                                     "void main() {\n"
                                     "   gl_Position = world.mvp * vec4(inPosition, 1.0);\n"
                                     "   fragTexCoord = inTexCoord;\n"
                                     "}\n";

    const char *fragment_shader_code = "#version 450\n"
                                       "#extension GL_ARB_separate_shader_objects : enable\n"
                                       //  "#extension GL_ARB_shading_language_420pack : enable\n"
                                       "\n"
                                       "layout(binding = 2) uniform sampler2D texSampler;\n"
                                       "\n"
                                       "layout (location = 0) out vec4 outColor;\n"
                                       "\n"
                                       "layout(location = 1) in vec2 fragTexCoord;\n"
                                       "\n"
                                       "void main() {\n"
                                       //  "   outColor = vec4(0.4, 0.2, 0.8, 1.0);\n"
                                       "   outColor = texture(texSampler, fragTexCoord);\n"
                                       "}\n";

    {
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
    pipelineLayoutInfo.pSetLayouts = &p_vkrs->mesh_prog.descriptor_layout;

    res = vkCreatePipelineLayout(p_vkrs->device, &pipelineLayoutInfo, NULL, &p_vkrs->mesh_prog.pipeline_layout);
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
    pipelineInfo.layout = p_vkrs->mesh_prog.pipeline_layout;
    pipelineInfo.renderPass = p_vkrs->offscreen_render_pass_3d;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    res =
        vkCreateGraphicsPipelines(p_vkrs->device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &p_vkrs->mesh_prog.pipeline);
    VK_CHECK(res, "vkCreateGraphicsPipelines :: Failed to create pipeline");
  }

  for (int i = 0; i < SHADER_STAGE_MODULES; ++i) {
    vkDestroyShaderModule(p_vkrs->device, shaderStages[i].module, NULL);
  }

  return VK_SUCCESS;
}

void _mvk_find_supported_format(vk_render_state *p_vkrs, VkFormat *preferred_formats,
                                unsigned int preferred_format_count, VkImageTiling image_tiling,
                                VkFormatFeatureFlagBits features, VkFormat *result)
{
  for (unsigned int i = 0; i < preferred_format_count; ++i) {
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(p_vkrs->gpus[0], preferred_formats[i], &props);

    if (image_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
      *result = preferred_formats[i];
      return;
    }
    else if (image_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
      *result = preferred_formats[i];
      return;
    }
  }

  *result = VK_FORMAT_UNDEFINED;
}

VkResult mvk_init_depth_buffer(vk_render_state *p_vkrs)
{
  VkFormat preferred_depth_formats[] = {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                                        VK_FORMAT_D24_UNORM_S8_UINT};

  _mvk_find_supported_format(p_vkrs, preferred_depth_formats, 3, VK_IMAGE_TILING_OPTIMAL,
                             VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, &p_vkrs->depth_buffer.format);
  VK_ASSERT(p_vkrs->depth_buffer.format != VK_FORMAT_UNDEFINED, "TODO -- Couldn't find suitable depth format");

  // Color attachment
  VkImageCreateInfo imageCreateInfo = {};
  imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
  imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
  imageCreateInfo.format = p_vkrs->depth_buffer.format;
  imageCreateInfo.extent.width = p_vkrs->swap_chain.extents.width;
  imageCreateInfo.extent.height = p_vkrs->swap_chain.extents.height;
  imageCreateInfo.extent.depth = 1;
  imageCreateInfo.mipLevels = 1;
  imageCreateInfo.arrayLayers = 1;
  imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
  imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
  imageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

  VkResult res = vkCreateImage(p_vkrs->device, &imageCreateInfo, NULL, &p_vkrs->depth_buffer.image);
  VK_CHECK(res, "vkCreateImage");

  VkMemoryRequirements memReqs;
  vkGetImageMemoryRequirements(p_vkrs->device, p_vkrs->depth_buffer.image, &memReqs);

  VkMemoryAllocateInfo memAlloc = {};
  memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
  memAlloc.allocationSize = memReqs.size;
  memAlloc.memoryTypeIndex =
      mvk_get_physical_memory_type_index(p_vkrs, memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
  res = vkAllocateMemory(p_vkrs->device, &memAlloc, NULL, &p_vkrs->depth_buffer.memory);
  VK_CHECK(res, "vkAllocateMemory");
  res = vkBindImageMemory(p_vkrs->device, p_vkrs->depth_buffer.image, p_vkrs->depth_buffer.memory, 0);
  VK_CHECK(res, "vkBindImageMemory");

  VkImageViewCreateInfo colorImageView = {};
  colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
  colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
  colorImageView.format = p_vkrs->depth_buffer.format;
  colorImageView.subresourceRange = {};
  colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
  colorImageView.subresourceRange.baseMipLevel = 0;
  colorImageView.subresourceRange.levelCount = 1;
  colorImageView.subresourceRange.baseArrayLayer = 0;
  colorImageView.subresourceRange.layerCount = 1;
  colorImageView.image = p_vkrs->depth_buffer.image;
  res = vkCreateImageView(p_vkrs->device, &colorImageView, NULL, &p_vkrs->depth_buffer.view);
  VK_CHECK(res, "vkCreateImageView");

  return res;
}

VkResult mvk_init_framebuffers(vk_render_state *p_vkrs /*, bool include_depth*/)
{
  /* DEPENDS on init_depth_buffer(), init_renderpass() and
   * init_swapchain_extension() */

  VkResult res;
  VkImageView attachments[1];
  // attachments[1] = p_vkrs->depth_buffer.view;

  VkFramebufferCreateInfo fb_info = {};
  fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
  fb_info.pNext = NULL;
  fb_info.renderPass = p_vkrs->present_render_pass;
  fb_info.attachmentCount = 1;
  fb_info.pAttachments = attachments;
  fb_info.width = p_vkrs->window_width;
  fb_info.height = p_vkrs->window_height;
  fb_info.layers = 1;

  uint32_t i;

  p_vkrs->swap_chain.framebuffers = (VkFramebuffer *)malloc(p_vkrs->swap_chain.size_count * sizeof(VkFramebuffer));

  for (i = 0; i < p_vkrs->swap_chain.size_count; i++) {
    attachments[0] = p_vkrs->swap_chain.image_views[i];
    res = vkCreateFramebuffer(p_vkrs->device, &fb_info, NULL, &p_vkrs->swap_chain.framebuffers[i]);
    VK_CHECK(res, "vkCreateFramebuffer");
  }
  return res;
}

VkResult mvk_init_descriptor_pool(vk_render_state *p_vkrs)
{
  /* DEPENDS on init_uniform_buffer() and
   * init_descriptor_and_pipeline_layouts() */

  VkResult res;

  const int DESCRIPTOR_POOL_COUNT = 2;
  VkDescriptorPoolSize type_count[DESCRIPTOR_POOL_COUNT];
  type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  type_count[0].descriptorCount = 4096;
  type_count[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
  type_count[1].descriptorCount = 2048;

  VkDescriptorPoolCreateInfo descriptor_pool = {};
  descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptor_pool.pNext = NULL;
  descriptor_pool.maxSets = MAX_DESCRIPTOR_SETS;
  descriptor_pool.poolSizeCount = DESCRIPTOR_POOL_COUNT;
  descriptor_pool.pPoolSizes = type_count;

  res = vkCreateDescriptorPool(p_vkrs->device, &descriptor_pool, NULL, &p_vkrs->descriptor_pool);
  assert(res == VK_SUCCESS);
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

  res = mvk_init_swapchain_data(vkrs);
  VK_CHECK(res, "mvk_init_swapchain_data");
  res = mvk_init_headless_image(vkrs);
  VK_CHECK(res, "mvk_init_headless_image");
  res = mvk_init_depth_buffer(vkrs);
  VK_CHECK(res, "mvk_init_depth_resources");
  res = mvk_init_uniform_buffer(vkrs);
  VK_CHECK(res, "mvk_init_uniform_buffer");

  res = mvk_init_present_renderpass(vkrs);
  VK_CHECK(res, "mvk_init_present_renderpass");
  res = mvk_init_offscreen_renderpass_2d(vkrs);
  VK_CHECK(res, "mvk_init_offscreen_renderpass_2d");
  res = mvk_init_offscreen_renderpass_3d(vkrs);
  VK_CHECK(res, "mvk_init_offscreen_renderpass_3d");

  res = mvk_init_tint_render_prog(vkrs);
  VK_CHECK(res, "mvk_init_tint_render_prog");
  res = mvk_init_textured_render_prog(vkrs);
  VK_CHECK(res, "mvk_init_textured_render_prog");
  res = mvk_init_font_render_prog(vkrs);
  VK_CHECK(res, "mvk_init_font_render_prog");
  res = mvk_init_mesh_render_prog(vkrs);
  VK_CHECK(res, "mvk_init_mesh_render_prog");

  res = mvk_init_framebuffers(vkrs);
  VK_CHECK(res, "mvk_init_framebuffers");
  res = mvk_init_descriptor_pool(vkrs);
  VK_CHECK(res, "mvk_init_descriptor_pool");

  return VK_SUCCESS;
}

void mvk_destroy_framebuffers(vk_render_state *p_vkrs)
{
  for (uint32_t i = 0; i < p_vkrs->swap_chain.size_count; i++) {
    vkDestroyFramebuffer(p_vkrs->device, p_vkrs->swap_chain.framebuffers[i], NULL);
  }
  free(p_vkrs->swap_chain.framebuffers);
}

void mvk_destroy_render_prog(vk_render_state *p_vkrs, render_program *render_prog)
{
  vkDestroyPipeline(p_vkrs->device, render_prog->pipeline, NULL);
  vkDestroyDescriptorSetLayout(p_vkrs->device, render_prog->descriptor_layout, NULL);
  vkDestroyPipelineLayout(p_vkrs->device, render_prog->pipeline_layout, NULL);
}

void mvk_destroy_renderpasses(vk_render_state *p_vkrs)
{
  vkDestroyRenderPass(p_vkrs->device, p_vkrs->present_render_pass, NULL);
  vkDestroyRenderPass(p_vkrs->device, p_vkrs->offscreen_render_pass_2d, NULL);
  vkDestroyRenderPass(p_vkrs->device, p_vkrs->offscreen_render_pass_3d, NULL);
}

void mvk_destroy_uniform_buffer(vk_render_state *p_vkrs)
{
  // vkDestroyBuffer(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.buf, NULL);
  // vkFreeMemory(p_vkrs->device, p_vkrs->global_vert_uniform_buffer.mem, NULL);

  for (int i = 0; i < p_vkrs->render_data_buffer.dynamic_buffers.size; ++i) {
    mvk_dynamic_buffer_block *block = p_vkrs->render_data_buffer.dynamic_buffers.blocks[i];

    vkDestroyBuffer(p_vkrs->device, block->buffer, NULL);
    vkFreeMemory(p_vkrs->device, block->memory, NULL);
  }

  if (p_vkrs->render_data_buffer.queued_copies) {
    free(p_vkrs->render_data_buffer.queued_copies);
    p_vkrs->render_data_buffer.queued_copies = NULL;
  }
}

void mvk_destroy_depth_buffer(vk_render_state *p_vkrs)
{
  vkDestroyImageView(p_vkrs->device, p_vkrs->depth_buffer.view, NULL);
  vkDestroyImage(p_vkrs->device, p_vkrs->depth_buffer.image, NULL);
  vkFreeMemory(p_vkrs->device, p_vkrs->depth_buffer.memory, NULL);
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
    for (uint32_t i = 0; i < p_vkrs->swap_chain.size_count; i++) {
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
    vkFreeCommandBuffers(p_vkrs->device, p_vkrs->command_pool, p_vkrs->swap_chain.size_count,
                         p_vkrs->swap_chain.command_buffers);
    p_vkrs->swap_chain.command_buffers = NULL;
  }

  vkDestroySwapchainKHR(p_vkrs->device, p_vkrs->swap_chain.instance, NULL);

  p_vkrs->swap_chain.size_count = 0;
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
VkResult mvk_destroy_vulkan(vk_render_state *vkrs)
{
  vkDestroyDescriptorPool(vkrs->device, vkrs->descriptor_pool, NULL);
  mvk_destroy_framebuffers(vkrs);

  vkDestroyPipelineCache(vkrs->device, vkrs->pipelineCache, NULL);
  mvk_destroy_render_prog(vkrs, &vkrs->mesh_prog);
  mvk_destroy_render_prog(vkrs, &vkrs->font_prog);
  mvk_destroy_render_prog(vkrs, &vkrs->texture_prog);
  mvk_destroy_render_prog(vkrs, &vkrs->tint_prog);

  mvk_destroy_renderpasses(vkrs);

  // mvk_destroy_descriptor_and_pipeline_layouts(vkrs);
  mvk_destroy_uniform_buffer(vkrs);
  mvk_destroy_depth_buffer(vkrs);
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