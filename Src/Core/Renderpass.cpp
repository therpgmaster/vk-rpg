#include "Core/Renderpass.h"
#include "Core/GPU/Device.h"

#include <stdexcept>

namespace EngineCore 
{
	AttachmentDescription::AttachmentDescription(std::shared_ptr<Attachment> a, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
									VkImageLayout initialLayout, VkImageLayout finalLayout)
		: a{ a }
	{
		d = {};
		d.loadOp = loadOp;
		d.storeOp = storeOp;
		d.format = a->getInfo().format;
		d.samples = a->getInfo().samples;
		// use setStencilOps function if stencil is used, disabled here by default
		d.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		d.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		
		d.initialLayout = initialLayout;
		d.finalLayout = finalLayout;
	};

	void AttachmentDescription::setStencilOps(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp) 
	{
		d.stencilLoadOp = stencilLoadOp;
		d.stencilStoreOp = stencilStoreOp;
	}


	Renderpass::Renderpass(EngineDevice& device, std::vector<AttachmentDescription> attachments)
		: device{ device }, attachments{ attachments }
	{
		createRenderpass();
		createFramebuffers();
	}

	void Renderpass::createRenderpass()
	{
		auto& atts = attachments;
		// descriptions and references for the attachments
		std::vector<VkAttachmentDescription> all{};			all.reserve(atts.size());
		std::vector<VkAttachmentReference> colorRefs{};		colorRefs.reserve(atts.size());
		std::vector<VkAttachmentReference> resolveRefs{};	resolveRefs.reserve(atts.size());
		VkAttachmentReference depthStencilRef{};
		bool hasDepthStencil = false;

		uint32_t i = 0; // index for VkRenderPassCreateInfo pAttachments array (all)
		for (auto& a : atts) 
		{
			VkAttachmentReference ref{};
			ref.attachment = i++;
			switch (a.a->getInfo().type) 
			{
			case AttachmentType::COLOR:
				ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				colorRefs.push_back(ref);
				break;

			case AttachmentType::RESOLVE:
				ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				resolveRefs.push_back(ref);
				break;

			case AttachmentType::DEPTH_STENCIL:
				ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				depthStencilRef = ref;
				if (hasDepthStencil) { throw std::runtime_error("cannot add more than one depth stencil attachment"); }
				hasDepthStencil = true;
				break;
			}
			all.push_back(a.d);
		}

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
		renderPassInfo.attachmentCount = static_cast<uint32_t>(all.size());
		renderPassInfo.pAttachments = all.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create renderpass"); }
	}

	void Renderpass::createFramebuffers()
	{
		const auto imageCount = attachments[0].a->getInfo().imageCount;

		framebuffers.resize(imageCount);
		for (size_t i = 0; i < imageCount; i++)
		{
			std::vector<VkImageView> atts;		atts.reserve(imageCount);
			for (auto& d : attachments) { atts.push_back(d.a.getImageViews()[i]); }

			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderpass;
			framebufferInfo.attachmentCount = atts.size();
			framebufferInfo.pAttachments = atts.data();
			framebufferInfo.width = attachments[0].a.getInfo().extent.width;
			framebufferInfo.height = attachments[0].a.getInfo().extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(device.device(), &framebufferInfo, nullptr, &framebuffers[i]) != VK_SUCCESS)
			{ throw std::runtime_error("failed to create framebuffer"); }
		}
	}

	void Renderpass::begin(VkCommandBuffer commandBuffer, uint32_t currentFrame) 
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderpass;
		renderPassInfo.framebuffer = framebuffers[currentFrame];

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = attachments[0].a.getInfo().extent;

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

