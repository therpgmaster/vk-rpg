#include "Core/Draw/FxDrawer.h"
#include "Core/Primitive.h"
#include "Core/Types/CommonTypes.h"
#include "Core/GPU/Descriptors.h"
#include "Core/Renderer.h"

namespace EngineCore
{
	FxDrawer::FxDrawer(EngineDevice& device, MaterialsManager& matMgr, const std::vector<VkImageView>& inputImageViews, DescriptorSet& defaultSet, VkRenderPass renderpass)
		: device{ device }, defaultSet{ defaultSet }
	{
		uboSet = std::make_unique<DescriptorSet>(device); // initialized as normal
		UBO_Struct ubo{};
		ubo.add(uelem::vec2); // viewport extent value to be used in shader
		uboSet->addUBO(ubo, device);
		uboSet->finalize();

		// attachments use the same image count as the swapchain, so that number is used instead of MAX_FRAMES_IN_FLIGHT
		attachmentSet = std::make_unique<DescriptorSet>(device, (uint32_t)inputImageViews.size()); // set 2
		ImageArrayDescriptor inputImage{}; // rendered attachment image from the previous renderpass
		inputImage.addImage(inputImageViews);
		attachmentSet->addImageArray(inputImage);
		attachmentSet->finalize();

		// setup material and mesh
		ShaderFilePaths shader(makePath("Shaders/fx_test.vert.spv"), makePath("Shaders/fx_test.frag.spv"));
		auto layouts = std::vector<VkDescriptorSetLayout>{ defaultSet.getLayout(), uboSet->getLayout(), attachmentSet->getLayout() };
		auto material = matMgr.createMaterial(MaterialCreateInfo(shader, layouts, VK_SAMPLE_COUNT_1_BIT, renderpass));

		// setup material for the fullscreen shaders (no mesh)
		ShaderFilePaths fullscreenShader(makePath("Shaders/fullscreen.vert.spv"), makePath("Shaders/fullscreen.frag.spv"));
		MaterialCreateInfo fullscreenInfo(fullscreenShader, layouts, VK_SAMPLE_COUNT_1_BIT, renderpass);
		fullscreenInfo.shadingProperties.useVertexInput = false;
		fullscreenInfo.shadingProperties.cullModeFlags = VK_CULL_MODE_NONE;
		fullscreenMaterial = matMgr.createMaterial(fullscreenInfo);

		Primitive::MeshBuilder builder{};
		builder.loadFromFile(makePath("Meshes/sphere.obj")); // hardcoded path //"Meshes/6star.obj"
		mesh = std::make_unique<Primitive>(device, builder);
		mesh->getTransform().translation = Vec{ 0.f, 0.f, 0.f };
		mesh->getTransform().scale = 25.f;
		mesh->setMaterial(material);
	}

	void FxDrawer::render(VkCommandBuffer cmdBuffer, Renderer& renderer)
	{
		const auto& frameIndex = renderer.getFrameIndex();
		const auto& imageIndex = renderer.getSwapImageIndex();

		// update viewport extent descriptor value
		VkExtent2D extent = renderer.getSwapchainExtent();
		uboSet->writeUBOMember(0, extent, UBO_Layout::ElementAccessor{0, 0, 0}, frameIndex);

		renderer.beginRenderpassFx(cmdBuffer); // FX PASS START

		bindDescriptorSets(cmdBuffer, fullscreenMaterial.get()->getPipelineLayout(), frameIndex, imageIndex);

		// draw fullscreen
		fullscreenMaterial.get()->bindToCommandBuffer(cmdBuffer);
		vkCmdDraw(cmdBuffer, 3, 1, 0, 0);

		bindDescriptorSets(cmdBuffer, mesh->getMaterial()->getPipelineLayout(), frameIndex, imageIndex);

		// draw mesh
		auto* material = mesh->getMaterial();
		material->bindToCommandBuffer(cmdBuffer); 
		
		Material::MeshPushConstants push{};
		push.transform = mesh->getTransform().mat4();
		material->writePushConstantsForMesh(cmdBuffer, push);

		mesh->bind(cmdBuffer);
		mesh->draw(cmdBuffer);

		renderer.endRenderpass(); // FX PASS END
	}

	void FxDrawer::bindDescriptorSets(VkCommandBuffer cmdBuffer, VkPipelineLayout pipelineLayout, uint32_t frameIndex, uint32_t swapImageIndex)
	{
		// note that sets 0-1 use frame index, but set 2 uses swapchain image index
		std::array<VkDescriptorSet, 3> vkSets = { defaultSet.getDescriptorSet(frameIndex), uboSet->getDescriptorSet(frameIndex), attachmentSet->getDescriptorSet(swapImageIndex) };
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, vkSets.data(), 0, nullptr);
	}



} // namespace