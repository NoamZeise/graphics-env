#include "swapchain.h"
#include <stdexcept>

namespace part {
  namespace create {

    VkSurfaceFormatKHR chooseSwapchainFormat(VkPhysicalDevice physicalDevice,
					     VkSurfaceKHR surface,
					     bool srgb);
    VkExtent2D getSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities,
				  uint32_t windowWidth,
				  uint32_t windowHeight);
    VkPresentModeKHR choosePresentMode(VkPhysicalDevice physicalDevice,
				       VkSurfaceKHR surface,
				       bool vsync);
    VkCompositeAlphaFlagBitsKHR getCompositeAlpha(VkSurfaceCapabilitiesKHR surfaceCapabilities);
    
    
    std::vector<VkImage> Swapchain(VkDevice device,
				   VkPhysicalDevice physicalDevice,
				   VkSurfaceKHR surface,
				   uint32_t windowWidth,
				   uint32_t windowHeight,
				   bool vsync,
				   bool srgb,
				   VkSwapchainKHR *swapchain,
				   VkSurfaceFormatKHR *format,
				   VkExtent2D *extent) {
	
	*format = chooseSwapchainFormat(physicalDevice, surface, srgb);

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		    physicalDevice, surface, &surfaceCapabilities) != VK_SUCCESS)
	    throw std::runtime_error("In Create Swapchain: "
				     "failed to get physical device surface capabilities!");

	uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
	if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
	    imageCount = surfaceCapabilities.maxImageCount;

	*extent = getSwapchainExtent(surfaceCapabilities, windowWidth, windowHeight);

	VkPresentModeKHR presentMode = choosePresentMode(physicalDevice, surface, vsync);

	VkSurfaceTransformFlagBitsKHR preTransform =
	    surfaceCapabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR ?
	    VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR :
	    surfaceCapabilities.currentTransform;

	VkCompositeAlphaFlagBitsKHR compositeAlpha = getCompositeAlpha(surfaceCapabilities);

	VkSwapchainKHR oldSwapchain = *swapchain;
	VkSwapchainCreateInfoKHR createInfo{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
	createInfo.flags = 0;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = format->format;
	createInfo.imageColorSpace = format->colorSpace;
	createInfo.imageExtent = { extent->width, extent->height };
	createInfo.imageArrayLayers = 1;
	
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;	
	if(surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_SRC_BIT)
	    createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if(surfaceCapabilities.supportedUsageFlags & VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	    createInfo.imageUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	
	createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.queueFamilyIndexCount = 0;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = oldSwapchain;
	createInfo.compositeAlpha = compositeAlpha;
	createInfo.preTransform = preTransform;	
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, swapchain) != VK_SUCCESS)
	    throw std::runtime_error("failed to create swapchain!");

	if (oldSwapchain != VK_NULL_HANDLE)
	    vkDestroySwapchainKHR(device, oldSwapchain, nullptr);

	if(vkGetSwapchainImagesKHR(device, *swapchain, &imageCount, nullptr) != VK_SUCCESS)
	    throw std::runtime_error("failed to get swap chain image count!");
	std::vector<VkImage> swapchainImages(imageCount);
	if (vkGetSwapchainImagesKHR(
		    device, *swapchain, &imageCount, swapchainImages.data()) != VK_SUCCESS)
	    throw std::runtime_error("failed to get swap chain images!");

	return swapchainImages;
    }


    /// ----------- HELPERS -----------


    VkSurfaceFormatKHR chooseSwapchainFormat(VkPhysicalDevice physicalDevice,
					     VkSurfaceKHR surface,
					     bool srgb) {
	VkSurfaceFormatKHR f;
	//get surface formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());
	//chose a format
	if (formatCount == 0) {
	    throw std::runtime_error("Vulkan Render: In Choose Swapchain Format: "
				     "no swapchain formats were found!");
	} else if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
	    f = formats[0];
	    if(srgb)
		f.format = VK_FORMAT_R8G8B8A8_SRGB;
	    else
		f.format = VK_FORMAT_R8G8B8A8_UNORM;
	} else {
	    f.format = VK_FORMAT_UNDEFINED;
	    for (auto& format : formats) {
		switch (format.format) {
		case VK_FORMAT_R8G8B8A8_SRGB:
		    if (srgb)
			f = format;
		    break;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_B8G8R8A8_UNORM:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		    if (f.format != VK_FORMAT_R8G8B8A8_SRGB)
			f = format;
		    break;
		default:
		    continue;
		}
	    }
	    if (f.format == VK_FORMAT_UNDEFINED) {
		f = formats[0];
	    }
	}
	return f;
    }


    VkExtent2D getSwapchainExtent(VkSurfaceCapabilitiesKHR surfaceCapabilities,
				  uint32_t windowWidth,
				  uint32_t windowHeight) {
	VkExtent2D extent = {0, 0};
	if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
	    extent = surfaceCapabilities.currentExtent;
	} else {
	    extent = {windowWidth, windowHeight};
	    //clamp width
	    if (windowWidth > surfaceCapabilities.maxImageExtent.width)
		extent.width = surfaceCapabilities.maxImageExtent.width;
	    else if (windowWidth < surfaceCapabilities.minImageExtent.width)
		extent.width = surfaceCapabilities.minImageExtent.width;
	    //clamp height
	    if (windowHeight > surfaceCapabilities.maxImageExtent.height)
		extent.width = surfaceCapabilities.maxImageExtent.height;
	    else if (windowWidth < surfaceCapabilities.minImageExtent.height)
		extent.width = surfaceCapabilities.minImageExtent.height;
	}
	return extent;
    }


    VkPresentModeKHR choosePresentMode(VkPhysicalDevice physicalDevice,
				       VkSurfaceKHR surface,
				       bool vsync) {
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	uint32_t presentModeCount;
	if(vkGetPhysicalDeviceSurfacePresentModesKHR(
		   physicalDevice, surface, &presentModeCount, nullptr) != VK_SUCCESS)
	    throw std::runtime_error("failed to get physical device surface present mode count!");
	
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	if(vkGetPhysicalDeviceSurfacePresentModesKHR(
		   physicalDevice, surface, &presentModeCount, presentModes.data()) != VK_SUCCESS)
	    throw std::runtime_error("failed to get physical device surface present modes!");
	
	if(!vsync) {
	    bool modeChosen = false;
	    for (const auto& mode : presentModes) {
		if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
		    presentMode = mode;
		    modeChosen = true;
		}
		else if (!modeChosen && mode == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
		    presentMode = mode;
		    modeChosen = true;
		}
	    }
	}
	return presentMode;
    }

    VkCompositeAlphaFlagBitsKHR getCompositeAlpha(VkSurfaceCapabilitiesKHR surfaceCapabilities) {
	std::vector<VkCompositeAlphaFlagBitsKHR> priority {
	    // highest to lowest priority
	    VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
	    VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
	    VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
	    VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
	};
	for(VkCompositeAlphaFlagBitsKHR f: priority) {
	    if (surfaceCapabilities.supportedCompositeAlpha & f)
		return f;
	}
	throw std::runtime_error("Requested composite alpha flags were unsupported!");
    }
    
  } // namespace end
}
