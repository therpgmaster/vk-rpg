#include "Core/Renderpass.h"

#include "Core/GPU/Device.h"
#include "Core/Renderer.h"
#include "Core/Attachment.h"

#include <array>
#include <stdexcept>
#include <cassert>

namespace EngineCore 
{
	Renderpass::Renderpass(EngineDevice& device, const std::vector<AttachmentUse>& attachmentUses, 
							VkExtent2D framebufferExtent, uint32_t framebufferCount)
		: device{ device }, attachments{ attachmentUses }, framebufferExtent{ framebufferExtent }, framebufferCount{ framebufferCount }
	{
		assert(areAttachmentsCompatible() && "failed to create renderpass, incompatible attachment");
		createRenderpass();
		createFramebuffers();
	}

	Renderpass::~Renderpass() 
	{
		for (auto f : framebuffers) { vkDestroyFramebuffer(device.device(), f, nullptr); }
		vkDestroyRenderPass(device.device(), renderpass, nullptr);
	}

	bool Renderpass::areAttachmentsCompatible() const
	{
		//for (const auto& a : attachments) { if (!attachments[0]->isCompatible(*a.get())) { return false; } }
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
		if (numResolve > 0 && numResolve != numColor) { throw std::runtime_error("resolve and color attachment counts mismatch"); }
		if (numColor == 0 && !hasDepthStencil) { throw std::runtime_error("no color or depth-stencil attachments specified"); }

		// create subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = (uint32_t)numColor;
		subpass.pColorAttachments = numColor ? colorRefs.data() : NULL;
		subpass.pResolveAttachments = numResolve ? resolveRefs.data() : NULL;
		subpass.pDepthStencilAttachment = hasDepthStencil ? &depthStencilRef : NULL;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		// create renderpass
		auto descriptions = getAttachmentDescriptions();
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(descriptions.size());
		renderPassInfo.pAttachments = descriptions.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create renderpass"); }
	}

	std::vector<VkAttachmentDescription> Renderpass::getAttachmentDescriptions() const 
	{
		std::vector<VkAttachmentDescription> d;
		d.reserve(attachments.size());
		for (const AttachmentUse& a : attachments) { d.push_back(a.description); }
		return d;
	}

	void Renderpass::createAttachmentReferences(std::vector<VkAttachmentReference>& color, std::vector<VkAttachmentReference>& resolve,
												VkAttachmentReference& depthStencil, bool& hasDepthStencil)
	{
		hasDepthStencil = false;
		uint32_t i = 0; // index for VkRenderPassCreateInfo pAttachments array
		for (const AttachmentUse& a : attachments)
		{
			VkAttachmentReference ref{};
			ref.attachment = i++;
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			switch (a.type)
			{
			case AttachmentType::COLOR:
				color.push_back(ref);
				break;
			case AttachmentType::RESOLVE:
				resolve.push_back(ref);
				break;
			case AttachmentType::DEPTH: case AttachmentType::DEPTH_STENCIL:
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
		framebuffers.resize(framebufferCount);
		for (size_t i = 0; i < framebufferCount; i++)
		{
			// for each attachment, get the corresponding view
			std::vector<VkImageView> views;
			views.reserve(attachments.size());
			for (auto& a : attachments) { views.push_back(a.imageViews[i]); }

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(views.size());
			framebufferInfo.pAttachments = views.data();
			framebufferInfo.width = framebufferExtent.width;
			framebufferInfo.height = framebufferExtent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			{ throw std::runtime_error("failed to create framebuffer"); }
		}
	}

	void Renderpass::begin(VkCommandBuffer cmdBuffer, uint32_t framebufferIndex)
	{
		assert(cmdBuffer != VK_NULL_HANDLE && "begin renderpass failed, no command buffer");
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass;
		renderPassInfo.framebuffer = framebuffers[framebufferIndex];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = framebufferExtent;

		//std::array<VkClearValue, 2> clearValues{};
		VkClearValue clearValueColor = { 0.1f, 0.12f, 0.2f, 1.0f };
		VkClearValue clearValueDepth = { 1.f, 0 };

		std::vector<VkClearValue> clearValues;
		clearValues.reserve(attachments.size());
		for (auto& a : attachments) 
		{
			if (Attachment::isColor(a.type)) { clearValues.push_back(clearValueColor); }
			else { clearValues.push_back(clearValueDepth); }
		}


		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmdBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(framebufferExtent.width);
		viewport.height = static_cast<float>(framebufferExtent.height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, framebufferExtent };
		vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
	}

}

