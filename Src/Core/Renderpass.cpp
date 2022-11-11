#include "Core/Renderpass.h"
#include "Core/GPU/Device.h"

#include <stdexcept>

namespace EngineCore 
{
	RpAttachment::RpAttachment(VkAttachmentDescription description, uint32_t index, Type type)
		: d{ description }, i{ index }, t{ type }{};
	
	void RpAttachmentsInfo::add(const RpAttachment& a)
	{
		switch (a.t)
		{
		case RpAttachment::Type::color: 
			colorAttachments.push_back(a);
			break;
		case RpAttachment::Type::resolve: 
			resolveAttachments.push_back(a);
			break;
		case RpAttachment::Type::depthStencil:
			assert(!hasDepthStencil && "cannot add more than one depth-stencil attachment");
			depthStencilAttachment = a;
			hasDepthStencil = true;
			break;
		}
	}


	Renderpass::Renderpass(EngineDevice& device, RpAttachmentsInfo a)
	{
		const auto numColor = a.colorAttachments.size();
		const auto numResolve = a.resolveAttachments.size();
		assert(numResolve == 0 || numResolve == numColor && "resolve attachments color attachments counts mismatch");
		assert(a.hasDepthStencil || numColor > 0 && "no color or depth-stencil attachments specified");

		// descriptions for all the attachments
		std::vector<VkAttachmentDescription> allAtt{};		allAtt.reserve(numColor + numResolve + a.hasDepthStencil);

		// color attachment references
		std::vector<VkAttachmentReference> colorRefs{};		colorRefs.reserve(numColor);
		for (auto& x : a.colorAttachments) 
		{
			VkAttachmentReference ref{};
			ref.layout = a.colorLayout;
			ref.attachment = x.i;
			colorRefs.push_back(ref);
			allAtt.push_back(x.d);
		}

		// resolve attachment references
		std::vector<VkAttachmentReference> resolveRefs{};	resolveRefs.reserve(numResolve);
		for (auto& x : a.resolveAttachments)
		{
			VkAttachmentReference ref{};
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ref.attachment = x.i;
			resolveRefs.push_back(ref);
			allAtt.push_back(x.d);
		}

		// depth-stencil attachment reference
		VkAttachmentReference depthStencilRef{};
		if (a.hasDepthStencil) 
		{
			VkAttachmentReference ref{};
			ref.layout = a.depthStencilLayout;
			ref.attachment = a.depthStencilAttachment.i;
			depthStencilRef = ref;
			allAtt.push_back(a.depthStencilAttachment.d);
		}
		
		// create subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // may need its own parameter in the future
		// subpass.inputAttachmentCount could be set here
		subpass.colorAttachmentCount = numColor;
		subpass.pColorAttachments = (numColor > 0) ? colorRefs.data() : NULL;
		subpass.pResolveAttachments = (numResolve > 0) ? resolveRefs.data() : NULL;
		subpass.pDepthStencilAttachment = (a.hasDepthStencil) ? &depthStencilRef : NULL;

		// TODO
		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(allAtt.size());
		renderPassInfo.pAttachments = allAtt.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create renderpass"); }
	}

	void Renderpass::begin(VkCommandBuffer commandBuffer, uint32_t currentFrame) 
	{
		assert(isFrameStarted && "beginSwapchainRenderPass failed, no frame in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "cannot begin renderpass on commandbuffer from other frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass;
		renderPassInfo.framebuffer = framebuffer.get(currentFrame);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE); // inline = no secondary cmdbuffers

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, swapchain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

}

