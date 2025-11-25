#ifndef SB_ENGINE_H
#define SB_ENGINE_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

typedef struct {
    GLFWwindow* window;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    uint32_t graphicsFamily;
    VkDevice device;
    VkQueue queue;
    // There's more to add still
} SbContext;

int sb_init(SbContext* ctx, const char* title);

void sb_cleanup(SbContext* ctx);

#endif
