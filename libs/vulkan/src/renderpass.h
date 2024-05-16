#ifndef VK_ENV_RENDER_PASS
#define VK_ENV_RENDER_PASS

#include <volk.h>
#include <vector>
#include <graphics/resources.h>
#include "resources/texture_loader.h"

enum class AttachmentType {
  Colour,
  Depth,
  Resolve,
};

enum class AttachmentUse {
  Attachment,
  TransientAttachment, 
  ShaderRead,
  PresentSrc,
};

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
	    VkDeviceSize* pMemSize,
	    uint32_t* pMemFlags,
	    bool createImage);
    
    VkResult CreateFramebuffer(
	    VkRenderPass renderpass,
	    VkDeviceMemory imageMemory,
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

    /// pMemReq will be added to by the amount of memory required to
    /// store all the attachment images.
    /// pMemFlags will have the memory flags required for the images.
    /// It's up to the caller to create the memory, which is then passed
    /// to createFramebuffers.
    ///
    /// The memory offset of the images is based
    /// on the size of pMemReq before it was sent to this function.
    ///
    /// This will also destroy the previous framebuffers, but the
    /// caller is responsible for freeing the memory they allocated for the
    /// previous attachment images
    VkResult createFramebufferImages(TexLoaderVk* tex,
				     std::vector<VkImage> *swapchainImages,
				     VkExtent2D extent,
				     VkDeviceSize *pMemReq,
				     uint32_t *pMemFlags);
    VkResult createFramebuffers(VkDeviceMemory framebufferImageMemory);

    /// It's up to the caller to end the render pass.
    /// This also sets the viewport and scissor to
    /// offsets of 0, 0 and extent equal to the framebuffer extent.
    void beginRenderPass(VkCommandBuffer cmdBuff, uint32_t frameIndex);
    
    std::vector<VkImageView> getAttachmentViews(uint32_t attachmentIndex);
    //std::vector<Resource::Texture> getAttachmentTextures(uint32_t attachmentIndex);
    
    VkExtent2D getExtent() { return this->framebufferExtent; }
    VkRenderPass getRenderPass() { return this->renderpass; }

 private:
    VkDevice device;
    VkRenderPass renderpass;
    std::vector<AttachmentDesc> attachmentDescription;
    std::vector<VkClearValue> attachmentClears;

    VkExtent2D framebufferExtent;
    std::vector<Framebuffer> framebuffers;

    VkOffset2D offset = {0, 0};
};

#endif
