/* renderer.cpp */

#include "rendering/renderer.h"

#define MRT_RUN(CALL)       \
  result = CALL;            \
  if (result != VK_SUCCESS) \
  {                         \
    thr->has_concluded = 1; \
    return NULL;            \
  }

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *midge_render_thread(void *vargp)
{
  // -- Arguments
  mthread_info *thr = (mthread_info *)vargp;

  // -- States
  mxcb_window_info winfo = {
      .shouldExit = 0,
  };
  vk_render_state vkrs;
  vkrs.window_width = 1024;
  vkrs.window_height = 640;
  vkrs.xcb_winfo = &winfo;

  // -- Initialization
  VkResult result;
  MRT_RUN(mvk_init_global_layer_properties(&vkrs.instance_layer_properties));
  init_device_extension_names(&vkrs);

  // -- Renderer
  MRT_RUN(mvk_init_instance(&vkrs, "midge"));
  MRT_RUN(init_enumerate_device(&vkrs, 1));
  initOSWindow(&winfo, vkrs.window_width, vkrs.window_height);

  // -- Update
  while (!thr->should_exit && !winfo.shouldExit)
  {
    usleep(1);
    updateOSWindow(&winfo);
  }

  // -- Cleanup
  deInitOSWindow(&winfo);
  vkDestroyInstance(vkrs.inst, NULL);
  printf("hasConcluded(SUCCESS)\n");
  thr->has_concluded = 1;
  return 0;

  // deInitOSSurface(vkrs.instance, &vkrs.surface);
  // printf("deInitOSWindow\n");
  // deInitOSWindow(&wnd);

  // printf("deInitVulkan\n");
  // deInitVulkan(&vkrs);

  // printf("hasConcluded\n");
  // thr->has_concluded = 1;
  // return 0;
}

// VkResult vk_init_layers_extensions(std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions)
// {
//   // Set up extensions
//   instanceExtensions->push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
//   instanceExtensions->push_back(VK_KHR_SURFACE_EXTENSION_NAME);

//   return VK_SUCCESS;
// }

// VkResult initVulkan(vk_render_state *p_vkrs, mxcb_window_info *p_wnfo)
// {
//   VkResult res;

//   // -- Application Info --
//   VkApplicationInfo application_info;
//   application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
//   // application_info.apiVersion = VK_MAKE_VERSION(1, 0, 2); // 1.0.2 should work on all vulkan enabled drivers.
//   // application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
//   application_info.pApplicationName = "Vulkan API Tutorial Series";

//   // -- Layers & Extensions --
//   std::vector<const char *> instanceLayers;
//   std::vector<const char *> instanceExtensions;
//   // vk_init_layers_extensions(&instanceLayers, &instanceExtensions);
//   instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
//   instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

//   // -- Debug --
//   VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
//   VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
//   // setupDebug(&debugReport, &debugCallbackCreateInfo, &instanceLayers, &instanceExtensions);

//   // -- VK Instance --
//   VkInstanceCreateInfo instance_create_info;
//   instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
//   instance_create_info.pApplicationInfo = &application_info;
//   instance_create_info.enabledLayerCount = instanceLayers.size();
//   instance_create_info.ppEnabledLayerNames = instanceLayers.data();
//   instance_create_info.enabledExtensionCount = instanceExtensions.size();
//   instance_create_info.ppEnabledExtensionNames = instanceExtensions.data();
//   // instance_create_info.pNext = debugCallbackCreateInfo;

//   // printf("vkinst=%p\n", p_vkstate->instance);
//   printf("aboutToVkCreateInstance()\n");
//   res = vkCreateInstance(&instance_create_info, NULL, &p_vkrs->instance);
//   if (res != VK_SUCCESS)
//   {
//     printf("Failed to create vulkan instance!\n");
//     return res;
//   }
//   printf("vkCreateInstance(SUCCESS)\n");

//   // initDebug();
//   res = initDevice(p_vkrs);
//   if (res != VK_SUCCESS)
//   {
//     printf("Failed to create vulkan instance!\n");
//     return res;
//   }

//   // Window
//   // printf("initOSWindow\n");
//   initOSWindow(p_wnfo, 800, 480);
//   initOSSurface(p_wnfo, p_vkrs->instance, &p_vkrs->surface);
//   // init_swapchain_extension(p_vkrs, p_wnfo);
//   return VK_SUCCESS;
// }

// VkResult initDevice(vk_render_state *p_vkstate)
// {
//   VkResult res;
//   std::vector<const char *> device_extensions; // TODO -- does this get filled with values anywhere?

//   {
//     uint32_t gpu_count = 0;
//     vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, NULL);
//     std::vector<VkPhysicalDevice> gpu_list(gpu_count);
//     vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, gpu_list.data());
//     p_vkstate->gpu = gpu_list[0];
//     vkGetPhysicalDeviceProperties(p_vkstate->gpu, &p_vkstate->gpu_properties);
//   }
//   {
//     uint32_t family_count = 0;
//     vkGetPhysicalDeviceQueueFamilyProperties(p_vkstate->gpu, &family_count, nullptr);
//     std::vector<VkQueueFamilyProperties> family_property_list(family_count);
//     vkGetPhysicalDeviceQueueFamilyProperties(p_vkstate->gpu, &family_count, family_property_list.data());

//     bool found = false;
//     for (uint32_t i = 0; i < family_count; ++i)
//     {
//       if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
//       {
//         found = true;
//         p_vkstate->graphics_family_index = i;
//       }
//     }
//     if (!found)
//     {
//       printf("Vulkan ERROR: Queue family supporting graphics not found.\n");
//       return VK_NOT_READY;
//     }
//   }

//   float queue_priorities[]{1.0f};
//   VkDeviceQueueCreateInfo device_queue_create_info{};
//   device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//   device_queue_create_info.queueFamilyIndex = p_vkstate->graphics_family_index;
//   device_queue_create_info.queueCount = 1;
//   device_queue_create_info.pQueuePriorities = queue_priorities;

//   VkDeviceCreateInfo device_create_info{};
//   device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//   device_create_info.queueCreateInfoCount = 1;
//   device_create_info.pQueueCreateInfos = &device_queue_create_info;
//   //	device_create_info.enabledLayerCount		= _device_layers.size();				// depricated
//   //	device_create_info.ppEnabledLayerNames		= _device_layers.data();				// depricated
//   device_create_info.enabledExtensionCount = device_extensions.size();
//   device_create_info.ppEnabledExtensionNames = device_extensions.data();

//   res = vkCreateDevice(p_vkstate->gpu, &device_create_info, nullptr, &p_vkstate->device);
//   if (res != VK_SUCCESS)
//   {
//     printf("unhandled error 8258528");
//     return res;
//   }

//   vkGetDeviceQueue(p_vkstate->device, p_vkstate->graphics_family_index, 0, &p_vkstate->queue);
//   return VK_SUCCESS;
// }

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

// void deInitVulkan(vk_render_state *p_vkrs)
// {
//   vkDestroyDevice(p_vkrs->device, NULL);
//   p_vkrs->device = NULL;

//   // deInitDebug() TODO

//   vkDestroyInstance(p_vkrs->instance, NULL);
//   p_vkrs->instance = NULL;
// }