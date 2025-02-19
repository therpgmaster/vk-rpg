#include "Core/Render/ShadowmapRenderer.h"
#include "Core/GPU/Device.h"
#include "Core/EngineSettings.h"
#include "Core/GPU/Swapchain.h"

#include <stdexcept>
#include <array>
#include <cassert>

namespace EngineCore
{
	ShadowmapRenderer::ShadowmapRenderer(EngineDevice& device, const EngineSwapChain& swapchain)
		: device{device}
	{
		// TODO: add lights dynamically
		AttachmentProperties colorProps(AttachmentType::COLOR);
		colorProps.extent = { .width = shadowmapResolution, .height = shadowmapResolution };
		colorProps.imageCount = EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
		colorProps.samples = VK_SAMPLE_COUNT_1_BIT;
		colorProps.format = swapchain.getImageFormat();
		AttachmentProperties depthProps = colorProps;
		depthProps.type = AttachmentType::DEPTH;
		depthProps.format = swapchain.getDepthFormat();
		shadowmaps.push_back(std::make_unique<Shadowmap>(device, depthProps, colorProps));

		createRenderpass();
	}

	void ShadowmapRenderer::createRenderpass()
	{
		using Load = AttachmentLoadOp;
		using Store = AttachmentStoreOp;

		// not only does multiple buffering require 2+ copies of each framebuffer (and image)
		// but these are duplicated for each light as well

		// TODO: maybe create "framebuffer sets" in Renderpass, instead of a single list, each set would correspond to a light
		
		const std::vector<AttachmentUse> attachmentUses =
		{
				AttachmentUse(colorAttachment, Load::CLEAR, Store::STORE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL),

				AttachmentUse(depthAttachment, Load::CLEAR, Store::STORE,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL),
		};

		// bound to descriptor set to be sampled in later renderpasses
		depthImageViews = depthAttachment.getImageViews();
		colorImageViews = colorAttachment.getImageViews();

		renderpass = std::make_unique<Renderpass>(device, attachmentUses, colorProps.extent, colorProps.imageCount);

	}


	void ShadowmapRenderer::beginRenderpass(VkCommandBuffer cmdBuffer, uint32_t lightIndex) { renderpass->begin(cmdBuffer, lightIndex, currentFrameIndex); }
	void ShadowmapRenderer::endRenderpass() { vkCmdEndRenderPass(commandBuffers[currentFrameIndex]); }

	VkCommandBuffer ShadowmapRenderer::beginFrame()
	{
		assert(!isFrameStarted && "beginFrame failed, frame already in progress");
		
		auto result = swapchain->acquireNextImage(&currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR) 
		{ 
			create(); // recreate swapchain and renderpasses
			return nullptr; 
		} 
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swapchain image"); }

		isFrameStarted = true;
		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command buffer"); }

		return commandBuffer;
	}

	void ShadowmapRenderer::endFrame()
	{
		assert(isFrameStarted && "endFrame failed, no frame in progress");

		auto commandBuffer = getCurrentCommandBuffer();

		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
		{ throw std::runtime_error("failed to record command buffer"); }

		auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.wasWindowResized())
		{
			window.resetWindowResizedFlag();
			create();
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swapchain image");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % EngineSwapChain::MAX_FRAMES_IN_FLIGHT;
	}


	//VkRenderPass Renderer::getSwapchainRenderPass() const { return swapchain->getRenderPass(); }

	float ShadowmapRenderer::getSwapchainAspectRatio() const { return swapchain->getExtentAspectRatio(); }

}