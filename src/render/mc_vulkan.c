
#include "render/mc_vulkan.h"
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

/*
 * Initializes a discovered global extension property.
 */
VkResult mvk_init_global_extension_properties(layer_properties *layer_props)
{
  VkResult res;
  char *layer_name = layer_props->properties.layerName;
  layer_props->instance_extensions.items = NULL;

  // Function has same format as mvk_init_global_layer_properties
  // for maybe the same reason??? Donno vulkan that well
  do {
    res = vkEnumerateInstanceExtensionProperties(layer_name, &layer_props->instance_extensions.size, NULL);
    if (res) {
      return res;
    }

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
    if (res)
      return res;

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
    layer_properties *layer_props = (layer_properties *)malloc(sizeof(layer_properties));
    p_vkrs->instance_layer_properties.items[i] = layer_props;
    layer_props->properties = vk_props[i];
    // printf("vk_props[%i]:%s-%s\n", i, vk_props[i].layerName, vk_props[i].description);
    res = mvk_init_global_extension_properties(layer_props);
    if (res)
      return res;
  }
  free(vk_props);

  return res;
}

void mvk_init_device_extension_names(vk_render_state *p_vkrs)
{
  p_vkrs->device_extension_names = (const char **)malloc(sizeof(const char *) * 1);
  p_vkrs->device_extension_names[0] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
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
  p_vkrs->instance_layer_names.push_back("VK_LAYER_KHRONOS_validation");
  // p_vkrs->instance_extension_names.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
  // p_vkrs->instance_extension_names.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
  p_vkrs->instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

  mvk_check_layer_support(p_vkrs);

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
VkResult mvk_init_vulkan(vk_render_state *vkrs)
{
  mvk_init_global_layer_properties(vkrs);
  mvk_init_device_extension_names(vkrs);
  mvk_init_instance(&result, &vkrs, "midge")

      return VK_SUCCESS;
}

void mvk_cleanup_device_extension_names(vk_render_state *p_vkrs) { free(p_vkrs->device_extension_names); }

void mvk_cleanup_global_layer_properties(vk_render_state *p_vkrs)
{
  if (p_vkrs->instance_layer_properties.items) {
    for (int i = 0; i < p_vkrs->instance_layer_properties.size; ++i) {
      if (p_vkrs->instance_layer_properties.items[i]) {
        if (p_vkrs->instance_layer_properties.items[i]->instance_extensions.items) {
          free(p_vkrs->instance_layer_properties.items[i]->instance_extensions.items);
        }
        if (p_vkrs->instance_layer_properties.items[i]->device_extensions.items) {
          free(p_vkrs->instance_layer_properties.items[i]->device_extensions.items);
        }

        free(p_vkrs->instance_layer_properties.items[i]);
      }
    }
    free(p_vkrs->instance_layer_properties.items);
  }
}

VkResult mvk_cleanup_vulkan(vk_render_state *vkrs)
{
  mvk_cleanup_device_extension_names(vkrs);
  mvk_cleanup_global_layer_properties(vkrs);

  return VK_SUCCESS;
}