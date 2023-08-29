#pragma once
#include "Core/Types/CommonTypes.h"

#include <memory>
#include <vector>

namespace Physics
{
	class Rigidbody;
	class ForceGenerator;

	class PhysicsScene
	{
	public:
		PhysicsScene();
		// temporary test functions
		void setupTest();
		std::vector<Vec> simulate(float deltaTime);

	protected:
		std::vector<std::shared_ptr<Rigidbody>> bodies;
		std::vector<std::shared_ptr<ForceGenerator>> generators;
	};

}
