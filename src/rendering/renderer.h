/* renderer.h */

#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#include <vulkan/vulkan.h>

typedef struct
{
    pthread_t threadId;
    bool shouldExit, hasConcluded;
} mthread_info;

typedef struct
{
    VkInstance instance;
    VkDevice device;
    VkQueue queue;
    VkSurfaceKHR surface;

    VkPhysicalDevice gpu;
    VkPhysicalDeviceProperties gpu_properties;
    uint32_t graphics_family_index;
} vk_render_state;

int beginRenderThread(mthread_info *pThreadInfo);
int endRenderThread(mthread_info *pThreadInfo);

#endif // RENDERER_H