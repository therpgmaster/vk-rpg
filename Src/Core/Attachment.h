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

}  // namespace
