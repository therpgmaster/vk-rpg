#pragma once
#include <vector>
#include <memory>

class VkFramebuffer;
class VkRenderPass;
class VkImageView;
class VkExtent2D;
namespace EngineCore 
{
	// abstraction for a VkFramebuffer, bound to a specific renderpass
	class Framebuffer
	{
	public:
		Framebuffer(class EngineDevice& device, class Renderpass renderpass, 
					const std::vector<std::vector<VkImageView>>& imageViews, VkExtent2D extent, uint32_t numCopies = 1);

		VkFramebuffer& get(uint32_t i) { return *framebuffers[i].get(); }
	private:
		std::vector<std::unique_ptr<VkFramebuffer>> framebuffers;
	};
}