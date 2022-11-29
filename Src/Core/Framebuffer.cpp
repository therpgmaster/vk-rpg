#include "Core/Framebuffer.h"
#include "Core/Renderpass.h"
#include "Core/GPU/Device.h"

#include <stdexcept>

namespace EngineCore 
{
	Framebuffer::Framebuffer(EngineDevice& device, Renderpass renderpass,
							const std::vector<std::vector<VkImageView>>& imageViews, VkExtent2D extent, uint32_t numCopies)
	{
		framebuffers.resize(numCopies);
		for (uint32_t i = 0; i < numCopies; i++)
		{
			std::vector<VkImageView> attachments;		attachments.reserve(imageViews[i].size());
			for (auto& view : imageViews[i]) { attachments.push_back(view); }

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass.get();
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, framebuffers[i].get()) != VK_SUCCESS)
			{ throw std::runtime_error("failed to create framebuffer"); }
		}
	}
}