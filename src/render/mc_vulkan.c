
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
  p_vkrs->device_extension_names.items[0] = "VK_KHR_SWAPCHAIN_EXTENSION_NAME";
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

  printf("create VkInstance...");
  VkResult res = vkCreateInstance(&inst_info, NULL, &p_vkrs->instance);
  VK_CHECK(res, "vkCreateInstance");
  printf("SUCCESS %p\n", p_vkrs->instance);

  return res;
}

VkResult init_layer_device_extension_properties(vk_render_state *p_vkrs, layer_properties *layer_props)
{
  VkResult res;
  layer_props->device_extensions.items = NULL;
  char *layer_name = layer_props->properties.layerName;

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
VkResult mvk_init_enumerate_device(vk_render_state *p_vkrs)
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
  res = mvk_init_enumerate_device(vkrs);
  VK_CHECK(res, "mvk_init_enumerate_device");

  //   mxcb_init_window(&winfo, vkrs.window_width, vkrs.window_height);
  return VK_SUCCESS;
}

void mvk_destroy_enumerate_device_data(vk_render_state *p_vkrs)
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
    for (int i = 0; i < p_vkrs->instance_layer_properties.size; ++i) {
      if (p_vkrs->instance_layer_properties.items[i]) {
        // printf("%i %s : %i %i\n", i, p_vkrs->instance_layer_properties.items[i]->properties.layerName,
        //        p_vkrs->instance_layer_properties.items[i]->instance_extensions.size,
        //        p_vkrs->instance_layer_properties.items[i]->device_extensions.size);
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
  mvk_destroy_enumerate_device_data(vkrs);
  mvk_destroy_instance(vkrs);
  mvk_cleanup_device_extension_names(vkrs);
  mvk_cleanup_global_layer_properties(vkrs);

  return VK_SUCCESS;
}