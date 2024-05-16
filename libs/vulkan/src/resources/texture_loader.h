#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <resource-loaders/texture_loader.h>
#include "../device_state.h"

struct TextureInGPU;

struct TextureInfoVk {
    VkFormat format;
    VkImageLayout layout;
    VkSampleCountFlagBits samples;
    VkImageAspectFlags aspect;	
    VkAccessFlagBits access;
    VkImageUsageFlags usage;
    uint32_t mipLevels = 1;
};

class TexLoaderVk : public InternalTexLoader {
public:
    TexLoaderVk(DeviceState base, VkCommandPool pool,
		Resource::Pool resPool, RenderConfig config);
    ~TexLoaderVk() override;
    void clearGPU() override;
    void loadGPU() override;

    Resource::Texture addGpuTexture(uint32_t width, uint32_t height, TextureInfoVk info);
    
    float getMinMipmapLevel();
    uint32_t getImageCount();
    VkImageView getImageView(Resource::Texture tex);
    VkImageLayout getImageLayout(Resource::Texture tex);
    bool sampledImage(Resource::Texture tex);
    void setIndex(Resource::Texture texture, uint32_t index);
    unsigned int getViewIndex(Resource::Texture tex) override;
    
private:
    VkDeviceSize stageTexDataCreateImages(VkBuffer &stagingBuffer,
					  VkDeviceMemory &stagingMemory,
					  uint32_t *pFinalMemType,
					  bool* stagingBufferCreated);
    void textureDataStagingToFinal(VkBuffer stagingBuffer,
				   VkCommandBuffer &cmdbuff);

    TextureInfoVk defaultShaderReadTextureInfo(StagedTex* t);

    void checkPoolValid(Resource::Texture tex, std::string msg);
            
    DeviceState base;
    VkCommandPool cmdpool;
    std::vector<TextureInGPU*> textures;
    VkDeviceMemory memory;
    uint32_t minimumMipmapLevel;
    VkFence loadedFence;
};

#endif
