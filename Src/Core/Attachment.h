#pragma once
// warning-ignore hack only works in header
#pragma warning(push, 0) 
#include <vulkan/vulkan.h> // include vulkan
#pragma warning(pop)

#include <vector>

namespace EngineCore
{
	// properties used to create an attachment's resources
	struct AttachmentInfo 
	{
		size_t imageCount;
		VkExtent2D extent;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspectFlags;
		VkFormat format;
		VkSampleCountFlagBits samples;
	};

	// handles the image resources for a framebuffer attachment, may be used in multiple framebuffers
	class Attachment
	{
	public:
		Attachment(const AttachmentInfo& info_, class EngineDevice& device);

		// returns a structure for renderpass creation, must be populated further before use
		VkAttachmentDescription getDescription() const;

		VkImageView getImageView() const { return imageView; }
		//VkImage getImage() const { return image; }
		//VkImage getImageMemory() const { return imageMemory; }

	private:
		AttachmentInfo info;
	
		VkImage image;
		VkDeviceMemory imageMemory;
		VkImageView imageView;
	};

	// abstraction for a VkFramebuffer, bound to a specific renderpass at initialization time
	class Framebuffer 
	{
	public: 
		Framebuffer(class EngineDevice& device, std::vector<VkImageView> imageViews, 
					VkRenderPass renderPass, VkExtent2D extent, uint32_t numCopies = 1);

		VkFramebuffer get() const { return framebuffer; }
		const Attachment& getAttachment(uint32_t attIndex = 0) const { return attachments[attIndex]; }
	private:
		VkFramebuffer framebuffer;
		std::vector<Attachment> attachments;

	};

}  // namespace