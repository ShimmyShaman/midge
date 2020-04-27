/* mvk_init_util.h */

#ifndef MVK_INIT_UTIL_H
#define MVK_INIT_UTIL_H

#include <vector>

#include "rendering/mvk_core.h"

/*
 * A layer can expose extensions, keep track of those
 * extensions here.
 */
typedef struct {
    VkLayerProperties properties;
    std::vector<VkExtensionProperties> instance_extensions;
    std::vector<VkExtensionProperties> device_extensions;
} layer_properties;

VkResult init_global_layer_properties(std::vector<layer_properties> *p_vk_layers);

#endif // MVK_INIT_UTIL_H