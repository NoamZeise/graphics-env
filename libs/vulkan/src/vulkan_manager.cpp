#include "vulkan_manager.h"
#include "parts/core.h"
#include "parts/command.h"
#include <stdexcept>

std::mutex graphicsPresentMutex;

#define throwOnErr(result_expr, error_message)                                 \
  if (result_expr != VK_SUCCESS)                                               \
    throw std::runtime_error(error_message);

VkSampleCountFlagBits getMaxSupportedMsaaSamples(
	VkDevice device, VkPhysicalDevice physicalDevice) {
    VkSampleCountFlagBits maxMsaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    VkSampleCountFlags samplesSupported =
	props.limits.framebufferColorSampleCounts
	& props.limits.framebufferDepthSampleCounts;
    if     (samplesSupported & VK_SAMPLE_COUNT_64_BIT) maxMsaaSamples = VK_SAMPLE_COUNT_64_BIT;
    else if(samplesSupported & VK_SAMPLE_COUNT_32_BIT) maxMsaaSamples = VK_SAMPLE_COUNT_32_BIT;
    else if(samplesSupported & VK_SAMPLE_COUNT_16_BIT) maxMsaaSamples = VK_SAMPLE_COUNT_16_BIT;
    else if(samplesSupported & VK_SAMPLE_COUNT_8_BIT)  maxMsaaSamples = VK_SAMPLE_COUNT_8_BIT;
    else if(samplesSupported & VK_SAMPLE_COUNT_4_BIT)  maxMsaaSamples = VK_SAMPLE_COUNT_4_BIT;
    else if(samplesSupported & VK_SAMPLE_COUNT_2_BIT)  maxMsaaSamples = VK_SAMPLE_COUNT_2_BIT;
    return maxMsaaSamples;
}

VulkanManager::VulkanManager(GLFWwindow *window, EnabledDeviceFeatures featuresToEnable) {
    throwOnErr(part::create::Instance(&instance),
	       "Failed to create Vulkan Instance");
#ifndef NDEBUG
    throwOnErr(part::create::DebugMessenger(instance, &debugMessenger,
					    featuresToEnable.debugErrorOnly),
	       "Failed to create Debug Messenger");
#endif
    throwOnErr(glfwCreateWindowSurface(instance, window, nullptr, &windowSurface),
	       "Failed to get Window Surface From GLFW");
    throwOnErr(part::create::Device(instance, &deviceState, windowSurface, featuresToEnable),
	       "Failed to get physical device and create logical device");
    throwOnErr(part::create::CommandPoolAndBuffer(
		       deviceState.device,
		       &generalCommandPool,
		       &generalCommandBuffer,
		       deviceState.queue.graphicsPresentFamilyIndex, 0),
	       "Failed to create command pool and buffer");
    deviceState.limits.maxMsaaSamples = getMaxSupportedMsaaSamples(
	    deviceState.device,
	    deviceState.physicalDevice);
}


VulkanManager::~VulkanManager() {
    vkQueueWaitIdle(deviceState.queue.graphicsPresentQueue);

    vkDestroyCommandPool(deviceState.device, generalCommandPool, nullptr);
    vkDestroyDevice(deviceState.device, nullptr);
    vkDestroySurfaceKHR(instance, windowSurface, nullptr);
#ifndef NDEBUG
    part::destroy::DebugMessenger(instance, debugMessenger, nullptr);
#endif
    vkDestroyInstance(instance, nullptr);
}
