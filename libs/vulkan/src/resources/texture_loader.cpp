#include "texture_loader.h"

#include <cstring>
#include "../logger.h"
#include "../vkhelper.h"
#include "../parts/images.h"
#include "../parts/command.h"
#include "../parts/threading.h"


const VkFilter MIPMAP_FILTER = VK_FILTER_LINEAR;

struct StagedTexVk : public StagedTex {
    void deleteData() override {}
    TextureInfoVk info;
};

struct TextureInGPU {
    TextureInGPU(VkDevice device, StagedTex *tex, TextureInfoVk info) {
	this->device = device;
	this->info = info;
	this->width = tex->width;
	this->height = tex->height;
	gpuOnly = tex->filesize == 0;
	currentImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	currentImageAccessMask = 0;
	if(!tex->internalTex) {
	    if(tex->nrChannels != 4)
		throw std::runtime_error("GPU Tex has unsupport no. of channels!");
	}
    }
    
    ~TextureInGPU() {
	vkDestroyImageView(device, view, nullptr);
	vkDestroyImage(device, image, nullptr);	  
    }
    
    VkDevice device;
    uint32_t imageViewIndex = 0;
    uint32_t width;
    uint32_t height;
    TextureInfoVk info;
    VkImageLayout currentImageLayout;
    VkAccessFlags currentImageAccessMask;

    VkImage image;
    VkImageView view;
    VkDeviceSize imageMemSize;
    VkDeviceSize imageMemOffset;
    bool gpuOnly;
    
    VkResult createImage(VkDevice device, VkMemoryRequirements *pMemreq);
    void createMipMaps(VkCommandBuffer &cmdBuff);
    VkResult createImageView(VkDevice device);
};

  
TexLoaderVk::TexLoaderVk(DeviceState base, VkCommandPool cmdpool,
			 Resource::Pool pool, RenderConfig config)
    : InternalTexLoader(pool, config) {
    this->base = base;
    this->cmdpool = cmdpool;
    checkResultAndThrow(part::create::Fence(base.device, &loadedFence, false),
			"failed to create finish load semaphore in texLoader");
}

TexLoaderVk::~TexLoaderVk() {
    vkDestroyFence(base.device, loadedFence, nullptr);
    clearGPU();
}

Resource::Texture TexLoaderVk::addGpuTexture(uint32_t width, uint32_t height, TextureInfoVk info) {
    StagedTexVk* tex = new StagedTexVk();
    tex->width = width;
    tex->height = height;
    tex->internalTex = true;
    tex->info = info;
    tex->filesize = 0;
    return addStagedTexture(tex);
}

void TexLoaderVk::clearGPU() {
    if (textures.size() <= 0)
	return;
    InternalTexLoader::clearGPU();
    for (auto& tex : textures)
	delete tex;
    vkFreeMemory(base.device, memory, nullptr);
    textures.clear();
}

float TexLoaderVk::getMinMipmapLevel() {
    return (float)minimumMipmapLevel;
}

void TexLoaderVk::loadGPU() {
    if(staged.size() <= 0)
	return;
    clearGPU();
    InternalTexLoader::loadGPU();
    textures.resize(staged.size());
    LOG("Loading " << staged.size() << " textures to GPU");

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;
    uint32_t memoryTypeBits;
    bool stagingBufferCreated = false;
    VkDeviceSize finalMemSize = stageTexDataCreateImages(
	    stagingBuffer, stagingMemory, &memoryTypeBits, &stagingBufferCreated);

    LOG("creating final memory buffer [" << finalMemSize << " bytes]");
    
    // create final memory
    checkResultAndThrow(vkhelper::allocateMemory(base.device, base.physicalDevice,
						 finalMemSize, &memory,
						 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
						 memoryTypeBits),
			"Failed to allocate memeory for final texture storage");

    VkCommandBuffer tempCmdBuffer;
    checkResultAndThrow(part::create::CommandBuffer(base.device, cmdpool, &tempCmdBuffer),
			"failed to create temporary command buffer in end texture loading");
    VkCommandBufferBeginInfo cmdBeginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    checkResultAndThrow(vkBeginCommandBuffer(tempCmdBuffer, &cmdBeginInfo),
			"failed to begin staging texture data command buffer");

    LOG("binding images to GPU memory");
    // move texture data from staging memory to final memory
    // and ready images for mipmapping
    textureDataStagingToFinal(stagingBuffer, tempCmdBuffer);

    checkResultAndThrow(vkEndCommandBuffer(tempCmdBuffer),
			"failed to end cmdbuff for moving tex data");
    
    checkResultAndThrow(vkhelper::submitCmdBuffAndWait(
				base.device,
				base.queue.graphicsPresentQueue, &tempCmdBuffer, loadedFence,
				&graphicsPresentMutex),
			"failed to move texture data to gpu");
    
    LOG("finished moving textures to final memory location");

    checkResultAndThrow(vkResetCommandPool(base.device, cmdpool, 0),
			"Failed to reset command pool in end texture loading");

    if(stagingBufferCreated) {
	LOG("freeing staging buffer and memory");
	vkUnmapMemory(base.device, stagingMemory);
	vkDestroyBuffer(base.device, stagingBuffer, nullptr);
	vkFreeMemory(base.device, stagingMemory, nullptr);
    }
    
    checkResultAndThrow(vkBeginCommandBuffer(tempCmdBuffer, &cmdBeginInfo),
			"Failed to begin mipmap creation command buffer");

    LOG("Converting textures to final layout");    
    for (auto& tex : textures)
	tex->createMipMaps(tempCmdBuffer);

    checkResultAndThrow(vkEndCommandBuffer(tempCmdBuffer),
			"failed to end command buffer");

    checkResultAndThrow(
	    vkhelper::submitCmdBuffAndWait(base.device, base.queue.graphicsPresentQueue,
					   &tempCmdBuffer, loadedFence,
					   &graphicsPresentMutex),
			"failed to sumbit mipmap creation commands");
    
    LOG("creating image views");
    
    //create image views
    for (auto &tex: textures)
	checkResultAndThrow(
		tex->createImageView(base.device), 
		"Failed to create image view from texture");

    vkFreeCommandBuffers(base.device, cmdpool, 1, &tempCmdBuffer);
    clearStaged();
    LOG("texture loading complete");
}

uint32_t TexLoaderVk::getImageCount() { return textures.size(); }

void TexLoaderVk::checkPoolValid(Resource::Texture tex, std::string msg) {
    if(tex.pool != this->pool)
	throw std::invalid_argument(
		"tex loader - " + msg + " Vk: Texture does not belong to this resource pool");
    if(tex.ID >= textures.size())
	throw std::runtime_error(msg + " Vk: texture ID was out of range");
}

VkImageView TexLoaderVk::getImageView(Resource::Texture tex) {
    checkPoolValid(tex, "getImageView");
    return textures[tex.ID]->view;
}

VkImageLayout TexLoaderVk::getImageLayout(Resource::Texture tex) {
    checkPoolValid(tex, "getImageLayout");
    return textures[tex.ID]->info.layout;
}

bool TexLoaderVk::sampledImage(Resource::Texture tex) {
    checkPoolValid(tex, "sampledImage");
    TextureInGPU* t = textures[tex.ID];
    return t->info.usage & VK_IMAGE_USAGE_SAMPLED_BIT;
}

void TexLoaderVk::setIndex(Resource::Texture tex, uint32_t index) {
    checkPoolValid(tex, "setIndex");
    textures[tex.ID]->imageViewIndex = index;
}

unsigned int TexLoaderVk::getViewIndex(Resource::Texture tex) {
    if(tex.pool != this->pool) {
	LOG_ERROR("tex loader - getViewIndex: Texture does not belong to this resource pool");
	return Resource::NULL_ID;
    }
    if(tex.ID == Resource::NULL_ID)
	return tex.ID;
    if (tex.ID < textures.size())
	return textures[tex.ID]->imageViewIndex;
      
    LOG_ERROR("View Index's texture was out of range. given id: " <<
	      tex.ID << " , size: " << textures.size() << " . Returning 0.");
    return 0;
}

/// ---- GPU loading helpers ---

void addImagePipelineBarrier(VkCommandBuffer &cmdBuff,
			     VkImageMemoryBarrier &barrier,
			     VkPipelineStageFlags srcStageMask,
			     VkPipelineStageFlags dstStageMask) {
    vkCmdPipelineBarrier(cmdBuff,
			 srcStageMask, dstStageMask,
			 0, 0, nullptr, 0, nullptr,
			 1, &barrier);
}

bool formatSupportsMipmapping(VkPhysicalDevice physicalDevice, VkFormat format) {
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(physicalDevice, format,
					&formatProperties);
    return (formatProperties.optimalTilingFeatures &
	    VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)
	&& (formatProperties.optimalTilingFeatures &
	    VK_FORMAT_FEATURE_BLIT_DST_BIT);
}

VkResult TextureInGPU::createImage(VkDevice device, VkMemoryRequirements *pMemreq) {
    return part::create::Image(device, &this->image, pMemreq,
			       info.usage,
			       VkExtent2D { this->width, this->height },
			       info.format,
			       info.samples, info.mipLevels);
}

VkDeviceSize TexLoaderVk::stageTexDataCreateImages(VkBuffer &stagingBuffer,
						   VkDeviceMemory &stagingMemory,
						   uint32_t *pFinalMemType,
						   bool *stagingBufferCreated) {
    VkDeviceSize totalDataSize = 0;
    for(const auto tex: staged)
	totalDataSize += tex->filesize;

    void* pMem = nullptr;
    if(totalDataSize > 0) {
	LOG("creating staging buffer for textures. size: " << totalDataSize << " bytes");
	checkResultAndThrow(vkhelper::createBufferAndMemory(
				    base, totalDataSize, &stagingBuffer, &stagingMemory,
				    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
				    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
			    "Failed to create staging memory for texture data");
	*stagingBufferCreated = true;
	vkBindBufferMemory(base.device, stagingBuffer, stagingMemory, 0);
	vkMapMemory(base.device, stagingMemory, 0, totalDataSize, 0, &pMem);
    } else {
	LOG("No textures to load from RAM");
	*stagingBufferCreated = false;
    }

    VkDeviceSize finalMemSize = 0;
    VkMemoryRequirements memreq;
    VkDeviceSize bufferOffset = 0;

    *pFinalMemType = 0;
    minimumMipmapLevel = UINT32_MAX;
    for (size_t i = 0; i < staged.size(); i++) {
	// move textures in RAM to staging buffer
	if(staged[i]->filesize > 0) {
	    if(pMem == nullptr)
		throw std::runtime_error(
			"Texture Load to GPU: Need to stage texture but "
			"pointer to staging buffer was nullptr");
	    std::memcpy(static_cast<char*>(pMem) + bufferOffset,
			staged[i]->data,
			staged[i]->filesize);
	    staged[i]->deleteData();
	    bufferOffset += staged[i]->filesize;
	}

	TextureInfoVk texInfo = defaultShaderReadTextureInfo(staged[i]);		    
	if(staged[i]->internalTex)
	    texInfo = ((StagedTexVk*)staged[i])->info;
	textures[i] = new TextureInGPU(base.device, staged[i], texInfo);
	
	if (!mipmapping ||
	    !formatSupportsMipmapping(base.physicalDevice, textures[i]->info.format))
	    textures[i]->info.mipLevels = 1;
	
	checkResultAndThrow(textures[i]->createImage(base.device, &memreq),
			    "failed to create image in texture loader"
			    "for texture at index " + std::to_string(i));
	
	// update smallest mip levels of any texture
	if (textures[i]->info.mipLevels < minimumMipmapLevel)
	    minimumMipmapLevel = textures[i]->info.mipLevels;
	
	*pFinalMemType |= memreq.memoryTypeBits;
	finalMemSize = vkhelper::correctMemoryAlignment(finalMemSize, memreq.alignment);
	textures[i]->imageMemOffset = finalMemSize;
	textures[i]->imageMemSize = vkhelper::correctMemoryAlignment(
		memreq.size, memreq.alignment);
	finalMemSize += textures[i]->imageMemSize;
    }

    if(stagingBufferCreated)
	LOG("successfully copied textures to staging buffer");
    
    return finalMemSize;
}

TextureInfoVk TexLoaderVk::defaultShaderReadTextureInfo(StagedTex* t) {
    TextureInfoVk info;
    info.mipLevels =
	1 + (int)std::floor(
		std::log2(t->width > t->height ? t->width : t->height));
    if(srgb)
	info.format = VK_FORMAT_R8G8B8A8_SRGB;
    else
	info.format = VK_FORMAT_R8G8B8A8_UNORM;
    info.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    info.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
    info.access = VK_ACCESS_SHADER_READ_BIT;
    info.usage = VK_IMAGE_USAGE_SAMPLED_BIT |
	VK_IMAGE_USAGE_TRANSFER_DST_BIT |
	VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    return info;
}

VkImageMemoryBarrier initialBarrierSettings();

// transition image to mipmapping format + copy data from staging buffer to gpu memory
// and bind to texture image.
void TexLoaderVk::textureDataStagingToFinal(VkBuffer stagingBuffer,
					    VkCommandBuffer &cmdbuff) {
    VkImageMemoryBarrier barrier = initialBarrierSettings();
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL; // this layout used for mipmapping

    VkBufferImageCopy region{};
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = { 0, 0, 0 };
      
    VkDeviceSize bufferOffset = 0;
    for (int i = 0; i < textures.size(); i++) {
	vkBindImageMemory(base.device, textures[i]->image, memory, textures[i]->imageMemOffset);

	if(staged[i]->filesize > 0) {
	    barrier.image = textures[i]->image;
	    barrier.subresourceRange.levelCount = textures[i]->info.mipLevels;
	    barrier.subresourceRange.aspectMask = textures[i]->info.aspect;
	    barrier.oldLayout = textures[i]->currentImageLayout;
	    barrier.srcAccessMask = textures[i]->currentImageAccessMask;
	    barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	    addImagePipelineBarrier(cmdbuff, barrier,
				    VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				    VK_PIPELINE_STAGE_TRANSFER_BIT);
	    textures[i]->currentImageLayout = barrier.newLayout;
	    textures[i]->currentImageAccessMask = barrier.dstAccessMask;
	    region.imageExtent = { textures[i]->width, textures[i]->height, 1 };
	    region.bufferOffset = bufferOffset;
	    region.imageSubresource.aspectMask = textures[i]->info.aspect;
	    bufferOffset += staged[i]->filesize;
	    
	    vkCmdCopyBufferToImage(cmdbuff, stagingBuffer, textures[i]->image,
				   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				   1, &region);
	}
    }
}


/// ----------- Mipmapping -----------

  
void dstToSrcBarrier(VkImageMemoryBarrier *barrier);
  
void dstSrcToLayoutBarrier(VkImageMemoryBarrier *barrier, VkImageLayout layout, VkAccessFlags accessMask);
  
VkImageBlit getMipmapBlit(int32_t currentW, int32_t currentH, int destMipLevel,
			  VkImageAspectFlags aspect);

void TextureInGPU::createMipMaps(VkCommandBuffer &cmdBuff) {
    if(gpuOnly)
	return;
    VkImageMemoryBarrier barrier = initialBarrierSettings();
    barrier.image = this->image;
    barrier.subresourceRange.aspectMask = info.aspect;
    int mipW = this->width;
    int mipH = this->height;
    for(int i = 1; i < this->info.mipLevels; i++) { //start at 1 (0 would be full size)
	// change image layout to be ready for mipmapping
	barrier.subresourceRange.baseMipLevel = i - 1;
	dstToSrcBarrier(&barrier);
	addImagePipelineBarrier(cmdBuff, barrier,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkImageBlit blit = getMipmapBlit(mipW, mipH, i, info.aspect);
	vkCmdBlitImage(cmdBuff, this->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		       this->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
		       MIPMAP_FILTER);
	  
	dstSrcToLayoutBarrier(&barrier, info.layout, info.access);
	addImagePipelineBarrier(cmdBuff, barrier,
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
	if(mipW > 1) mipW /= 2;
	if(mipH > 1) mipH /= 2;
    }

    
    // Transition Image to final layout
    
    dstSrcToLayoutBarrier(&barrier, info.layout, info.access);
    barrier.subresourceRange.baseMipLevel = info.mipLevels - 1;
    barrier.oldLayout = currentImageLayout;
    barrier.srcAccessMask = currentImageAccessMask;
    addImagePipelineBarrier(cmdBuff, barrier,
			    VK_PIPELINE_STAGE_TRANSFER_BIT,
			    VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    currentImageLayout = info.layout;
    currentImageAccessMask = info.access;
}

VkImageBlit getMipmapBlit(int32_t currentW, int32_t currentH, int destMipLevel,
			  VkImageAspectFlags aspect) {
    VkImageBlit blit{};
    blit.srcOffsets[0] = {0, 0, 0};
    blit.srcOffsets[1] = { currentW, currentH, 1 };
    blit.srcSubresource.aspectMask = aspect;
    blit.srcSubresource.mipLevel = destMipLevel - 1;
    blit.srcSubresource.baseArrayLayer = 0;
    blit.srcSubresource.layerCount = 1;
    blit.dstOffsets[0] = { 0, 0, 0 };
    blit.dstOffsets[1] = {
	currentW > 1 ? currentW / 2 : 1,
	currentH > 1 ? currentH / 2 : 1,
	1 };
    blit.dstSubresource.aspectMask = aspect;
    blit.dstSubresource.mipLevel = destMipLevel;
    blit.dstSubresource.baseArrayLayer = 0;
    blit.dstSubresource.layerCount = 1;
    return blit;
}


/// --- image view creation ---

VkResult TextureInGPU::createImageView(VkDevice device) {
    return part::create::ImageView(
	    device, &this->view, this->image,
	    this->info.format, this->info.aspect, this->info.mipLevels);
}

/// --- memory barriers ---
    
VkImageMemoryBarrier initialBarrierSettings() {
    VkImageMemoryBarrier barrier { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;
    return barrier;
}

void dstToSrcBarrier(VkImageMemoryBarrier *barrier) {
      barrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
      barrier->newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
      barrier->srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
      barrier->dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
  }

void dstSrcToLayoutBarrier(VkImageMemoryBarrier *barrier, VkImageLayout layout,
			   VkAccessFlags accessMask) {
    barrier->oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier->newLayout = layout;
    barrier->srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier->dstAccessMask = accessMask;
}
