#include "Core/World/ECS/Component.h"
#include "Core/World/ECS/Actor.h"
#include "Core/Physics/Physics.h"
#include <cassert>

namespace ECS 
{
	Component::~Component() { unregister(); }

	void Component::unregister() { if (parent) { parent->releaseComponent(this); } }

} // namespace