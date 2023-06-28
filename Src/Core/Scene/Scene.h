#pragma once

#include <vector>
#include <memory>
#include <string>

namespace EngineCore
{
	class EngineDevice;
	class Primitive;
	class Material;
	struct MaterialCreateInfo;

	class Scene
	{
	public:
		Scene(EngineDevice& device);

		void addPrimitive(const std::string& filePath);
		std::shared_ptr<Material> addMaterial(const MaterialCreateInfo& info);

	private:
		EngineDevice& device;
		std::vector<std::unique_ptr<Primitive>> primitives;
		std::vector<std::shared_ptr<Material>> materials;

	};

}