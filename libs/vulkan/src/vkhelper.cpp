#include "vkhelper.h"

#include <stdexcept>
#include <mutex>
#include "logger.h"

namespace vkhelper {

  uint32_t findMemoryIndex(VkPhysicalDevice physicalDevice,
			   uint32_t memoryTypeBits,
			   VkMemoryPropertyFlags properties) {
      VkPhysicalDeviceMemoryProperties memProperties;
      vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);
      for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
	  if(memoryTypeBits & (1 << i)
	      && memProperties.memoryTypes[i].propertyFlags & properties) {
		  return i;
	      }
      }
      throw std::runtime_error("VkHelper::findMemoryIndex Error: "
			       "failed to find suitable memory type");
  }

  VkResult createBufferAndMemory(DeviceState base,
				 VkDeviceSize size,
				 VkBuffer* buffer,
				 VkDeviceMemory* memory,
				 VkBufferUsageFlags usage,
				 VkMemoryPropertyFlags properties) {
      VkBufferCreateInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, NULL};
      bufferInfo.size = size;
      bufferInfo.usage = usage;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      bufferInfo.queueFamilyIndexCount = 1;
      bufferInfo.pQueueFamilyIndices = &base.queue.graphicsPresentFamilyIndex;
      bufferInfo.flags = 0;
	
      VkResult result = vkCreateBuffer(base.device, &bufferInfo, nullptr, buffer);
      if(result != VK_SUCCESS)
	  return result;

      VkMemoryRequirements memReq;
      vkGetBufferMemoryRequirements(base.device, *buffer, &memReq);

      return allocateMemory(base.device, base.physicalDevice, memReq.size,
			    memory, properties, memReq.memoryTypeBits);
  }

  VkResult allocateMemory(VkDevice device,
			  VkPhysicalDevice physicalDevice,
			  VkDeviceSize size,
			  VkDeviceMemory* memory,
			  VkMemoryPropertyFlags properties,
			  uint32_t memoryTypeBits) {
      VkMemoryAllocateInfo memInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
      memInfo.allocationSize = size;
      memInfo.memoryTypeIndex = findMemoryIndex(physicalDevice, memoryTypeBits, properties);
      return vkAllocateMemory(device, &memInfo, nullptr, memory);
  }

  VkDeviceSize correctMemoryAlignment(VkDeviceSize desiredSize, VkDeviceSize alignment) {
      if (desiredSize % alignment != 0)
	  desiredSize = desiredSize + ((alignment - (desiredSize % alignment)) % alignment);
      return desiredSize;
  }


  VkFormat findSupportedFormat(VkPhysicalDevice physicalDevice,
			       const std::vector<VkFormat>& formats,
			       VkImageTiling tiling,
			       VkFormatFeatureFlags features) {
      for(const auto& format : formats) {
	  VkFormatProperties props;
	  vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);
	  if(tiling == VK_IMAGE_TILING_LINEAR
	     && (props.linearTilingFeatures & features) == features)
	      return format;
	  else if(tiling == VK_IMAGE_TILING_OPTIMAL
		  && (props.optimalTilingFeatures & features) == features)
	      return format;
      }
      return VK_FORMAT_UNDEFINED;
  }

  VkResult submitQueue(VkQueue queue, VkSubmitInfo* info, std::mutex* queueMut, VkFence fence) {
      if(queueMut != nullptr)
	  queueMut->lock();
      VkResult r = vkQueueSubmit(queue, 1, info, fence);
      if(queueMut != nullptr)
	  queueMut->unlock();
      return r;
  }

  VkResult submitCmdBuffAndWait(VkDevice device,
				VkQueue queue,
				VkCommandBuffer* cmdbuff,
				VkFence fence,
				std::mutex* queueSubmitMutex) {
      VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = cmdbuff;
      VkResult result = VK_SUCCESS;
      returnOnErr(submitQueue(queue, &submitInfo, queueSubmitMutex, fence));
      returnOnErr(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));
      return vkResetFences(device, 1, &fence);
  }

  void insertDebugPipelineBarrier(VkCommandBuffer cmdBuff) {
      VkMemoryBarrier barrier;
      barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT | VK_ACCESS_MEMORY_WRITE_BIT;
      vkCmdPipelineBarrier(
	      cmdBuff,
	      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	      VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
	      0,
	      1, &barrier,
	      0, nullptr,
	      0, nullptr);
  }

}//namespace end
