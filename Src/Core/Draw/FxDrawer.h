#pragma once
#include "Core/vk.h"
#include "Core/GPU/MaterialsManager.h"
#include <array>
#include <memory>
#include <vector>

namespace EngineCore
{
	class EngineDevice;
	class Renderer;
	class DescriptorSet;
	class Primitive;
	class MaterialsManager;
	
	class FxDrawer
	{
	public:
		FxDrawer(EngineDevice& device, MaterialsManager& matMgr, const std::vector<VkImageView>& inputImageViews, DescriptorSet& defaultSet, VkRenderPass renderpass);

		void render(VkCommandBuffer cmdBuffer, Renderer& renderer);

	private:
		EngineDevice& device;
		

		/*	set 1 holds additional data, treated as any other descriptor set (using frames in flight number)
			set 2 is for the the attachment image binding, same number of internal sets as swapchain images */
		DescriptorSet& defaultSet;
		std::unique_ptr<DescriptorSet> uboSet;
		std::unique_ptr<DescriptorSet> attachmentSet;
		std::unique_ptr<Primitive> mesh;
		MaterialHandle fullscreenMaterial;

		void bindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, uint32_t frameIndex, uint32_t swapImageIndex);
	};

} // namespace
