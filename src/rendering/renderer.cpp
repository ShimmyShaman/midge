/* renderer.c */

#include <vector>

#include "renderer.h"
#include "xcbwindow.h"

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *renderThread(void *vargp)
{
  mthread_info *thr = (mthread_info *)vargp;

  mrender_info rnd;

  // Platform
  // initPlatform();

  // Renderer
  initVulkan();

  // Window
  initOSWindow();
  initOSSurface();

  _UpdateOSWindow();

  while (!thr->shouldExit)
    usleep(1);

  thr->hasConcluded = 1;
  return 0;
}

int beginRenderThread(mthread_info **pThreadInfo)
{
  if ((*pThreadInfo = (mthread_info *)malloc(sizeof *pThreadInfo)) != NULL)
  {
    (*pThreadInfo)->shouldExit = 0;
    (*pThreadInfo)->hasConcluded = 0;

    pthread_create(&(*pThreadInfo)->threadId, NULL, renderThread, (void *)*pThreadInfo);

    return 0;
  }
  return -1;
}

int endRenderThread(mthread_info *pThreadInfo)
{
  pThreadInfo->shouldExit = 1;

  const int MAX_ITERATIONS = 500000;
  int iterations = 0;
  while (!pThreadInfo->hasConcluded)
  {
    usleep(1);
    ++iterations;
    if (iterations >= MAX_ITERATIONS)
    {
      printf("TODO -- Thread-Handling for unresponsive thread:: renderer.c");
      return -1;
    }
  }

  free(pThreadInfo);
  return 0;
}

int initVulkan()
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

  VkInstance *vkInstance;
  initVulkanInstance(&instanceLayers, &instanceExtensions, &debugCallbackCreateInfo, &vkInstance);
  // initDebug();
  initDevice();
}

void initVulkanInstance(std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions,
                        VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo, VkInstance **vulkanInstance)
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
  instance_create_info.pNext = debugCallbackCreateInfo;

  if (vkCreateInstance(&instance_create_info, nullptr, *vulkanInstance) != VK_SUCCESS)
  {
    throw 114;
  }
}

void initDevice(vk_render_state *p_vkstate)
{
  VkPhysicalDevice gpu;
  VkPhysicalDeviceProperties gpu_properties;
  uint32_t graphics_family_index;
  std::vector<const char *> device_extensions; // TODO -- does this get filled with values anywhere?

  {
    uint32_t gpu_count = 0;
    vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, nullptr);
    std::vector<VkPhysicalDevice> gpu_list(gpu_count);
    vkEnumeratePhysicalDevices(p_vkstate->instance, &gpu_count, gpu_list.data());
    gpu = gpu_list[0];
    vkGetPhysicalDeviceProperties(gpu, &gpu_properties);
  }
  {
    uint32_t family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, nullptr);
    std::vector<VkQueueFamilyProperties> family_property_list(family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(gpu, &family_count, family_property_list.data());

    bool found = false;
    for (uint32_t i = 0; i < family_count; ++i)
    {
      if (family_property_list[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
      {
        found = true;
        graphics_family_index = i;
      }
    }
    if (!found)
    {
      // assert(0 && "Vulkan ERROR: Queue family supporting graphics not found.");
      std::exit(-1);
    }
  }

  float queue_priorities[]{1.0f};
  VkDeviceQueueCreateInfo device_queue_create_info{};
  device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
  device_queue_create_info.queueFamilyIndex = graphics_family_index;
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

  if(vkCreateDevice(gpu, &device_create_info, nullptr, &p_vkstate->device) != VK_SUCCESS)
  {
    printf("unhandled error 8258528");
    return;
  }

  vkGetDeviceQueue(device, graphics_family_index, 0, &queue);
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