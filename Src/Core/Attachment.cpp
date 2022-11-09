#include "Core/Attachment.h"
#include "Core/GPU/Device.h"

#include <stdexcept>

namespace EngineCore
{
	VkAttachmentDescription Attachment::getDescription() const
	{
		VkAttachmentDescription d{};
		d.format = info.format;
		d.samples = info.samples;
		/* the following properties must be specified manually whenever this function is used:
		loadOp, storeOp, stencilLoadOp, stencilStoreOp, initialLayout, finalLayout */
		return d;
	}
	
	void Attachment::createResources(EngineDevice& device)
	{
		auto& a = info; // attachment properties

		images.resize(a.imageCount);
		imageMemorys.resize(a.imageCount);
		imageViews.resize(a.imageCount);

		for (int i = 0; i < images.size(); i++) 
		{
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


	Framebuffer::Framebuffer(EngineDevice& device, std::vector<VkImageView> imageViews,
							VkRenderPass renderPass, VkExtent2D extent, uint32_t numCopies)
	{
		framebuffers.resize(numCopies);
		for (size_t i = 0; i < numCopies; i++)
		{
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(imageViews.size());
			framebufferInfo.pAttachments = imageViews.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;
			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			{ throw std::runtime_error("failed to create framebuffer"); }
		}
	}

}

