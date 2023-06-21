#include "Engine.h"

#include "Core/GPU/Material.h"
#include "Core/GPU/Buffer.h"
#include "Core/GPU/Image.h"

#include <stdexcept>
#include <array>
#include <iostream>
#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace EngineCore
{
	EngineApplication::EngineApplication()
	{
		loadActors();
		// demo textures
		marsTexture = std::make_unique<Image>(device, makePath("Textures/mars6k_v2.jpg"));
		spaceTexture = std::make_unique<Image>(device, makePath("Textures/space.png"));
		// temporary single-camera setup
		camera = Camera(45.f, 0.8f, 10.f);
		camera.transform.translation.x = -8.f;
	}

	void EngineApplication::startExecution()
	{
		setupDescriptors();
		std::vector<VkDescriptorSetLayout> dsetLayout = { dset.getLayout() };

		meshDrawer = std::make_unique<MeshDrawer>(device);
		skyDrawer = std::make_unique<SkyDrawer>(materialsMgr, dsetLayout, device, renderSettings.sampleCountMSAA, renderer.getBaseRenderpass().getRenderpass());
		fxDrawer = std::make_unique<FxDrawer>(device, materialsMgr, renderer.getFxPassInputImageViews(), renderer.getFxPassInputDepthImageViews(),
												dset, renderer.getFxRenderpass().getRenderpass());
		
		setupDefaultInputs();
		applyDemoMaterials(dsetLayout);

		// window event loop
		while (!window.getCloseWindow())
		{
			window.input.resetInputValues(); // reset input values
			window.input.updateBoundInputs(); // get new input states
			window.pollEvents(); // process events in window queue
			render(); // render frame
		}

		// window pending close, wait for GPU
		vkDeviceWaitIdle(device.device());
	}

	void EngineApplication::loadActors() 
	{
		Primitive::MeshBuilder builder{};
		builder.loadFromFile(makePath("Meshes/mars.obj")); // TODO: hardcoded path
		loadedMeshes.push_back(std::make_unique<Primitive>(device, builder));
		loadedMeshes.back()->getTransform().translation = Vec{160.f, 0.f, 0.f};
		loadedMeshes.back()->getTransform().scale = 120.f;

		/*builder.loadFromFile("G:/VulkanDev/VulkanEngine/Core/DevResources/Meshes/sphere.obj");
		loadedMeshes.push_back(new Primitive(device, builder)); 
		loadedMeshes[1]->useFakeScale = true;//FakeScaleTest082 testing on mesh at index 1
		loadedMeshes[1]->getTransform().translation.x = 0.f;
		loadedMeshes[1]->getTransform().scale = 1.f;*/

		return; // TODO: function terminates here!
		for (uint32_t i = 0; i < 1; i++) 
		{ 
			loadedMeshes.push_back(std::make_unique<Primitive>(device, builder));
			loadedMeshes[i]->getTransform().translation.x = 0.5f * i;
			if (i == 1) { loadedMeshes[i]->useFakeScale = true; } 
		}

	}

	void EngineApplication::setupDefaultInputs()
	{
		InputSystem& inputSys = window.input;

		inputSys.captureMouseCursor(true);

		// add binding for forwards (and backwards) movement
		uint32_t fwdAxisIndex = inputSys.addBinding(KeyBinding(GLFW_KEY_W, 1.f), "kbForwardAxis");
		inputSys.addBinding(KeyBinding(GLFW_KEY_S, -1.f), fwdAxisIndex);
		// right/left
		uint32_t rightAxisIndex = inputSys.addBinding(KeyBinding(GLFW_KEY_D, 1.f), "kbRightAxis");
		inputSys.addBinding(KeyBinding(GLFW_KEY_A, -1.f), rightAxisIndex);
		// up/down
		uint32_t upAxisIndex = inputSys.addBinding(KeyBinding(GLFW_KEY_R, 1.f), "kbUpAxis");
		inputSys.addBinding(KeyBinding(GLFW_KEY_F, -1.f), upAxisIndex);
		// move faster
		inputSys.addBinding(KeyBinding(GLFW_KEY_LEFT_SHIFT, 1.f), "kbFasterAxis");
	}
	/*
	void EngineApplication::simulateDistanceByScale(const StaticMesh& mesh, const Transform& cameraTransform)
	{
		// old, regular-precision version
		//const auto actualScale = mesh.transform.scale.x; // uniform scale required! (for now)
		//const auto actualLoc = mesh.transform.translation;
		//const auto camLoc = cameraTransform.translation;
		//
		//const float max = 2.f;
		//const auto dst = Vec::distance(actualLoc, camLoc); std::cout << "\n" << dst;
		//
		//
		//auto dir = Vec::direction(camLoc, actualLoc);
		//const auto offset = dst - max; // offset between actual location and render location, along direction
		//const auto scale = max / dst; // fake scale, decided by observer distance
		//
		//simDistOffsets.translation = actualLoc + (dir * -offset);
		//simDistOffsets.scale = actualScale * scale;

		const float actualScale_f = 696340 * 2000;
		const float actualLoc_f = 2000000 * 1000;

		double actualScale = (double)actualScale_f;
		auto actualLoc = Vector3D<double>{ (double)actualLoc_f, 0.0, 0.0 };
		const auto camLoc = Vector3D<double>{ cameraTransform.translation.x, cameraTransform.translation.y, cameraTransform.translation.z };

		const double max = 8.0;
		const double dst = ddist(actualLoc, camLoc);
		const auto dir = ddir(camLoc, actualLoc);
		const auto offset = dst - max;
		const auto scale = max / dst;

		const Vector3D<double> t = actualLoc + (dir * -offset);
		const double s = actualScale * scale;

		simDistOffsets.translation = { (float)t.x, (float)t.y, (float)t.z };
		simDistOffsets.scale = (float)s;
	}

	double EngineApplication::ddist(const Vector3D<double>& a, const Vector3D<double>& b)
	{
		const double dsq = pow(b.x - a.x, 2) + pow(b.y - a.y, 2) + pow(b.z - a.z, 2);
		return sqrt(dsq);
	}

	Vector3D<double> EngineApplication::ddir(const Vector3D<double>& a, const Vector3D<double>& b) 
	{
		Vector3D<double> k = b - a; // direction
		// normalization to unit
		const auto sum = (k.x * k.x) + (k.y * k.y) + (k.z * k.z);
		const auto f = 1.0 / sqrt(sum);
		return { k.x * f, k.y * f, k.z * f };
	}
		*/
	void EngineApplication::applyWorldOriginOffset(Transform& cameraTransform)
	{
		const auto threshold = 250000.f;
		auto& tft = cameraTransform.translation;

		const auto x = std::max(std::max(abs(tft.x), abs(tft.y)), abs(tft.z));
		if (x < threshold) { return; }

		const auto nw = cameraTransform.translation;

		std::cout << "\n\n\n\n\n rebasing world origin to x:" << nw.x << " y:" << nw.y << " z:" << nw.z;

		cameraTransform.translation = {0.f,0.f,0.f};
		if (loadedMeshes.size() > 0) 
		{
			for (auto& m : loadedMeshes) { m->getTransform().translation = m->getTransform().translation - nw; }
		}
	}

	void EngineApplication::setupDescriptors() 
	{
		// runtime descriptors test
		//UBOCreateInfo ubo1{ device };
		//ubo1.addMember(UBOCreateInfo::mat4); // MVP matrix
		//ubo1.addMember(UBOCreateInfo::vec3); // camera position

		UBO_Struct ubo1{};
		ubo1.add(uelem::mat4); // MVP matrix
		//ubo1.add(std::vector{ uelem::scalar, uelem::vec3 }, 2); // test
		dset.addUBO(ubo1, device);
		// as the demo textures will never be overwritten from the CPU, only one buffer is needed for each, so the view can simply be duplicated
		ImageArrayDescriptor demoTextureArray{};
		demoTextureArray.addImage(std::vector<VkImageView>(EngineSwapChain::MAX_FRAMES_IN_FLIGHT, marsTexture->getView()));
		demoTextureArray.addImage(std::vector<VkImageView>(EngineSwapChain::MAX_FRAMES_IN_FLIGHT, spaceTexture->getView()));
		dset.addImageArray(demoTextureArray);
		dset.addSampler(marsTexture->sampler);
		dset.finalize();
	}

	void EngineApplication::applyDemoMaterials(const std::vector<VkDescriptorSetLayout>& setLayouts)
	{
		// create materials
		ShaderFilePaths shader(makePath("Shaders/shader.vert.spv"), makePath("Shaders/shader.frag.spv"));
		auto mat1 = materialsMgr.createMaterial(MaterialCreateInfo(shader, setLayouts, renderSettings.sampleCountMSAA, renderer.getBaseRenderpass().getRenderpass()));
		//auto mat2 = materialsMgr.createMaterial(MaterialCreateInfo(shader2, setLayout));

		// apply materials
		if (loadedMeshes.size() > 0 && loadedMeshes[0])
		{
			for (auto& m : loadedMeshes)
			{
				//if (m->useFakeScale) { m->setMaterial(mat2); continue; } //FakeScaleTest082
				m->setMaterial(mat1);
			}
		}
		else { throw std::runtime_error("could not access loaded mesh"); }
	}

	void EngineApplication::render() 
	{
		if (auto commandBuffer = renderer.beginFrame())
		{
			const uint32_t frameIndex = renderer.getFrameIndex(); // current framebuffer index
			engineClock.measureFrameDelta(frameIndex);

			glm::mat4 pvm{ 1.f };
			pvm = camera.getProjectionMatrix() * Camera::getWorldBasisMatrix() * camera.getViewMatrix(true);
			dset.writeUBOMember(0, pvm, UBO_Layout::ElementAccessor{ 0, 0, 0 }, frameIndex);

			//float testScalar1 = 1.f - std::sin(engineClock.getElapsed() * 10.f);
			//float testScalar2 = 1.f - std::sin(engineClock.getElapsed() * 50.f);
			//dset.writeUBOMember(0, testScalar1, UBO_Layout::ElementAccessor{ 1, 0, 0 }, frameIndex);
			//dset.writeUBOMember(0, testScalar2, UBO_Layout::ElementAccessor{ 1, 1, 0 }, frameIndex);

			//applyWorldOriginOffset(camera.transform); //(TODO: ) experimental
			moveCamera();

			renderer.beginRenderpassBase(commandBuffer);

			// render sky sphere
			skyDrawer->renderSky(commandBuffer, dset.getDescriptorSet(frameIndex), camera.transform.translation);
			//simulateDistanceByScale(*loadedMeshes[1].get(), camera.transform); //FakeScaleTest082
			// render meshes
			meshDrawer->renderMeshes(commandBuffer, loadedMeshes, engineClock.getDelta(), engineClock.getElapsed(),
										dset.getDescriptorSet(frameIndex), simDistOffsets); //FakeScaleTest082

			renderer.endRenderpass();


			fxDrawer->render(commandBuffer, renderer);

			renderer.endFrame(); // submit command buffer
			camera.aspectRatio = renderer.getAspectRatio();
		}
	}

	void EngineApplication::moveCamera()
	{
		auto lookInput = window.input.getMouseDelta();
		auto mf = window.input.getAxisValue(0);
		auto mr = window.input.getAxisValue(1);
		auto mu = window.input.getAxisValue(2);
		auto xs = window.input.getAxisValue(3) > 0 ? true : false;
		camera.moveInPlaneXY(lookInput, mf, mr, mu, xs, engineClock.getDelta());
	}

} 