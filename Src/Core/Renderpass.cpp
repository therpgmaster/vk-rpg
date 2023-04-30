#include "Core/Renderpass.h"
#include "Core/GPU/Device.h"
#include "Core/Renderer.h"

#include <array>
#include <stdexcept>
#include <cassert>

namespace EngineCore 
{
	Renderpass::Renderpass(EngineDevice& device, const std::vector<AttachmentUse>& attachmentUses)
		: device{ device }
	{
		attachments.reserve(attachmentUses.size());
		attachmentDescriptions.reserve(attachmentUses.size());
		for (auto& a : attachmentUses)
		{
			attachments.push_back(std::shared_ptr<Attachment>(&a.attachment)); // acquire ownership of attachment
			attachmentDescriptions.push_back(a.description); // add VkAttachmentDescription
		}

		assert(areAttachmentsCompatible() && "failed to create renderpass, incompatible attachment");
		createRenderpass();
		createFramebuffers();
	}

	Renderpass::~Renderpass() 
	{
		for (auto f : framebuffers) { vkDestroyFramebuffer(device.device(), f, nullptr); }
	}

	bool Renderpass::areAttachmentsCompatible() const
	{
		for (const auto& a : attachments) { if (!attachments[0]->isCompatible(*a.get())) { return false; } }
		return true;
	}

	void Renderpass::createRenderpass()
	{
		// descriptions and references for the attachments
		std::vector<VkAttachmentReference> colorRefs, resolveRefs;
		VkAttachmentReference depthStencilRef{};
		bool hasDepthStencil;
		createAttachmentReferences(colorRefs, resolveRefs, depthStencilRef, hasDepthStencil);

		const auto numColor = colorRefs.size();
		const auto numResolve = resolveRefs.size();
		if (numResolve > 0 && numResolve != numColor); { throw std::runtime_error("resolve and color attachment counts mismatch"); }
		if (numColor == 0 && !hasDepthStencil); { throw std::runtime_error("no color or depth-stencil attachments specified"); }

		// create subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = numColor;
		subpass.pColorAttachments = (numColor > 0) ? colorRefs.data() : NULL;
		subpass.pResolveAttachments = (numResolve > 0) ? resolveRefs.data() : NULL;
		subpass.pDepthStencilAttachment = (hasDepthStencil) ? &depthStencilRef : NULL;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// create renderpass
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create renderpass"); }
	}

	void Renderpass::createAttachmentReferences(std::vector<VkAttachmentReference>& color, std::vector<VkAttachmentReference>& resolve,
												VkAttachmentReference& depthStencil, bool& hasDepthStencil)
	{
		hasDepthStencil = false;
		uint32_t i = 0; // index for VkRenderPassCreateInfo pAttachments array (all)
		for (auto& attachment : attachments)
		{
			VkAttachmentReference ref{};
			ref.attachment = i++;

			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			switch (attachment->info().type)
			{
			case AttachmentType::COLOR:
				color.push_back(ref);
				break;

			case AttachmentType::RESOLVE:
				resolve.push_back(ref);
				break;

			case AttachmentType::DEPTH_STENCIL:
				ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				assert(!hasDepthStencil && "cannot add more than one depth stencil attachment");
				hasDepthStencil = true;
				depthStencil = ref;
				break;
			}
		}
	}

	void Renderpass::createFramebuffers()
	{
		const auto count = attachments[0]->info().imageCount;
		const auto extent = attachments[0]->info().extent;

		framebuffers.resize(count);
		for (size_t i = 0; i < count; i++)
		{
			// for each attachment, get the corresponding image view
			std::vector<VkImageView> attachmentViews;
			for (auto& a : attachments) { attachmentViews.push_back(a->getImageViews()[i]); }

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = attachmentViews.size();
			framebufferInfo.pAttachments = attachmentViews.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			{ throw std::runtime_error("failed to create framebuffer"); }
		}
	}

	void Renderpass::begin(const Renderer& renderer)
	{
		begin(renderer.getCurrentCommandBuffer(), renderer.getFrameIndex());
	}

	void Renderpass::begin(VkCommandBuffer cmdBuffer, uint32_t framebufferIndex)
	{
		commandBuffer = cmdBuffer;
		assert(commandBuffer != VK_NULL_HANDLE && "begin renderpass null command buffer");

		const auto extent = attachments[0]->info().extent; // framebuffer extent

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass;
		renderPassInfo.framebuffer = framebuffers[framebufferIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = extent;

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(extent.width);
		viewport.height = static_cast<float>(extent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, extent };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void Renderpass::end()
	{
		assert(commandBuffer != VK_NULL_HANDLE && "end renderpass null command buffer");
		vkCmdEndRenderPass(commandBuffer);
	}

}

