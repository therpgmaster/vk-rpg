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

		std::vector<VkAttachmentReference> colorRefs{};		colorRefs.reserve(numColor);
		std::vector<VkAttachmentReference> resolveRefs{};	resolveRefs.reserve(numResolve);
		VkAttachmentReference depthStencilRef{};

		// color attachment references
		for (auto& x : a.colorAttachments) 
		{
			VkAttachmentReference ref{};
			ref.layout = a.colorLayout;
			ref.attachment = x.index;
			colorRefs.push_back(ref);
		}

		// resolve attachment references
		for (auto& x : a.resolveAttachments)
		{
			VkAttachmentReference ref{};
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			ref.attachment = x.index;
			resolveRefs.push_back(ref);
		}

		// depth-stencil attachment references
		VkAttachmentReference ref{};
		ref.layout = a.depthStencilLayout;
		ref.attachment = a.depthStencilAttachment.index;
		depthStencilRef = ref;

		// create subpass
		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS; // may need its own parameter in the future
		// subpass.inputAttachmentCount could be set here
		subpass.colorAttachmentCount = numColor;
		subpass.pColorAttachments = (numColor > 0) ? colorRefs.data() : NULL;
		subpass.pResolveAttachments = (numResolve > 0) ? resolveRefs.data() : NULL;
		subpass.pDepthStencilAttachment = (a.hasDepthStencil) ? &depthStencilRef : NULL;


		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstSubpass = 0;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = allAttachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device.device(), &renderPassInfo, nullptr, &renderpass) != VK_SUCCESS)
		{ throw std::runtime_error("failed to create renderpass"); }
	}

}

