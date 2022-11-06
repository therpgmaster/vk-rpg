#pragma once
#include "Core/Types/CommonTypes.h"
#include "Core/World/ECS/ComponentPresence.h"

#include <glm/glm.hpp>

namespace ECS
{
	/* base class for ECS components */
	class Component : public ComponentPresence
	{
		friend class ComponentManager;
	public:
		~Component();
		bool operator==(const Component& comparePtr) const { return &comparePtr == this; }
	
		Transform transform{}; // location/rotation/scale relative to parent

		bool canTick = true; // if true, parent actor will attempt to call user-defined tick logic
		virtual void tick(const float& deltaTime) {};

		ComponentManager* getParent() { return parent; }

	private:
		ComponentManager* parent = nullptr;
		// detaches this component from its parent (friend function)
		void unregister();
	};

} // namespace


