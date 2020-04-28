/* mvk_init_util.c */

#include "rendering/mvk_init_util.h"

/*
 * TODO: function description here
 */
VkResult mvk_init_global_extension_properties(layer_properties &layer_props)
{
  VkExtensionProperties *instance_extensions;
  uint32_t instance_extension_count;
  VkResult res;
  char *layer_name = NULL;

  layer_name = layer_props.properties.layerName;

  do
  {
    res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, NULL);
    if (res)
      return res;

    if (instance_extension_count == 0)
    {
      return VK_SUCCESS;
    }

    layer_props.instance_extensions.resize(instance_extension_count);
    instance_extensions = layer_props.instance_extensions.data();
    res = vkEnumerateInstanceExtensionProperties(layer_name, &instance_extension_count, instance_extensions);
  } while (res == VK_INCOMPLETE);

  return res;
}

/*
 * TODO: function description here
 */
VkResult mvk_init_global_layer_properties(std::vector<layer_properties> *p_vk_layers)
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
  do
  {
    res = vkEnumerateInstanceLayerProperties(&instance_layer_count, NULL);
    if (res)
      return res;

    if (instance_layer_count == 0)
    {
      return VK_SUCCESS;
    }

    vk_props = (VkLayerProperties *)realloc(vk_props, instance_layer_count * sizeof(VkLayerProperties));

    res = vkEnumerateInstanceLayerProperties(&instance_layer_count, vk_props);
  } while (res == VK_INCOMPLETE);

  /*
     * Now gather the extension list for each instance layer.
     */
  for (uint32_t i = 0; i < instance_layer_count; i++)
  {
    layer_properties layer_props;
    layer_props.properties = vk_props[i];
    res = mvk_init_global_extension_properties(layer_props);
    if (res)
      return res;
    p_vk_layers->push_back(layer_props);
  }
  free(vk_props);

  return res;
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

  // -- Layers & Extensions --
  std::vector<const char *> instance_layers;
  std::vector<const char *> instance_extensions;
  instance_extensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
  instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);

  VkInstanceCreateInfo inst_info = {};
  inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
  inst_info.pNext = NULL;
  inst_info.flags = 0;
  inst_info.pApplicationInfo = &app_info;
  inst_info.enabledLayerCount = instance_layers.size();
  inst_info.ppEnabledLayerNames = instance_layers.size() ? instance_layers.data() : NULL;
  inst_info.enabledExtensionCount = instance_extensions.size();
  inst_info.ppEnabledExtensionNames = instance_extensions.data();

  printf("aboutToVkCreateInstance()\n");
  VkResult res = vkCreateInstance(&inst_info, NULL, &p_vkrs->inst);
  assert(res == VK_SUCCESS);
  printf("vkCreateInstance(SUCCESS)\n");

  return res;
}

void init_device_extension_names(vk_render_state *p_vkrs)
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

  do
  {
    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &device_extension_count, NULL);
    if (res)
      return res;

    if (device_extension_count == 0)
    {
      return VK_SUCCESS;
    }

    layer_props.device_extensions.resize(device_extension_count);
    device_extensions = layer_props.device_extensions.data();
    res = vkEnumerateDeviceExtensionProperties(p_vkrs->gpus[0], layer_name, &device_extension_count, device_extensions);
  } while (res == VK_INCOMPLETE);

  return res;
}

VkResult init_enumerate_device(vk_render_state *p_vkrs, const uint32_t required_gpu_count)
{
  uint32_t gpu_count = required_gpu_count;
  VkResult res = vkEnumeratePhysicalDevices(p_vkrs->inst, &gpu_count, NULL);
  assert(gpu_count);
  p_vkrs->gpus.resize(gpu_count);

  res = vkEnumeratePhysicalDevices(p_vkrs->inst, &gpu_count, p_vkrs->gpus.data());
  assert(!res && gpu_count >= required_gpu_count);

  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, NULL);
  assert(p_vkrs->queue_family_count >= 1);

  p_vkrs->queue_props.resize(p_vkrs->queue_family_count);
  vkGetPhysicalDeviceQueueFamilyProperties(p_vkrs->gpus[0], &p_vkrs->queue_family_count, p_vkrs->queue_props.data());
  assert(p_vkrs->queue_family_count >= 1);

  /* This is as good a place as any to do this */
  vkGetPhysicalDeviceMemoryProperties(p_vkrs->gpus[0], &p_vkrs->memory_properties);
  vkGetPhysicalDeviceProperties(p_vkrs->gpus[0], &p_vkrs->gpu_props);
  /* query device extensions for enabled layers */
  for (auto &layer_props : p_vkrs->instance_layer_properties)
  {
    init_device_extension_properties(p_vkrs, layer_props);
  }

  return res;
}