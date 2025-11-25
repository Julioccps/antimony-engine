#include "sb.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

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
    
    if(ctx->physicalDevice == NULL){                                // If at the end,  it doesn't find a suitable one
        printf("Fatal Error: Failed to find a suitable GPU\n");     // The user is informed
        return 1;
    }

    free(devices); // Cleans the allocated memory for the devices list, now that it was already used

    uint32_t queueFamilyCount = 0;  // Asks how many family queues the device has
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueFamilyCount, NULL);
                                    // Allocates memory for the family queues of the device
    VkQueueFamilyProperties* queueFamilies = malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physicalDevice, &queueFamilyCount, queueFamilies);

    uint32_t graphicsFamily = UINT32_MAX;
    for (uint32_t i = 0; i < queueFamilyCount; i++) { // Goes through the queues looking for graphic bit queues
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamily = i;
            break;
        }
    }
    if (graphicsFamily == UINT32_MAX) {
        printf("Fatal Error: Failed to find a suitable queue family\n"); // If there's none graphic bit queue, it returns an error
        free(queueFamilies);
        return 1;
    }

    ctx->graphicsFamily =(uint32_t)graphicsFamily;                       // Else, it stores the queue found
    free(queueFamilies);
                                                                            // Priority queue, some drivers ignore, but it's required
    float queuePriority = 1.0f;                                             // to be defines | 1.0 = High priority
    VkDeviceQueueCreateInfo queueCreateInfo = {0};                          // 
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;     
    queueCreateInfo.queueFamilyIndex = ctx->graphicsFamily;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceCreateInfo = {0};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;

    if (vkCreateDevice(ctx->physicalDevice, &deviceCreateInfo, NULL, &ctx->device) != VK_SUCCESS) {
        printf("Fatal Error: Failed to create logical device\n");
        return 1;
    }
    vkGetDeviceQueue(ctx->device, ctx->graphicsFamily, 0, &ctx->queue);

    VkPhysicalDeviceProperties pProperties = {0};
    vkGetPhysicalDeviceProperties(ctx->physicalDevice, &pProperties);
    printf("Physical Device: %s\n", pProperties.deviceName);
    
    

    return 0;
}

void sb_cleanup(SbContext* ctx){
    vkDestroyDevice(ctx->device, NULL);
    vkDestroyInstance(ctx->instance, NULL);
    glfwDestroyWindow(ctx->window);         
    glfwTerminate();
}
