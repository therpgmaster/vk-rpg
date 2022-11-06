#pragma once

#include <vector>

namespace ECS
{
	// forward-declarations
	class Component;
	class ComponentPresence;

	/* interface to allow attachment and management of components */
	class ComponentManager
	{
	public:
		// constructs and registers a component of the given subclass type
		template <typename T>
		T* addComponent()
		{
			Component* c = T();
			registerComponent(c);
			return c;
		}
		// registers a component as a child, this takes control of its lifetime
		void registerComponent(Component* c);
		// deletes and unregisters a component
		void destroyComponent(Component* c);
		// swaps the component onto a new parent, removing it from this one
		bool handoverComponent(Component* c, ComponentManager* np);
		
		// quickly returns only the components with physical presence, regenerates the cache if necessary
		const std::vector<ComponentPresence*>& getPresenceCache();
		// returns all registered components
		const std::vector<Component*>& getAll() const { return components; }

	protected:
		// returns -1 if the component could not be found
		int32_t getComponentIndex(Component* c);

		// unregisters a component but does NOT free its memory, may cause memory leaks, consider using handover instead
		bool releaseComponent(Component* c);
		friend void Component::unregister(); // used by components when they are unexpectedly destroyed

	private:
		std::vector<Component*> components; // all registered components
		std::vector<ComponentPresence*> presenceCache; // components with renderable primitives and/or physics
		bool presenceCacheStale = true; // indicates if the cache must be regenerated, set when component registry is modified

	};
} // namespace


