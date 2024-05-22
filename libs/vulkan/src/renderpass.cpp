#include "renderpass.h"

#include <stdexcept>
#include "logger.h"
#include "parts/images.h"


/// ----- Internal Attachment Image -----

class AttachmentImage {
public:
    AttachmentImage(AttachmentDesc &desc);
    void Destroy(VkDevice device);
    VkResult LoadImage(VkDevice device,
			 VkExtent2D extent,
			 TexLoaderVk* texloader);
    void AddImage(VkImage);
    VkResult GetImageView(TexLoaderVk* texloader,
			     VkDevice device);
    void AddImageView(VkImageView);
    VkImageView getView();
    Resource::Texture getTexture() { return texture; }
    bool isUsingExternalImage();
    bool hasImageForEachFrame();
    
private:
    enum class state {
	unmade,
	image,
	imageview,
    };

    state state = state::unmade;

    bool usingExternalImage = false;
    bool imageForEachFrame = false;
        
    TextureInfoVk texinfo;
    Resource::Texture texture;
    VkImage externalImage;
    VkImageView view;
};


/// ---- Renderpass ----


enum class SubpassDependancy {
  PreviousImageOps,
  FutureShaderRead,
};

VkSubpassDependency genSubpassDependancy(bool colour, bool depth,
                                         SubpassDependancy depType);

VkClearValue getClearValueAndSetRef(AttachmentDesc &attachment, float clearColour[3],
                           std::vector<VkAttachmentReference> &colourRefs,
                           VkAttachmentReference &depthRef, bool &hasDepth,
                           VkAttachmentReference &resolveRef, bool &hasResolve);

RenderPass::RenderPass(VkDevice device,
		       std::vector<AttachmentDesc> attachments,
		       float clearColour[3]) {
    this->attachmentDescription.resize(attachments.size());
    this->device = device;

    std::vector<VkAttachmentDescription> attachDescVK(attachments.size());
    bool hasDepth, hasResolve, hasShaderReadAttachment;
    hasDepth = hasResolve = hasShaderReadAttachment = false;
    VkAttachmentReference depthRef, resolveRef;
    std::vector<VkAttachmentReference> colourRefs;
    
    attachmentClears.resize(attachments.size());
    for(int i = 0; i < attachments.size(); i++) {
	int attachIndex = attachments[i].getIndex();
	if(attachIndex > attachments.size())
	    throw std::runtime_error("Render Pass Creation Error: Attachment Index "
				     "was greater than the number of supplied attachments");
	if(attachmentDescription[attachIndex].wasCreated())
	    throw std::runtime_error("Render Pass Creation Error: tried to have two attachments "
				     "with the same index!");
	attachmentDescription[attachIndex] = attachments[i];    
	attachDescVK[attachIndex] = attachments[i].getAttachmentDescription();

	if(attachments[i].getUse() == AttachmentUse::ShaderRead)
	    hasShaderReadAttachment = true;

	attachmentClears[attachIndex] = getClearValueAndSetRef(
		attachments[i], clearColour, colourRefs,
		depthRef, hasDepth, resolveRef, hasResolve);
    }

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = (uint32_t)colourRefs.size();
    subpass.pColorAttachments = colourRefs.data();
    if(hasDepth)
	subpass.pDepthStencilAttachment = &depthRef;
    if(hasResolve)
	subpass.pResolveAttachments = &resolveRef;

    std::vector<VkSubpassDependency> subpassDependancies;
    subpassDependancies.push_back(
	    genSubpassDependancy(colourRefs.size() > 0, hasDepth,
				 SubpassDependancy::PreviousImageOps));
    if(hasShaderReadAttachment)
	subpassDependancies.push_back(
		genSubpassDependancy(colourRefs.size() > 0, hasDepth,
				     SubpassDependancy::FutureShaderRead));

    VkRenderPassCreateInfo createInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    createInfo.attachmentCount = (uint32_t)attachDescVK.size();
    createInfo.pAttachments = attachDescVK.data();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpass;
    createInfo.dependencyCount = subpassDependancies.size();
    createInfo.pDependencies = subpassDependancies.data();
    VkResult res = vkCreateRenderPass(device, &createInfo, VK_NULL_HANDLE, &this->renderpass);
    checkResultAndThrow(res, "Failed to created render pass!");
}

RenderPass::~RenderPass() {
    framebuffers.clear();
    vkDestroyRenderPass(device, this->renderpass, VK_NULL_HANDLE);
}

VkResult RenderPass::loadFramebufferImages(TexLoaderVk* tex,
					     std::vector<VkImage> *swapchainImages,
					     VkExtent2D extent) {
    VkResult result = VK_SUCCESS;
    std::vector<AttachmentImage> attachImages;
    for(int i = 0; i < attachmentDescription.size(); i++)
	attachImages.push_back(AttachmentImage(attachmentDescription[i]));
    framebuffers.clear();
    if(swapchainImages != nullptr)
	framebuffers.resize((swapchainImages->size()));
    else
	framebuffers.resize(1);
    framebufferExtent = extent;
    for(int i = 0; i < framebuffers.size(); i++) {
	framebuffers[i].CreateImages(
		device, tex, attachImages,
		swapchainImages == nullptr ? nullptr : &(*swapchainImages)[i],
		extent, i == 0);
    }
    return result;
}

VkResult RenderPass::createFramebuffers(TexLoaderVk* tex) {
    VkResult result = VK_SUCCESS;
    for(int i = 0; i < framebuffers.size(); i++) {
	msgAndReturnOnErr(
		framebuffers[i].CreateFramebuffer(
			tex,
			renderpass,
			i == 0 ? nullptr : &framebuffers[0]),
		"RenderPass - Failed to create framebuffer");
    }
    return result;
}

VkViewport fbViewport(VkExtent2D extent);
VkRect2D fbScissor(VkExtent2D extent);

void RenderPass::beginRenderPass(VkCommandBuffer cmdBuff, uint32_t frameIndex) {
    VkRenderPassBeginInfo beginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    if(frameIndex >= framebuffers.size())
	throw std::runtime_error("RenderPass::beginRenderPass - Tried to begin pass, "
				 "but the frame index was out of range. "
				 "Ensure that the renderpass framebuffers "
				 "have been created and the index is correct.");
    beginInfo.renderPass = renderpass;
    beginInfo.framebuffer = framebuffers[frameIndex].framebuffer;
    beginInfo.renderArea.offset = offset;
    beginInfo.renderArea.extent = framebufferExtent;
    beginInfo.clearValueCount = attachmentClears.size();
    beginInfo.pClearValues = attachmentClears.data();
    vkCmdBeginRenderPass(cmdBuff, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
    VkViewport viewport = fbViewport(framebufferExtent);
    vkCmdSetViewport(cmdBuff, 0, 1, &viewport);
    VkRect2D scissor = fbScissor(framebufferExtent);
    vkCmdSetScissor(cmdBuff, 0, 1, &scissor);
}

std::vector<Resource::Texture> RenderPass::getAttachmentTextures(uint32_t attachmentIndex) {
    if(framebuffers.empty())
	throw std::runtime_error("RenderPass Error: Tried to get attachment views but framebuffers"
				 " haven't been created.");
    if(attachmentDescription.size() <= attachmentIndex)
	throw std::runtime_error("RenderPass Error: Tried to get attchment views, but the"
				 " supplied attachment Index was out of range");
    if(attachmentDescription[attachmentIndex].getUse() != AttachmentUse::ShaderRead)
	throw std::runtime_error("RenderPass Error: Tried to get attachment views, but the"
				 " attachment is not for reading from a shader");
    std::vector<Resource::Texture> views(framebuffers.size());
    for(int i = 0; i < framebuffers.size(); i++) {
	if(framebuffers[0].attachments[attachmentIndex].hasImageForEachFrame())
	    views[i] = framebuffers[i].attachments[attachmentIndex].getTexture();
	else
	    views[i] = framebuffers[0].attachments[attachmentIndex].getTexture();
    }
    return views;
}


/// --- Attachment Image Implementation ---


AttachmentImage::AttachmentImage(AttachmentDesc &attachmentDesc) {
    this->texinfo = attachmentDesc.getImageInfo();
    switch(attachmentDesc.getUse()) {
    case AttachmentUse::Screen:
	usingExternalImage = true;
	imageForEachFrame = true;
	break;
    default:
	break;
    }
}

void AttachmentImage::Destroy(VkDevice device)
{
    switch(state) {
    case state::imageview:
	if(usingExternalImage)
	    vkDestroyImageView(device, view, nullptr);
    case state::image:
    case state::unmade:
	break;
    }
}

VkResult AttachmentImage::LoadImage(VkDevice device,
				      VkExtent2D extent,
				      TexLoaderVk* texloader) {
    if(state != state::unmade)
	throw std::runtime_error("Error: invalid attachment image state, "
				 "tried to create image but previous "
				 "state wasn't state::unmade");
    if(usingExternalImage)
	throw std::runtime_error("Attachment Image Error: invalid attachment image op, "
				 "tried to create image with attachment that "
				 "uses an external image.");
    this->texture = texloader->addGpuTexture(extent.width, extent.height, texinfo);
    state = state::image;
    return VK_SUCCESS;
}

void AttachmentImage::AddImage(VkImage image) {
    if(state != state::unmade)
	throw std::runtime_error("Attachment Image Error: invalid attachment image state, "
				 "tried to add image but previous "
				 "state wasn't state::unmade");
    if(!usingExternalImage)
	throw std::runtime_error("Attachment Image Error: invalid attachment image op, "
				 "tried to add image to attachment that "
				 "does not use an external image (ie swapchain image)");
    this->externalImage = image;
    state = state::image;
}

VkResult AttachmentImage::GetImageView(
	TexLoaderVk* texloader,
	VkDevice device) {
    VkResult result = VK_SUCCESS;
    if(state != state::image)
	throw std::runtime_error("Attachment Image Error: "
				 "invalid attachment image state, tried to "
				 "create image view, but previous state "
				 "wasn't state::image");
    if(!usingExternalImage) {
	view = texloader->getImageView(texture);
    } else {    
	result = part::create::ImageView(
		device, &view, externalImage, texinfo.format, texinfo.aspect, 1);
    }

    if(result == VK_SUCCESS)
	state = state::imageview;
    return result;
}

void AttachmentImage::AddImageView(VkImageView view) {
    this->view = view;
}

VkImageView AttachmentImage::getView() { return this->view; }

bool AttachmentImage::isUsingExternalImage() { return this->usingExternalImage; }
bool AttachmentImage::hasImageForEachFrame() { return this->imageForEachFrame; }



/// ---- Framebuffer ----


Framebuffer::~Framebuffer() {
    if(framebufferCreated)
	vkDestroyFramebuffer(device, framebuffer, VK_NULL_HANDLE);
    for(auto &a: attachments)
	a.Destroy(device);
}

VkResult Framebuffer::CreateImages(
	    VkDevice device,
	    TexLoaderVk* tex,
	    std::vector<AttachmentImage> attachmentImages,
	    VkImage *swapchainImage,
	    VkExtent2D extent,
	    bool createImage) {
    VkResult result = VK_SUCCESS;
    
    this->attachments = attachmentImages;
    this->device = device;
    this->extent = extent;
    
    for(int i = 0; i < attachments.size(); i++) {
	if(attachments[i].isUsingExternalImage()) {
	    if(swapchainImage == nullptr)
		throw std::runtime_error(
			"create fb images: swapchain images null, "
			" but attachment is using external image!");			    
	    attachments[i]
		.AddImage(*swapchainImage);
	}
	else if(createImage || attachments[i].hasImageForEachFrame())
	    msgAndReturnOnErr(
		    attachments[i]
		    .LoadImage(device, extent, tex),
		    "RenderPass Error: Failed to create framebuffer attachment image");
    }
    return result;
}

VkResult Framebuffer::CreateFramebuffer(TexLoaderVk* tex,
					VkRenderPass renderpass,
					Framebuffer* firstFramebuffer) {
    VkResult result = VK_SUCCESS;

    VkFramebufferCreateInfo fbCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    fbCreateInfo.renderPass = renderpass;
    fbCreateInfo.flags = 0;
    fbCreateInfo.width = extent.width;
    fbCreateInfo.height = extent.height;
    fbCreateInfo.layers = 1;
    
    std::vector<VkImageView> attachmentViews(attachments.size());

    for(int i = 0; i < attachments.size(); i++) {
	if(firstFramebuffer == nullptr || attachments[i].hasImageForEachFrame()) {
	    msgAndReturnOnErr(
		    attachments[i].GetImageView(tex, device),
		    "RenderPass Error: Failed to create image view for framebuffer");
	} else {
	    attachments[i].AddImageView(firstFramebuffer->attachments[i].getView());
	}
	attachmentViews[i] = attachments[i].getView();
    }

    fbCreateInfo.attachmentCount = attachmentViews.size();
    fbCreateInfo.pAttachments = attachmentViews.data();
    msgAndReturnOnErr(vkCreateFramebuffer(device, &fbCreateInfo, VK_NULL_HANDLE, &framebuffer),
		      "RenderPass Error: Failed to create Framebuffer");
    framebufferCreated = true;
    return result;
}


/// --- Attachment Description ---


AttachmentDesc::AttachmentDesc(uint32_t index, AttachmentType type, AttachmentUse use,
			       VkSampleCountFlagBits sampleCount, VkFormat format) {
    this->index = index;
    this->type = type;
    this->use = use;

    this->tex.format = format;
    this->tex.samples = sampleCount;
    this->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    switch(type) {
    case AttachmentType::Resolve:
	this->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    case AttachmentType::Colour:
	attachmentImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	this->tex.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	this->tex.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	break;
    case AttachmentType::Depth:
	//must be depth_stencil, unless seperate depth stencil layout feature is enabled.
	attachmentImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	this->tex.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	this->tex.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	break;
    }
    switch(use) {
	//case AttachmentUse::TransientAttachment:
	//this->tex.usage |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
    case AttachmentUse::Attachment:
        tex.layout = attachmentImageLayout;
	storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	break;
    case AttachmentUse::ShaderRead:
	storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	tex.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	this->tex.usage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	break;
    case AttachmentUse::Screen:
	storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	tex.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	break;
    }
}

VkAttachmentReference AttachmentDesc::getAttachmentReference() {
    VkAttachmentReference attachRef;
    attachRef.attachment = index;
    attachRef.layout = attachmentImageLayout;
    return attachRef;
}

VkAttachmentDescription AttachmentDesc::getAttachmentDescription() {
    VkAttachmentDescription desc;
    desc.flags = 0;
    desc.format = tex.format;
    desc.samples = tex.samples;
    desc.loadOp = loadOp;
    desc.storeOp = storeOp;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    desc.finalLayout = tex.layout;
    return desc;
}


/// ---- Helpers ----


VkClearValue getClearValueAndSetRef(AttachmentDesc &attachment,
				    float clearColour[3],
				    std::vector<VkAttachmentReference> &colourRefs,
				    VkAttachmentReference &depthRef,
				    bool &hasDepth,
				    VkAttachmentReference &resolveRef,
				    bool &hasResolve) {
    VkClearValue clear;
    VkAttachmentReference attachRef = attachment.getAttachmentReference();
    switch(attachment.getType()) {
    case AttachmentType::Colour:
	colourRefs.push_back(attachRef);
	clear.color = {{clearColour[0], clearColour[1], clearColour[2], 1.0f}};
	break;
    case AttachmentType::Depth:
	if(hasDepth)
	    std::runtime_error("Render Pass State Error: Can only have 1 depth attachment");
	hasDepth = true;
	depthRef = attachRef;
	clear.depthStencil = {1.0f, 0};
	break;
    case AttachmentType::Resolve:
	if(hasResolve)
	    std::runtime_error("Render Pass State Error: Can only have 1 resolve attachment");
	hasResolve = true;
	resolveRef = attachRef;
    default:
	std::runtime_error("Render Pass Error: Unrecognised attachment type");
	clear.color = {{0.0f, 0.0f, 0.0f}};
    }
    return clear;
}


VkViewport fbViewport(VkExtent2D extent) {
    VkViewport viewport;
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)extent.width;
    viewport.height = (float)extent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    return viewport;
}

VkRect2D fbScissor(VkExtent2D extent) {
    VkRect2D scissor;
    scissor.offset = {0, 0};
    scissor.extent = extent;
    return scissor;
}

void setStageMask(VkPipelineStageFlags *pStageMask, bool colour, bool depth) {
    *pStageMask = 0;
    if(colour)
	*pStageMask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    if(depth)
	*pStageMask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
}

void setAccessMask(VkAccessFlags *pAccessMask, bool colour, bool depth) {
    *pAccessMask = 0;
    if(colour)
	*pAccessMask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    if(depth)
	*pAccessMask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
}

VkSubpassDependency genSubpassDependancy(bool colour, bool depth, SubpassDependancy depType) {
    VkSubpassDependency dep;
    dep.srcStageMask = 0;
    dep.srcAccessMask = 0;
    dep.dstStageMask = 0;
    dep.dstAccessMask = 0;
    dep.dependencyFlags = 0;
    switch(depType) {
    case SubpassDependancy::PreviousImageOps:
	dep.srcSubpass = VK_SUBPASS_EXTERNAL;
	dep.dstSubpass = 0;

	dep.srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT |
	    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dep.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	
	dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
	    VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
	    VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
	    VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	break;
    case SubpassDependancy::FutureShaderRead:
	dep.srcSubpass = 0;
	dep.dstSubpass = VK_SUBPASS_EXTERNAL;
	setStageMask(&dep.srcStageMask, colour, depth);
	setAccessMask(&dep.srcAccessMask, colour, depth);
	dep.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	dep.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	break;
    }
    return dep;
}
