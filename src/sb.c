#include "sb.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vulkan/vulkan_core.h>

int sb_init(SbContext* ctx, const char* title){
	if(!glfwInit()) return 1; // Initializes glfw, for the window of the context
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Specifies to glfw to not use OpenGL
	ctx->window = glfwCreateWindow(800, 600, title, NULL, NULL); // Creates the window

	VkApplicationInfo appInfo = {0};                    // Optional struct | It's for the driver to know who the device is
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO; // Struct content | StructType
	appInfo.pApplicationName = title;                   // the title
	appInfo.apiVersion = VK_API_VERSION_1_3;            // Setting Vulkan version
						      // Vulkan doesn't have the context for the extensions needed, it's glfw's job to inform what's needed
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo = {0}; // Vulkan Instance main struct
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO; 
	createInfo.pApplicationInfo = &appInfo;

	createInfo.enabledExtensionCount = glfwExtensionCount; // Activating the extensions glfw determined as needed
	createInfo.ppEnabledExtensionNames = glfwExtensions;   // Needed, so the instance can draw on the screen

	if (vkCreateInstance(&createInfo, NULL, &ctx->instance) != VK_SUCCESS){ // Call driver to create the instance
		printf("Fatal Error: Failed while creating VkInstance\n");          // If succed the instance is stored in ctx->instance
		return 1;                                                           // Else it informs that it falied at creating the instance
	}

	if (glfwCreateWindowSurface(ctx->instance, ctx->window, NULL, &ctx->surface) != VK_SUCCESS){ // Use glfw to create the VkSurfaceKHR
		printf("Fatal Error: Failed to create Window Surface\n");                          
		return 1;
	}

	uint32_t deviceCount = 0;                                       // Asks first how many devices there are
	vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, NULL);  // 

	if (deviceCount == 0){
		printf("Fatal Error: Any GPUs not detected\n");             // If there's none it informs that there was an error   
		return 1;
	}

	VkPhysicalDevice* devices = malloc(sizeof(VkPhysicalDevice) * deviceCount); // Allocates memory for the device list
	vkEnumeratePhysicalDevices(ctx->instance, &deviceCount, devices);           // Fills it with pointers of the devices

	uint32_t bestScore = 0;                                      // Start of choosing which device to use
	for(uint32_t i = 0; i < deviceCount; i++){
		VkPhysicalDeviceProperties properties;
		vkGetPhysicalDeviceProperties(devices[i], &properties);
		uint32_t score = 0;     // Preference for Dedicated GPU
		if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
		else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) score += 500;
		else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU) score += 250;
		else if(properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU) score += 100;

		if(score > bestScore){  // If the current is better then the last one to be choose, it becomes the now best device
			bestScore = score;
			ctx->physicalDevice = devices[i];
		}
	}

	free(devices); // Cleans the allocated memory for the devices list, now that it was already use

	if(ctx->physicalDevice == NULL){                                // If at the end,  it doesn't find a suitable one
		printf("Fatal Error: Failed to find a suitable GPU\n");     // The user is informed
		return 1;
	}

	uint32_t queueFamilyCount = 0;  // Asks how many family queues the device has
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueFamilyCount, NULL);
	// Allocates memory for the family queues of the device
	VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueFamilyCount, queueFamilies);

	uint32_t graphicsFamily = UINT32_MAX;
	uint32_t presentFamily = UINT32_MAX;

	for (uint32_t i = 0; i < queueFamilyCount; i++) { // Goes through the queues looking for graphic bit queues
		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			graphicsFamily = i;
		}

		VkBool32 presentSupport = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physicalDevice, i, ctx->surface, &presentSupport);

		if (presentSupport) {
			presentFamily = i;
		}

		if (graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX) {
			break;
		}
	}

	if (graphicsFamily == UINT32_MAX || presentFamily == UINT32_MAX) {
		printf("Fatal Error: Failed to find suitable queue families\n"); // If there's none graphic bit queue, it returns an error
		free(queueFamilies);
		return 1;
	}

	ctx->graphicsFamily = graphicsFamily;
	ctx->presentFamily = presentFamily;
	free(queueFamilies);
	// Priority queue, some drivers ignore, but it's required
	float queuePriority = 1.0f;                             // to be defines | 1.0 = High priority

	VkDeviceQueueCreateInfo queueCreateInfos[2];
	uint32_t uniqueQueueFamilies[2];
	uint32_t uniqueQueueFamilyCount = 0;

	uniqueQueueFamilies[uniqueQueueFamilyCount++] = ctx->graphicsFamily;
	if (ctx->graphicsFamily != ctx->presentFamily) {
		uniqueQueueFamilies[uniqueQueueFamilyCount++] = ctx->presentFamily;
	}

	for (uint32_t i = 0; i < uniqueQueueFamilyCount; i++) {
		VkDeviceQueueCreateInfo queueCreateInfo = {0};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = uniqueQueueFamilies[i];
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos[i] = queueCreateInfo;
	}

	VkDeviceCreateInfo deviceCreateInfo = {0};                      // Begin to adress the info to create the device
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = uniqueQueueFamilyCount;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
	
	const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	deviceCreateInfo.enabledExtensionCount = 1;
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions;

	if (vkCreateDevice(ctx->physicalDevice, &deviceCreateInfo, NULL, &ctx->device) != VK_SUCCESS) { // Tries to create the logical device
		printf("Fatal Error: Failed to create logical device\n");                           // if fails, returns an error
		return 1;                                                                           // else it stores the device on the context
	}
	vkGetDeviceQueue(ctx->device, ctx->graphicsFamily, 0, &ctx->queue);     // Stores the queue to the context
	vkGetDeviceQueue(ctx->device, ctx->presentFamily, 0, &ctx->presentQueue);

	VkPhysicalDeviceProperties pProperties = {0};                    // Gets the physical device's properties to address which one is it
	vkGetPhysicalDeviceProperties(ctx->physicalDevice, &pProperties);
	printf("Physical Device: %s\n", pProperties.deviceName);

	return 0;
}

void sb_cleanup(SbContext* ctx){
	vkDestroyDevice(ctx->device, NULL);
	vkDestroySurfaceKHR(ctx->instance, ctx->surface, NULL);
	vkDestroyInstance(ctx->instance, NULL);
	glfwDestroyWindow(ctx->window);         
	glfwTerminate();
}
