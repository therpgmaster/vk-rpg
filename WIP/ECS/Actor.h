#pragma once

#include "Core/Types/CommonTypes.h"
#include "Core/World/ECS/ComponentManager.h"

#include <vector>

namespace ECS 
{
	// forward-declarations
	class WorldEnv;

	/*	common base class for actor objects which can exist in a world
		instances could have renderable primitives and/or physics through components with "presence" */
	class Actor : public ComponentManager
	{
		friend WorldEnv;
	public:
		Actor() = default;
		~Actor();

		ActorTransform transform;

		// if false, user defined tick functions will be skipped for this actor and its components
		bool hasTickEnabled = false;

	};
} // namespace
