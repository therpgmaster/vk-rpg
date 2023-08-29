#pragma once
#include "Core/Types/CommonTypes.h"

#include <memory>
#include <array>

namespace Physics
{
	class Rigidbody;

	class Collision
	{
	public:
		std::array<std::shared_ptr<Rigidbody>, 2> bodies = { nullptr, nullptr };
		float restitution;
		Vec normal;
		float penetrationDepth;
	protected:
		void resolve(float deltaTime);

		float getSeparatingVelocity() const;
	private:
		void resolveVelocity(float deltaTime);
		void resolvePenetration(float deltaTime);

	};

}
