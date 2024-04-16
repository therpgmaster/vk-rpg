#pragma once
#include "Core/Types/vk.h"

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

namespace EngineCore
{
	class EngineDevice;
	class Primitive;
	class DescriptorSet;

	class SkyDrawer 
	{
	public:
		SkyDrawer(EngineDevice& device, DescriptorSet& defaultSet, VkRenderPass renderpass, VkSampleCountFlagBits samples);

		void renderSky(VkCommandBuffer commandBuffer, VkDescriptorSet sceneGlobalDescriptorSet, 
						const glm::vec3& observerPosition);

		float skyMeshScale = 1000.f * 10.f;

	private:
		std::unique_ptr<Primitive> skyMesh;
		
	};

}
