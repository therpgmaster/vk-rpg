#include "Core/World/ECS/ComponentManager.h"
#include "Core/World/ECS/Component.h"

#include <cassert>

namespace ECS 
{
	ComponentManager::~ComponentManager() 
	{
		// destroy all components
		for (auto& c : components) { destroyComponent(c); }
	}

	void ComponentManager::registerComponent(Component* c)
	{
		assert(!c->getParent() && "tried to register component with existing parent");
		if (c->getParent() == this) { return; }
		components.push_back(c); // add to record
		c->parent = this;
	}

	void ComponentManager::destroyComponent(Component* c)
	{
		assert(c->getParent() == this || !c->getParent() && "destroying component of different parent, bad practice");
		releaseComponent(c); // unregister
		delete c; // free memory
	}

	bool ComponentManager::handoverComponent(Component* c, ComponentManager* np) 
	{
		if (!np || !releaseComponent(c)) { return false; }
		np->registerComponent(c);
		return true;
	}

	const std::vector<ComponentPresence*>& ComponentManager::getPresenceCache()
	{
		// regen cache (only) if required
		if (presenceCacheStale)
		{
			presenceCache.clear();
			for (auto c : components) 
			{ if (c->presence()) { presenceCache.push_back(static_cast<ComponentPresence*>(c)); } }
			presenceCacheStale = false; // mark cache as up-to-date
		}
		return presenceCache;
	}

	int32_t ComponentManager::getComponentIndex(Component* c)
	{
		for (int32_t i = 0; i < components.size(); i++) { if (components[i] == c) { return i; } }
		return -1;
	}
	
	bool ComponentManager::releaseComponent(Component* c)
	{
		const auto i = getComponentIndex(c);
		if (i < 0) { return false; }
		components.erase(std::next(components.begin(), i));
		c->parent = nullptr;
		return true;
	}

} // namespace