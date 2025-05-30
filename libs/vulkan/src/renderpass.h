#ifndef VK_ENV_RENDER_PASS
#define VK_ENV_RENDER_PASS

#include <volk.h>
#include <vector>
#include <graphics/resources.h>
#include <graphics/render_pass.h>
#include "resources/texture_loader.h"

/// A high level description of the framebuffer attachments
class AttachmentDesc {
 public:
    AttachmentDesc() { created = false; }
    AttachmentDesc(uint32_t index, AttachmentType type, AttachmentUse use,
		   VkSampleCountFlagBits sampleCount, VkFormat format);
    VkAttachmentReference getAttachmentReference();
    VkAttachmentDescription getAttachmentDescription();
    AttachmentType getType() { return type; }
    AttachmentUse getUse() { return use; }
    uint32_t getIndex() { return index; }
    bool wasCreated() { return created; }
    TextureInfoVk getImageInfo() { return tex; }
private:
    bool created = true;
    uint32_t index;
    AttachmentType type;
    AttachmentUse use;
    TextureInfoVk tex;
    VkImageLayout attachmentImageLayout;
    VkAttachmentLoadOp loadOp;
    VkAttachmentStoreOp storeOp;
};

/// Holds the attachment image resources
class AttachmentImage;

/// Holds the framebuffer resources;
struct Framebuffer {
    ~Framebuffer();
    VkResult CreateImages(
	    VkDevice device,
	    TexLoaderVk* tex,
	    std::vector<AttachmentImage> attachImages,
	    VkImage *swapchainImage, // can be null
	    VkExtent2D extent,
	    bool createImage);
    
    VkResult CreateFramebuffer(
	    TexLoaderVk* tex,
	    VkRenderPass renderpass,
	    Framebuffer* firstFramebuffer);
    
    VkDevice device;
    VkExtent2D extent;
    bool framebufferCreated = false;
    VkFramebuffer framebuffer;
    std::vector<AttachmentImage> attachments;
};

class RenderPass {
 public:
    RenderPass(VkDevice device, std::vector<AttachmentDesc> attachments,
	       float clearColour[3]);
    ~RenderPass();

    VkResult loadFramebufferImages(TexLoaderVk* tex,
				   std::vector<VkImage> *swapchainImages,
				   VkExtent2D extent);
    VkResult createFramebuffers(TexLoaderVk* tex);

    /// It's up to the caller to end the render pass.
    /// This also sets the viewport and scissor to
    /// offsets of 0, 0 and extent equal to the framebuffer extent.
    void beginRenderPass(VkCommandBuffer cmdBuff, uint32_t frameIndex);
    
    std::vector<Resource::Texture> getAttachmentTextures(uint32_t attachmentIndex);
    
    VkExtent2D getExtent() { return this->framebufferExtent; }
    VkRenderPass getRenderPass() { return this->renderpass; }
    VkSampleCountFlagBits msaaSamples() { return this->samples; }

 private:
    VkDevice device;
    VkRenderPass renderpass;
    std::vector<AttachmentDesc> attachmentDescription;
    std::vector<VkClearValue> attachmentClears;

    VkExtent2D framebufferExtent;
    std::vector<Framebuffer> framebuffers;

    VkOffset2D offset = {0, 0};
    VkSampleCountFlagBits samples;
};

#endif
