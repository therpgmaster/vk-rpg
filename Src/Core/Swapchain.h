#pragma once

#include "Core/GPU/Device.h"

// warning-ignore hack only works in header
#pragma warning(push, 0) 
#include <vulkan/vulkan.h> // include vulkan
#pragma warning(pop)

#include <string>
#include <vector>
#include <memory>
#include <cassert>

namespace EngineCore 
{

	class EngineSwapChain 
	{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		EngineSwapChain(EngineDevice& deviceRef, VkExtent2D extent);
		EngineSwapChain(EngineDevice& deviceRef, VkExtent2D extent,
						std::shared_ptr<EngineSwapChain> previous);
		void init();

		~EngineSwapChain();
		EngineSwapChain(const EngineSwapChain&) = delete;
		EngineSwapChain& operator=(const EngineSwapChain&) = delete;

		// public getters
		VkFramebuffer getFrameBuffer(int index) const { return swapChainFramebuffers[index]; }
		VkRenderPass getRenderPass() const { return renderPass; }
		VkImageView getImageView(int index) const { return swapChainImageViews[index]; }
		size_t imageCount() const { return swapChainImages.size(); }
		VkFormat getSwapChainImageFormat() const { return swapChainImageFormat; }
		VkFormat getDepthFormat() const { return swapChainDepthFormat; }
		VkExtent2D getSwapChainExtent() const { return swapChainExtent; }
		uint32_t width() const { return swapChainExtent.width; }
		uint32_t height() const { return swapChainExtent.height; }

		float getExtentAspectRatio() const 
		{
			return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
		}
		
		bool compareSwapFormats(const EngineSwapChain& swapchain) const 
		{
			return (swapchain.swapChainDepthFormat == swapChainDepthFormat)
					&& (swapchain.swapChainImageFormat == swapChainImageFormat); 
		}

		VkResult acquireNextImage(uint32_t* imageIndex);
		VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

	private:
		void createSwapChain();
		void createImageViews();
		void createSyncObjects();
		//void createDepthResources(VkSampleCountFlagBits samples);
		//void createColorResources(VkSampleCountFlagBits samples);
		//void createRenderPass(VkSampleCountFlagBits samples);
		//void createFramebuffers();
		//void createFramebuffers_legacy();
		
		// helpers
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
		VkFormat findDepthFormat();

		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		// renderpass
		VkRenderPass renderPass;
		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> multisampleImages;
		std::vector<VkDeviceMemory> multisampleImageMemorys;
		std::vector<VkImageView> multisampleImageViews;
		// renderpass

		std::vector<VkFramebuffer> swapChainFramebuffers;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;

		EngineDevice& device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<EngineSwapChain> oldSwapChain;

		std::vector<VkSemaphore> imageAvailableSemaphores;
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;
		size_t currentFrame = 0;
	};

}  // namespace