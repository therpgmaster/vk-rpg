#pragma once
#include "Core/GPU/Swapchain.h"
#include "Core/Render/Renderpass.h"
#include "Core/Render/Shadowmap.h"

#include <memory>
#include <vector>
#include <cassert>
#include <functional>

namespace EngineCore
{
	class EngineDevice;
	class EngineSwapChain;
	struct EngineRenderSettings;

	class ShadowmapRenderer
	{
	public:
		ShadowmapRenderer(EngineDevice& device, const EngineSwapChain& swapchain);
		ShadowmapRenderer(const ShadowmapRenderer&) = delete;
		ShadowmapRenderer& operator=(const ShadowmapRenderer&) = delete;

		Renderpass& getRenderpass() { return *renderpass.get(); }
		void beginRenderpass(VkCommandBuffer cmdBuffer);
		void endRenderpass();

	private:
		void createRenderpass();

		std::unique_ptr<Renderpass> renderpass;
		std::vector<std::unique_ptr<Shadowmap>> shadowmaps; // per-light attachments

		uint32_t shadowmapResolution{ 2048 };
		uint32_t shadowmapCount{ 1 }; // we will probably need more than one later

		EngineDevice& device;
		std::vector<VkCommandBuffer> commandBuffers;
		int currentFrameIndex{ 0 }; // index of the current frame, 0 - MAX_FRAMES_IN_FLIGHT
	};

}