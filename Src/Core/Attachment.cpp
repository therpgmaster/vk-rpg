#include "Core/Attachment.h"
#include "Core/GPU/Device.h"

#include <stdexcept>

namespace EngineCore
{
	//AttachmentCreateInfo::AttachmentCreateInfo(AttachmentType type, EngineSwapChain& swapchain, VkSampleCountFlagBits samples, bool usedAsInput)
	//	: type{ type }, samples{ samples }, extent{ swapchain.getSwapChainExtent() }, imageCount{ swapchain.imageCount() }, isInputAttachment{ usedAsInput } {}
	//bool AttachmentCreateInfo::isColor() const { return type == AttachmentType::COLOR || type == AttachmentType::RESOLVE || type == AttachmentType::SWAPCHAIN; }

	VkImageAspectFlags AttachmentProperties::getAspectFlags() const
	{
		switch (type)
		{
			case AttachmentType::DEPTH_STENCIL: return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
			case AttachmentType::DEPTH:			return VK_IMAGE_ASPECT_DEPTH_BIT;
			default:							return VK_IMAGE_ASPECT_COLOR_BIT;
		}
	}

	Attachment::Attachment(EngineDevice& device, const AttachmentProperties& props, bool input, bool sampled)
		: device{ device }, props{ props }
	{	
		const auto& p = getProps();
		
		VkImageCreateInfo imageInfo = Image::makeImageCreateInfo(p.extent.width, p.extent.height); // defaults
		imageInfo.format = p.format;
		imageInfo.samples = p.samples;
		imageInfo.usage = Attachment::isColor(p.type) ? VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT : VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.usage |= input ? VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT : 0;
		imageInfo.usage |= sampled ? VK_IMAGE_USAGE_SAMPLED_BIT : 0;
		// support lazily allocated memory (device-only, incompatible with sampled usage)
		imageInfo.usage |= !sampled ? VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT : 0; 

		images.reserve(p.imageCount);
		for (uint32_t i = 0; i < p.imageCount; i++)
		{
			images.push_back(std::make_unique<Image>(device, imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));
			images.back()->updateView(p.format, p.getAspectFlags()); // create image view
		}
		return;
	}

	Attachment::Attachment(EngineDevice& device, const AttachmentProperties& props, const std::vector<VkImage>& swapchainImages)
		: device{ device }, props{ props }
	{
		const auto& p = getProps();
		
		images.reserve(swapchainImages.size());
		for (const auto& image : swapchainImages)
		{
			images.push_back(std::make_unique<Image>(device, image)); // copy image handle
			images.back()->updateView(p.format, VK_IMAGE_ASPECT_COLOR_BIT); // create image view
		}
	}

	//VkFormat Attachment::getFormat(const EngineSwapChain& swapchain) const
	//{
	//	return info().isColor() ? swapchain.getSwapChainImageFormat() : swapchain.getDepthFormat(); // defaults
	//}

	std::vector<VkImageView> Attachment::getImageViews() const 
	{
		std::vector<VkImageView> views;
		views.reserve(images.size());
		for (const auto& image : images) { views.push_back(image->getView()); }
		return views;
	}

	bool Attachment::isCompatible(const Attachment& b) const 
	{
		return (getProps().samples ==		b.getProps().samples &&
				getProps().extent.width ==	b.getProps().extent.width &&
				getProps().extent.height == b.getProps().extent.height);
	}

	AttachmentUse::AttachmentUse(const Attachment& attachment, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
								VkImageLayout initialLayout, VkImageLayout finalLayout)
	{
		imageViews = attachment.getImageViews();
		type = attachment.getProps().type;

		description.format = attachment.getProps().format;
		description.samples = attachment.getProps().samples;

		description.loadOp = loadOp;
		description.storeOp = storeOp;
		// use setStencilOps function if stencil is used, disabled here by default
		description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		description.initialLayout = initialLayout;
		description.finalLayout = finalLayout;
	};

	void AttachmentUse::setStencilOps(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp)
	{
		description.stencilLoadOp = stencilLoadOp;
		description.stencilStoreOp = stencilStoreOp;
	}

}

