#include <stdio.h>
#include <stdlib.h>

#include <vulkan/vulkan.h>

typedef struct
{
    VkInstance inst;
} sample_info;

VkResult initVulkanInstance(VkInstance *inst)
{
    VkInstanceCreateInfo vk_info;

    vk_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vk_info.pNext = NULL;
    vk_info.pApplicationInfo = NULL;
    vk_info.enabledLayerCount = 0;
    vk_info.ppEnabledLayerNames = NULL;
    vk_info.enabledExtensionCount = 0;
    vk_info.ppEnabledExtensionNames = NULL;
    
    return vkCreateInstance(&vk_info, NULL, inst);
}

int createDevice()
{
    sample_info si = { .inst = 0};
    // VkInstance inst = 0;
    VkResult res;

    // res = vkCreateInstance(&vk_info, NULL, &inst);
    res = initVulkanInstance(&si.inst);

    if (res != VK_SUCCESS)
    {
        // Error!
        printf("Error %d\n", res);
        return 1;
    };

    printf("Device created: %p\n", si.inst);

    vkDestroyInstance(si.inst, NULL);
    return (EXIT_SUCCESS);
}