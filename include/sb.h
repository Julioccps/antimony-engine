#ifndef SB_ENGINE_H
#define SB_ENGINE_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

typedef struct {
	GLFWwindow* window;
	VkInstance instance;
	VkSurfaceKHR surface;
	VkPhysicalDevice physicalDevice;
	uint32_t graphicsFamily;
	uint32_t presentFamily;
	VkDevice device;
	VkQueue queue;
	VkQueue presentQueue;
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;
	VkExtent2D swapchainExetent;
	VkImage* swapchainImages;
	VkImageView* swapchainImageViews;
	uint32_t imagesCount;
} SbContext;

int sb_init(SbContext* ctx, const char* title);

void sb_cleanup(SbContext* ctx);

#endif
