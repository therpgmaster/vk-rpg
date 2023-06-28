#include "Core/Scene/Scene.h"
#include "Core/GPU/Device.h"
#include "Core/Primitive.h"
#include "Core/GPU/Material.h"

#include <stdexcept>

namespace EngineCore
{
	Scene::Scene(EngineDevice& device) 
		: device{ device }{}

	void Scene::addPrimitive(const std::string& filePath)
	{
		Primitive::MeshBuilder builder{};
		builder.loadFromFile(filePath);
		primitives.push_back(std::make_unique<Primitive>(device, builder));
	}

	std::shared_ptr<Material> Scene::addMaterial(const MaterialCreateInfo& info)
	{
		materials.push_back(std::make_shared<Material>(info, device));
		return materials.back();
	}

	
}

