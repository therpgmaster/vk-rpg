#pragma once
// warning-ignore hack only works in header
#pragma warning(push, 0) 
#include <vulkan/vulkan.h> // include vulkan
#pragma warning(pop)

#include <vector>

namespace EngineCore
{
	enum class AttachmentType { COLOR, RESOLVE, DEPTH, DEPTH_STENCIL };
	// used to create an attachment's image resources
	struct AttachmentCreateInfo
	{
		size_t imageCount;
		VkExtent2D extent;
		VkImageUsageFlags usage;
		VkImageAspectFlags aspectFlags;
		VkFormat format;
		VkSampleCountFlagBits samples;
		AttachmentType type;

		AttachmentCreateInfo() = default;
		// sets defaults based on the desired attachment type and swapchain properties
		AttachmentCreateInfo(AttachmentType t, class EngineSwapChain& swp, VkSampleCountFlagBits s);
	};
	
	// handles the image resources for a framebuffer attachment, may be used in multiple framebuffers
	class Attachment
	{
	public:
		Attachment(class EngineDevice& device, const AttachmentCreateInfo& info);
		Attachment(class EngineDevice& device, const AttachmentCreateInfo& info);
		~Attachment();

		const AttachmentCreateInfo& info() const { return createInfo; }
		const std::vector<VkImageView>& getImageViews() const { return imageViews; }
		bool isCompatible(const Attachment& b) const;

	private:
		AttachmentCreateInfo createInfo;
		class EngineDevice& device;
		
		std::vector<VkImage> images;
		std::vector<VkDeviceMemory> imageMemorys;
		std::vector<VkImageView> imageViews;
	};


	// just syntactic sugar, e.g. AttachmentLoadOp::LOAD
	struct AttachmentLoadOp
	{
		const static VkAttachmentLoadOp DONT_CARE = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		const static VkAttachmentLoadOp LOAD = VK_ATTACHMENT_LOAD_OP_LOAD;
		const static VkAttachmentLoadOp CLEAR = VK_ATTACHMENT_LOAD_OP_CLEAR;
	};
	struct AttachmentStoreOp
	{
		const static VkAttachmentStoreOp DONT_CARE = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		const static VkAttachmentStoreOp STORE = VK_ATTACHMENT_STORE_OP_STORE;
	};
	// attachment info for renderpass and framebuffer creation
	struct AttachmentUse
	{
		Attachment& attachment;
		VkAttachmentDescription description;

		AttachmentUse(Attachment& attachment, VkAttachmentLoadOp loadOp, VkAttachmentStoreOp storeOp,
			VkImageLayout initialLayout, VkImageLayout finalLayout);
		void setStencilOps(VkAttachmentLoadOp stencilLoadOp, VkAttachmentStoreOp stencilStoreOp);
	};

}