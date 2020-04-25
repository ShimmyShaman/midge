/* renderer.c */

#include <vector>

#include <vulkan/vulkan_xcb.h>

#include "rendering/renderer.h"
#include "rendering/xcbwindow.h"

int initVulkan(vk_render_state *p_vkstate);
int initVulkanInstance(std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions,
                       VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo, VkInstance *vulkanInstance);
int initDevice(vk_render_state *p_vkstate);
void deInitVulkan(vk_render_state *p_vkstate);

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *renderThread(void *vargp)
{
  printf("renderThread\n");
  mthread_info *thr = (mthread_info *)vargp;

  vk_render_state vkrs;

  // Platform
  // initPlatform();

  // Renderer
  printf("initVulkan\n");
  if (!initVulkan(&vkrs))
  {
    printf("Failed to initialize Vulkan\n");
    thr->hasConcluded = 1;
    return NULL;
  }

  // Window
  mxcb_window_info wnd;
  wnd.shouldExit = 0;
  printf("initOSWindow\n");
  initOSWindow(&wnd, 800, 480);
  // initOSSurface(&wnd, vkrs.instance, &vkrs.surface);

  // while (!thr->shouldExit && !wnd.shouldExit)
  // {
  //   usleep(1);
  //   updateOSWindow(&wnd);
  // }

  // deInitOSSurface(vkrs.instance, &vkrs.surface);
  printf("deInitOSWindow\n");
  deInitOSWindow(&wnd);

  printf("deInitVulkan\n");
  deInitVulkan(&vkrs);

  printf("hasConcluded\n");
  thr->hasConcluded = 1;
  return 0;
}

int beginRenderThread(mthread_info *pThreadInfo)
{
  printf("beginRenderThread\n");
  pThreadInfo->shouldExit = 0;
  pThreadInfo->hasConcluded = 0;
  if (pthread_create(&pThreadInfo->threadId, NULL, renderThread, (void *)pThreadInfo))
  {
    return 0;
  }
  return -1;
}

int endRenderThread(mthread_info *pThreadInfo)
{
  pThreadInfo->shouldExit = 1;

  const int MAX_ITERATIONS = 1000;
  int iterations = 0;
  while (!pThreadInfo->hasConcluded)
  {
    usleep(1000);
    ++iterations;
    if (iterations >= MAX_ITERATIONS)
    {
      printf("TODO -- Thread-Handling for unresponsive thread:: renderer.c\n");
      return -1;
    }
  }

  return 0;
}

int initVulkan(vk_render_state *p_vkstate)
{
  // Set up layers ...?
  std::vector<const char *> instanceLayers;

  // Set up extensions
  std::vector<const char *> instanceExtensions;
  instanceExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  instanceExtensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  // Set up debug
  VkDebugReportCallbackEXT debugReport = VK_NULL_HANDLE;
  VkDebugReportCallbackCreateInfoEXT debugCallbackCreateInfo;
  // setupDebug(&debugReport, &debugCallbackCreateInfo, &instanceLayers, &instanceExtensions);

  int err = initVulkanInstance(&instanceLayers, &instanceExtensions, &debugCallbackCreateInfo, &p_vkstate->instance);
  if (!err)
    return err;

  // initDebug();
  return initDevice(p_vkstate);
}

int initVulkanInstance(std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions,
                       VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo, VkInstance *p_vk_instance)
{
  VkApplicationInfo application_info;
  application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
  application_info.apiVersion = VK_MAKE_VERSION(1, 0, 2); // 1.0.2 should work on all vulkan enabled drivers.
  application_info.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
  application_info.pApplicationName = "Vulkan API Tutorial Series";

  VkInstanceCreateInfo instance_create_info;
  instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  instance_create_info.pApplicationInfo = &application_info;
  instance_create_info.enabledLayerCount = instanceLayers->size();
  instance_create_info.ppEnabledLayerNames = instanceLayers->data();
  instance_create_info.enabledExtensionCount = instanceExtensions->size();
  instance_create_info.ppEnabledExtensionNames = instanceExtensions->data();
  // instance_create_info.pNext = debugCallbackCreateInfo;

  printf("here2\n");
  vk_render_state vkrs;
  // vkrs = (vk_render_state){.instance = VkInstance()};
  vkrs.instance = VkInstance();
  VkInstance vkInstance;
  if (vkCreateInstance(&instance_create_info, NULL, &vkrs.instance) != VK_SUCCESS)
  {
    printf("Failed to create vulkan instance!\n");
    return -1;
  }
  printf("here1\n");
  return 0;
}

int initDevice(vk_render_state *p_vkstate)
{
  std::vector<const char *> device_extensions; // TODO -- does this get filled with values anywhere?

  {
    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, nullptr);
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
      // assert(0 && "Vulkan ERROR: Queue family supporting graphics not found.");
      return -1;
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

  if (vkCreateDevice(p_vkstate->gpu, &device_create_info, nullptr, &p_vkstate->device) != VK_SUCCESS)
  {
    printf("unhandled error 8258528");
    return -1;
  }

  vkGetDeviceQueue(p_vkstate->device, p_vkstate->graphics_family_index, 0, &p_vkstate->queue);
  return 0;
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