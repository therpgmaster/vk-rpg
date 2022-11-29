#pragma once
// warning-ignore hack only works in header
#pragma warning(push, 0) 
#include <vulkan/vulkan.h> // include vulkan
#pragma warning(pop)

#include <vector>

namespace EngineCore
{
	enum class AttachmentType { COLOR, RESOLVE, DEPTH_STENCIL };
	// used to create an attachment's resources
	struct AttachmentInfo
	{
		size_t imageCount;
		VkExtent2D extent;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspectFlags;
		// also used in renderpass initialization
		VkFormat format;
		VkSampleCountFlagBits samples;
		AttachmentType type;
	};
	
	// handles the image resources for a framebuffer attachment, may be used in multiple framebuffers
	class Attachment
	{
	public:
		Attachment(class EngineDevice& device, const AttachmentInfo& info);
		~Attachment();

		const AttachmentInfo& getInfo() const { return info; }
		const std::vector<VkImageView>& getImageViews() const { return imageViews; }

	private:
		AttachmentInfo info;
		class EngineDevice& device;
	
		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemorys;
		std::vector<VkImageView> imageViews;
	};

	/*class Framebuffer
	{
	public: 
		Framebuffer(class EngineDevice& device, std::vector<VkImageView> imageViews, 
					VkRenderPass renderPass, VkExtent2D extent, uint32_t numCopies = 1);

		VkFramebuffer get() const { return framebuffer; }
		const Attachment& getAttachment(uint32_t attIndex = 0) const { return attachments[attIndex]; }
	private:
		VkFramebuffer framebuffer;
		std::vector<Attachment> attachments;

	};*/

}  // namespace