#pragma once
#include "Core/Attachment.h"

#include <vector>
#include <memory>

namespace EngineCore 
{
	// just syntactic sugar, e.g. AttachmentLoadOp::LOAD
	struct AttachmentLoadOp
	{
		const static VkAttachmentLoadOp DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		const static VkAttachmentLoadOp LOAD = VK_ATTACHMENT_LOAD_OP_LOAD;
		const static VkAttachmentLoadOp CLEAR = VK_ATTACHMENT_LOAD_OP_CLEAR;
	};
	struct AttachmentStoreOp
	{
		const static VkAttachmentStoreOp DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		const static VkAttachmentStoreOp STORE = VK_ATTACHMENT_STORE_OP_STORE;
	};
	// info for renderpass creation
	struct AttachmentDescription
	{
		VkAttachmentDescription d;
		std::shared_ptr<Attachment> a;

		AttachmentDescription(std::shared_ptr<Attachment> a, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp, 
							VkImageLayout initialLayout, VkImageLayout finalLayout);

		void setStencilOps(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp);
	};

	class Renderpass 
	{
	public:
		Renderpass(class EngineDevice& device, std::vector<AttachmentDescription> attachments);
		
		void begin(VkCommandBuffer commandBuffer, uint32_t currentFrame);

	private:
		std::vector<AttachmentDescription> attachments;
		VkRenderPass renderpass;
		std::vector<VkFramebuffer> framebuffers;

		class EngineDevice& device;
		void createRenderpass();
		void createFramebuffers();

		

	};

}  // namespace
