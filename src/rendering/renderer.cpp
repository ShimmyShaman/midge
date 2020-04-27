/* renderer.cpp */

#include "rendering/renderer.h"

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *midge_render_thread(void *vargp)
{
  mthread_info *thr = (mthread_info *)vargp;

  VkResult result;
  std::vector<layer_properties> vk_layers;
  result = init_global_layer_properties(&vk_layers);
  if (result != VK_SUCCESS)
  {
    printf("midge_render_thread> init_global_layer_properties error");
    return NULL;
  }

  vk_render_state vkrs = {
      .instance = NULL,
  };
  mxcb_window_info wnd = {
      .shouldExit = 0,
  };

  // Renderer
  printf("initVulkan\n");
  if (initVulkan(&vkrs, &wnd))
  {
    printf("Failed to initialize Vulkan\n");
    thr->has_concluded = 1;
    return NULL;
  }

  while (!thr->should_exit && !wnd.shouldExit)
  {
    usleep(1);
    updateOSWindow(&wnd);
  }

  deInitOSSurface(vkrs.instance, &vkrs.surface);
  printf("deInitOSWindow\n");
  deInitOSWindow(&wnd);

  printf("deInitVulkan\n");
  deInitVulkan(&vkrs);

  printf("hasConcluded\n");
  thr->has_concluded = 1;
  return 0;
}

VkResult vk_init_layers_extensions(std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions)
{
  // Set up extensions
  instanceExtensions->push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  instanceExtensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  return VK_SUCCESS;
}
/*
https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#vkCreateInstance
https://github.com/LunarG/VulkanSamples/blob/master/API-Samples/utils/util_init.cpp
https://github.com/LunarG/VulkanSamples/blob/master/API-Samples/15-draw_cube/15-draw_cube.cpp
*/
// VkResult init_enumerate_device(struct sample_info &info, uint32_t gpu_count) {
//     uint32_t const U_ASSERT_ONLY req_count = gpu_count;
//     VkResult res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, NULL);
//     assert(gpu_count);
//     info.gpus.resize(gpu_count);

//     res = vkEnumeratePhysicalDevices(info.inst, &gpu_count, info.gpus.data());
//     assert(!res && gpu_count >= req_count);

//     vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0], &info.queue_family_count, NULL);
//     assert(info.queue_family_count >= 1);

//     info.queue_props.resize(info.queue_family_count);
//     vkGetPhysicalDeviceQueueFamilyProperties(info.gpus[0], &info.queue_family_count, info.queue_props.data());
//     assert(info.queue_family_count >= 1);

//     /* This is as good a place as any to do this */
//     vkGetPhysicalDeviceMemoryProperties(info.gpus[0], &info.memory_properties);
//     vkGetPhysicalDeviceProperties(info.gpus[0], &info.gpu_props);
//     /* query device extensions for enabled layers */
//     for (auto &layer_props : info.instance_layer_properties) {
//         init_device_extension_properties(info, layer_props);
//     }

//     return res;
// }

// void init_swapchain_extension(vk_render_state *p_vkrs, mxcb_window_info *p_mwinfo)
// {
//   VkResult res;

//   // Iterate over each queue to learn whether it supports presenting:
//   VkBool32 *pSupportsPresent = (VkBool32 *)malloc(p_vkrs-> queue_family_count * sizeof(VkBool32));
//   for (uint32_t i = 0; i < info.queue_family_count; i++)
//   {
//     vkGetPhysicalDeviceSurfaceSupportKHR(info.gpus[0], i, info.surface, &pSupportsPresent[i]);
//   }

//   // Search for a graphics and a present queue in the array of queue
//   // families, try to find one that supports both
//   info.graphics_queue_family_index = UINT32_MAX;
//   info.present_queue_family_index = UINT32_MAX;
//   for (uint32_t i = 0; i < info.queue_family_count; ++i)
//   {
//     if ((info.queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
//     {
//       if (info.graphics_queue_family_index == UINT32_MAX)
//         info.graphics_queue_family_index = i;

//       if (pSupportsPresent[i] == VK_TRUE)
//       {
//         info.graphics_queue_family_index = i;
//         info.present_queue_family_index = i;
//         break;
//       }
//     }
//   }

//   if (info.present_queue_family_index == UINT32_MAX)
//   {
//     // If didn't find a queue that supports both graphics and present, then
//     // find a separate present queue.
//     for (size_t i = 0; i < info.queue_family_count; ++i)
//       if (pSupportsPresent[i] == VK_TRUE)
//       {
//         info.present_queue_family_index = i;
//         break;
//       }
//   }
//   free(pSupportsPresent);

//   // Generate error if could not find queues that support graphics
//   // and present
//   if (info.graphics_queue_family_index == UINT32_MAX || info.present_queue_family_index == UINT32_MAX)
//   {
//     std::cout << "Could not find a queues for both graphics and present";
//     exit(-1);
//   }

//   // Get the list of VkFormats that are supported:
//   uint32_t formatCount;
//   res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, NULL);
//   assert(res == VK_SUCCESS);
//   VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
//   res = vkGetPhysicalDeviceSurfaceFormatsKHR(info.gpus[0], info.surface, &formatCount, surfFormats);
//   assert(res == VK_SUCCESS);
//   // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
//   // the surface has no preferred format.  Otherwise, at least one
//   // supported format will be returned.
//   if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
//   {
//     info.format = VK_FORMAT_B8G8R8A8_UNORM;
//   }
//   else
//   {
//     assert(formatCount >= 1);
//     info.format = surfFormats[0].format;
//   }
//   free(surfFormats);
// }

VkResult initVulkan(vk_render_state *p_vkrs, mxcb_window_info *p_wnfo)
{
  VkResult res;

  // -- Application Info --
  VkApplicationInfo application_info;
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  // application_info.apiVersion = VK_MAKE_VERSION(1, 0, 2); // 1.0.2 should work on all vulkan enabled drivers.
  // application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  application_info.pApplicationName = "Vulkan API Tutorial Series";

  // -- Layers & Extensions --
  std::vector<const char *> instanceLayers;
  std::vector<const char *> instanceExtensions;
  // vk_init_layers_extensions(&instanceLayers, &instanceExtensions);
  instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  // -- Debug --
  VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
  VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
  // setupDebug(&debugReport, &debugCallbackCreateInfo, &instanceLayers, &instanceExtensions);

  // -- VK Instance --
  VkInstanceCreateInfo instance_create_info;
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &application_info;
  instance_create_info.enabledLayerCount = instanceLayers.size();
  instance_create_info.ppEnabledLayerNames = instanceLayers.data();
  instance_create_info.enabledExtensionCount = instanceExtensions.size();
  instance_create_info.ppEnabledExtensionNames = instanceExtensions.data();
  // instance_create_info.pNext = debugCallbackCreateInfo;

  // printf("vkinst=%p\n", p_vkstate->instance);
  printf("aboutToVkCreateInstance()\n");
  res = vkCreateInstance(&instance_create_info, NULL, &p_vkrs->instance);
  if (res != VK_SUCCESS)
  {
    printf("Failed to create vulkan instance!\n");
    return res;
  }
  printf("vkCreateInstance(SUCCESS)\n");

  // initDebug();
  res = initDevice(p_vkrs);
  if (res != VK_SUCCESS)
  {
    printf("Failed to create vulkan instance!\n");
    return res;
  }

  // Window
  // printf("initOSWindow\n");
  initOSWindow(p_wnfo, 800, 480);
  initOSSurface(p_wnfo, p_vkrs->instance, &p_vkrs->surface);
  // init_swapchain_extension(p_vkrs, p_wnfo);
  return VK_SUCCESS;
}

VkResult initDevice(vk_render_state *p_vkstate)
{
  VkResult res;
  std::vector<const char *> device_extensions; // TODO -- does this get filled with values anywhere?

  {
    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, NULL);
    std::vector<VkPhysicalDevice> gpu_list(gpu_count);
    vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, gpu_list.data());
    p_vkstate->gpu = gpu_list[0];
    vkGetPhysicalDeviceProperties(p_vkstate->gpu, &p_vkstate->gpu_properties);
  }
  {
    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(p_vkstate->gpu, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> family_property_list(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(p_vkstate->gpu, &family_count, family_property_list.data());

    bool found = false;
    for (uint32_t i = 0; i < family_count; ++i)
    {
      if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        found = true;
        p_vkstate->graphics_family_index = i;
      }
    }
    if (!found)
    {
      printf("Vulkan ERROR: Queue family supporting graphics not found.\n");
      return VK_NOT_READY;
    }
  }

  float queue_priorities[]{1.0f};
  VkDeviceQueueCreateInfo device_queue_create_info{};
  device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  device_queue_create_info.queueFamilyIndex = p_vkstate->graphics_family_index;
  device_queue_create_info.queueCount = 1;
  device_queue_create_info.pQueuePriorities = queue_priorities;

  VkDeviceCreateInfo device_create_info{};
  device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
  device_create_info.queueCreateInfoCount = 1;
  device_create_info.pQueueCreateInfos = &device_queue_create_info;
  //	device_create_info.enabledLayerCount		= _device_layers.size();				// depricated
  //	device_create_info.ppEnabledLayerNames		= _device_layers.data();				// depricated
  device_create_info.enabledExtensionCount = device_extensions.size();
  device_create_info.ppEnabledExtensionNames = device_extensions.data();

  res = vkCreateDevice(p_vkstate->gpu, &device_create_info, nullptr, &p_vkstate->device);
  if (res != VK_SUCCESS)
  {
    printf("unhandled error 8258528");
    return res;
  }

  vkGetDeviceQueue(p_vkstate->device, p_vkstate->graphics_family_index, 0, &p_vkstate->queue);
  return VK_SUCCESS;
}

// void setupDebug(VkDebugReportCallbackEXT *debugReport, VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo, std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions)
// {
//   debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
//   debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
//   debugCallbackCreateInfo.flags =
//       //		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
//       VK_DEBUG_REPORT_WARNING_BIT_EXT |
//       VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
//       VK_DEBUG_REPORT_ERROR_BIT_EXT |
//       //		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
//       0;

//   instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
//   /*
// //	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_threading" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_GOOGLE_threading" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_draw_state" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_image" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_object_tracker" );
// 	vulkanInstanceLayers.push_back( "VK_LAYER_LUNARG_param_checker" );
// 	*/
//   instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

//   //	_device_layers.push_back( "VK_LAYER_LUNARG_standard_validation" );				// depricated
//   /*
// //	_device_layers.push_back( "VK_LAYER_LUNARG_threading" );
// 	_device_layers.push_back( "VK_LAYER_GOOGLE_threading" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_draw_state" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_image" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_object_tracker" );
// 	_device_layers.push_back( "VK_LAYER_LUNARG_param_checker" );
// 	*/
// }

// VKAPI_ATTR VkBool32 VKAPI_CALL
// VulkanDebugCallback(
//     VkDebugReportFlagsEXT flags,
//     VkDebugReportObjectTypeEXT obj_type,
//     uint64_t src_obj,
//     size_t location,
//     int32_t msg_code,
//     const char *layerPrefix,
//     const char *msg,
//     void *user_data)
// {
//   printf("VKDBG: ");
//   if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
//   {
//     printf("INFO: ");
//   }
//   if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
//   {
//     printf("WARNING: ");
//   }
//   if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
//   {
//     printf("PERFORMANCE: ");
//   }
//   if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
//   {
//     printf("ERROR: ");
//   }
//   if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
//   {
//     printf("DEBUG: ");
//   }
//   printf("@[%s]: ", layerPrefix);
//   printf("%s\n", msg);

//   return false;
// }

void deInitVulkan(vk_render_state *p_vkrs)
{
  vkDestroyDevice(p_vkrs->device, NULL);
  p_vkrs->device = NULL;

  // deInitDebug() TODO

  vkDestroyInstance(p_vkrs->instance, NULL);
  p_vkrs->instance = NULL;
}