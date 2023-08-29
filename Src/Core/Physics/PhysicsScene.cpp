#include "Core/Physics/PhysicsScene.h"
#include "Core/Physics/Rigidbody.h"
#include "Core/Physics/ForceGenerator.h"

namespace Physics
{

	PhysicsScene::PhysicsScene() 
	{
		setupTest();
	}

	void PhysicsScene::setupTest()
	{

	}

	std::vector<Vec> PhysicsScene::simulate(float deltaTime) 
	{
		// keep reading 139 - 7.2 Collision Processing

		for (auto& f : generators) { f->applyForces(deltaTime); }
		for (auto& b : bodies) { b->simulate(deltaTime); }

		return std::vector<Vec>();
	}

}