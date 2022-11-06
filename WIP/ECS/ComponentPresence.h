#pragma once
#include <vector>

namespace ECS
{
	// forward-declarations
	namespace EngineCore { class Primitive; }
	namespace Physics { class PhysBody; }

	/*	interface for components that affect physics and/or have primitives to render
		this serves to decouple rendering and physics from the component system */
	class ComponentPresence
	{
	protected:
		std::vector <EngineCore::Primitive*> primitives;
		std::vector <Physics::PhysBody*> physicsBodies;
	public:
		bool hasPrimitives() const { return !primitives.empty(); };
		bool hasPhysics() const { return !physicsBodies.empty(); }
		// returns true if the component should participate in rendering or physics
		bool presence() const { return hasPhysics() || hasPrimitives(); }
		void addPrimitive(EngineCore::Primitive* p) { if (p) { primitives.push_back(p); } }
	};

} // namespace


