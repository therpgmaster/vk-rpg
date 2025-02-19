#pragma once
#include "Core/Types/vk.h"
#include "Core/Render/Attachment.h"

#include <vector>
#include <memory>

namespace EngineCore 
{
	class Renderer;
	class EngineDevice;

	class LightAttachment;

	// a collection of related framebuffers (e.g. one for each frame in flight / swapchain image)
	class FramebufferSet
	{
	public:
		FramebufferSet(class EngineDevice& device) : device(device) {};
		~FramebufferSet();
		void createFramebuffers(const LightAttachment& lightAttachment);
		VkFramebuffer getFramebuffer(size_t i) const { return framebuffers[i]; }
	private:
		std::vector<VkFramebuffer> framebuffers;
		class EngineDevice& device;
	};

	// acquires shared ownership of the Attachment objects
	class Renderpass 
	{
	public:
		Renderpass(EngineDevice& device, const std::vector<AttachmentUse>& attachmentUses, 
					VkExtent2D framebufferExtent, uint32_t framebufferCount);
		~Renderpass();
		Renderpass(Renderpass&&) = default;

		// uses the supplied command buffer to begin the renderpass
		void begin(VkCommandBuffer cmdBuffer, uint32_t framebufferSetIndex, uint32_t framebufferIndex);

		VkRenderPass getRenderpass() const { return renderpass; }

	private:
		VkRenderPass renderpass;
		std::vector<FramebufferSet> framebufferSets; // one framebuffer for each swapchain image (or frame), in each set
		VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
		class EngineDevice& device;

		// image views, description, and type for each attachment
		std::vector<AttachmentUse> attachments;
		std::vector<VkAttachmentDescription2> attachmentDescriptions;

		VkExtent2D framebufferExtent;
		uint32_t framebufferCount;
		
		bool areAttachmentsCompatible() const;
		std::vector<VkAttachmentDescription2> getAttachmentDescriptions() const;
		void createRenderpass();
		void createAttachmentReferences(std::vector<VkAttachmentReference2>& color, std::vector<VkAttachmentReference2>& resolve, 
										VkAttachmentReference2& depthStencil, VkAttachmentReference2& depthStencilResolve, 
										bool& hasDepth, bool& hasDepthResolve);
										
		void createFramebuffers();

	};

}
