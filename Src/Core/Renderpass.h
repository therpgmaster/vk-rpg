#pragma once
#include "Core/Attachment.h"

#include <memory>

namespace EngineCore 
{
	// acquires shared ownership of the Attachment objects
	class Renderpass 
	{
	public:
		Renderpass(class EngineDevice& device, const std::vector<AttachmentUse>& attachmentUses);
		~Renderpass();

		// uses the renderer's current command buffer to begin the renderpass
		void begin(const class Renderer& renderer);
		void begin(VkCommandBuffer cmdBuffer, uint32_t framebufferIndex);
		void end();

	private:
		VkRenderPass renderpass;
		std::vector<VkFramebuffer> framebuffers; // multiple, to support one per swapchain image
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		class EngineDevice& device;

		std::vector<std::shared_ptr<Attachment>> attachments;
		std::vector<VkAttachmentDescription> attachmentDescriptions;
		
		bool areAttachmentsCompatible() const;
		void createRenderpass();
		void createAttachmentReferences(std::vector<VkAttachmentReference>& color, std::vector<VkAttachmentReference>& resolve, 
										VkAttachmentReference& depthStencil, bool& hasDepthStencil);
		void createFramebuffers();

	};

}
