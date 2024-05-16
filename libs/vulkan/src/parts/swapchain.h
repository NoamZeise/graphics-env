#ifndef PARTS_SWAPCHAIN_H
#define PARTS_SWAPCHAIN_H

#include <volk.h>
#include <vector>

namespace part {
  namespace create {

    // Returns swapchain images
    std::vector<VkImage> Swapchain(
	    VkDevice device,
	    VkPhysicalDevice physicalDevice,
	    VkSurfaceKHR surface,
	    uint32_t windowWidth,
	    uint32_t windowHeight,
	    bool vsync,
	    bool srgb,
	    VkSwapchainKHR *swapchain,
	    VkSurfaceFormatKHR *format,
	    VkExtent2D *extent);
    
  }
}

#endif
