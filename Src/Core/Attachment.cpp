#include "Core/Attachment.h"
#include "Core/GPU/Device.h"
#include "Core/Swapchain.h"

#include <stdexcept>

namespace EngineCore
{

	AttachmentCreateInfo::AttachmentCreateInfo(AttachmentType t, EngineSwapChain& swp, VkSampleCountFlagBits s)
	{
		AttachmentCreateInfo i = {};
		i.type = t;
		i.samples = s;
		i.imageCount = swp.imageCount();
		i.extent = swp.getSwapChainExtent();

		// defaults for color attachments
		i.format = swp.getSwapChainImageFormat();
		i.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // note: removed VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT
		i.aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT;

		if (t == AttachmentType::DEPTH || t == AttachmentType::DEPTH_STENCIL)
		{
			// defaults for depth attachments
			i.format = swp.getDepthFormat();
			i.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			i.aspectFlags = VK_IMAGE_ASPECT_DEPTH_BIT;
			if (t == AttachmentType::DEPTH_STENCIL) { i.aspectFlags |= VK_IMAGE_ASPECT_STENCIL_BIT; } // set stencil bit as well
		}
	}

	Attachment::Attachment(EngineDevice& device, const AttachmentCreateInfo& info)
		: createInfo{ info }, device{ device }
	{
		auto& a = info; // image properties

		images.resize(a.imageCount);
		imageMemorys.resize(a.imageCount);
		imageViews.resize(a.imageCount);

		for (int i = 0; i < images.size(); i++) 
		{
			// create images
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = a.extent.width;
			imageInfo.extent.height = a.extent.height;
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = 1;
			imageInfo.format = a.format;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = a.usage;
			imageInfo.samples = a.samples;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = 0;

			device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images[i], imageMemorys[i]);

			// create image views
			VkImageViewCreateInfo viewInfo{};
			viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewInfo.image = images[i];
			viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewInfo.format = a.format;
			viewInfo.subresourceRange.aspectMask = a.aspectFlags;
			viewInfo.subresourceRange.baseMipLevel = 0;
			viewInfo.subresourceRange.levelCount = 1;
			viewInfo.subresourceRange.baseArrayLayer = 0;
			viewInfo.subresourceRange.layerCount = 1;
		
			if (vkCreateImageView(device.device(), &viewInfo, nullptr, &imageViews[i]) != VK_SUCCESS)
			{ throw std::runtime_error("failed to create attachment image view"); }
		}
	}

	Attachment::~Attachment()
	{
		for (int i = 0; i < images.size(); i++)
		{
			vkDestroyImageView(device.device(), imageViews[i], nullptr);
			vkDestroyImage(device.device(), images[i], nullptr);
			vkFreeMemory(device.device(), imageMemorys[i], nullptr);
		}
	}

	bool Attachment::isCompatible(const Attachment& b) const 
	{
		return (
				createInfo.samples == b.createInfo.samples &&
				createInfo.extent.width == b.createInfo.extent.width &&
				createInfo.extent.height == b.createInfo.extent.height
			);
	}

	AttachmentUse::AttachmentUse(Attachment& attachment, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
		VkImageLayout initialLayout, VkImageLayout finalLayout)
		: attachment{ attachment }, description{}
	{
		description.loadOp = loadOp;
		description.storeOp = storeOp;
		description.format = attachment.info().format;
		description.samples = attachment.info().samples;
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

