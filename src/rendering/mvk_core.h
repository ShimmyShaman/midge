#ifndef MVK_CORE_H
#define MVK_CORE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <assert.h>
#include <unistd.h>

#include <xcb/xproto.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xcb.h>

static void *vk_alloc_func(void *pUserData, size_t size, size_t alignment,
                           VkSystemAllocationScope allocationScope)
{
    printf("vk_alloc(size=%lu)", size);
    return malloc(size);
}

static void *vk_realloc_func(void *pUserData, void *pOriginal, size_t size,
                             size_t alignment,
                             VkSystemAllocationScope allocationScope)
{
    printf("vk_realloc(size=%lu)", size);
    return realloc(pOriginal, size);
}

static void vk_free_func(void *pUserData, void *pMemory)
{
    printf("vk_free()");
    free(pMemory);
}

static void vk_int_alloc_notif(void *pUserData, size_t size,
                               VkInternalAllocationType allocationType,
                               VkSystemAllocationScope allocationScope)
{
}

static void vk_int_free_notif(void *pUserData, size_t size,
                              VkInternalAllocationType allocationType,
                              VkSystemAllocationScope allocationScope)
{
}

static const VkAllocationCallbacks alloc_callbacks = {
    .pUserData = NULL,
    .pfnAllocation = vk_alloc_func,
    .pfnReallocation = vk_realloc_func,
    .pfnFree = vk_free_func,
    .pfnInternalAllocation = vk_int_alloc_notif,
    .pfnInternalFree = vk_int_free_notif,
};

#endif // MVK_CORE_H