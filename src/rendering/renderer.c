/* renderer.c */


#include "renderer.h"

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *renderThread(void *vargp)
{
  mthread_info *thr = (mthread_info *)vargp;

  mrender_info rnd;

  // Platform
  initPlatform();
  // Renderer
  initVulkan();
  // Window
  _InitOSWindow();
  _InitSurface();

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
  setupDebug(&debugReport, &debugCallbackCreateInfo, &instanceLayers, &instanceExtensions);

  initVulkanInstance();
  _InitInstance();
  _InitDebug();
  _InitDevice();
}

void initVulkanInstance(){
  
}

void setupDebug(VkDebugReportCallbackEXT *debugReport, VkDebugReportCallbackCreateInfoEXT *debugCallbackCreateInfo, std::vector<const char *> *instanceLayers, std::vector<const char *> *instanceExtensions)
{
  debugCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
  debugCallbackCreateInfo.pfnCallback = VulkanDebugCallback;
  debugCallbackCreateInfo.flags =
      //		VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
      VK_DEBUG_REPORT_WARNING_BIT_EXT |
      VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT |
      VK_DEBUG_REPORT_ERROR_BIT_EXT |
      //		VK_DEBUG_REPORT_DEBUG_BIT_EXT |
      0;

  instanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");
  /*
//	_instanceLayers.push_back( "VK_LAYER_LUNARG_threading" );
	_instanceLayers.push_back( "VK_LAYER_GOOGLE_threading" );
	_instanceLayers.push_back( "VK_LAYER_LUNARG_draw_state" );
	_instanceLayers.push_back( "VK_LAYER_LUNARG_image" );
	_instanceLayers.push_back( "VK_LAYER_LUNARG_mem_tracker" );
	_instanceLayers.push_back( "VK_LAYER_LUNARG_object_tracker" );
	_instanceLayers.push_back( "VK_LAYER_LUNARG_param_checker" );
	*/
  instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

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
}

VKAPI_ATTR VkBool32 VKAPI_CALL
VulkanDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t src_obj,
    size_t location,
    int32_t msg_code,
    const char *layerPrefix,
    const char *msg,
    void *user_data)
{
  printf("VKDBG: ");
  if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
  {
    printf("INFO: ");
  }
  if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
  {
    printf("WARNING: ");
  }
  if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
  {
    printf("PERFORMANCE: ");
  }
  if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
  {
    printf("ERROR: ");
  }
  if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
  {
    printf("DEBUG: ");
  }
  printf("@[%s]: ", layerPrefix);
  printf("%s\n", msg);

  return false;
}