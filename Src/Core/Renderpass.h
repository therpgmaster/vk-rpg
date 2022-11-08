#pragma once
#include "Core/Attachment.h"

#include <vector>

namespace EngineCore 
{
	struct RpAttachment 
	{ 
		VkAttachmentDescription d{};
		uint32_t i;
		// syntactic sugar
		enum class Type { color, resolve, depthStencil } t;
		RpAttachment(VkAttachmentDescription description, uint32_t index, Type type);
	};

	struct RpAttachmentsInfo
	{
		std::vector<RpAttachment> colorAttachments;
		std::vector<RpAttachment> resolveAttachments;
		RpAttachment depthStencilAttachment;
		bool hasDepthStencil = false;
		VkImageLayout colorLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		VkImageLayout depthStencilLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		void add(const RpAttachment& a);
	};

	class Renderpass 
	{
	public:
		Renderpass(class EngineDevice& device, RpAttachmentsInfo a);

	private:
		VkRenderPass renderpass;

	};

}  // namespace
