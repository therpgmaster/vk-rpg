#pragma once
// warning-ignore hack only works in header
#pragma warning(push, 0) 
#include <vulkan/vulkan.h> // include vulkan
#pragma warning(pop)

#include <vector>

namespace EngineCore
{
	// properties that specify a certain type of attachment
	struct AttachmentInfo 
	{
		size_t imageCount;
		VkExtent2D extent;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspectFlags;
		VkFormat format;
		VkSampleCountFlagBits samples;
	};


	class Attachment
	{
	public:
		Attachment(const AttachmentInfo& info_) : info{ info_ } {};

		// returns a structure for renderpass creation, must be populated further before use
		VkAttachmentDescription getDescription() const;

		// initializes an attachment's images and memory (this could possibly be moved to the constructor)
		void createResources(class EngineDevice& device);

	private:
		AttachmentInfo info;
	
		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemorys;
		std::vector<VkImageView> imageViews;

	};

	// abstraction for a framebuffer, or a series of them (multi-buffering/swapchain use)
	class Framebuffer 
	{
	public: 
		Framebuffer(class EngineDevice& device, std::vector<VkImageView> imageViews, 
					VkRenderPass renderPass, VkExtent2D extent, uint32_t numCopies = 1);
		VkFramebuffer get(uint32_t i = 0) { return framebuffers[i]; }
	private:
		std::vector<VkFramebuffer> framebuffers;
	};

}  // namespace