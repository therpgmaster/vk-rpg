#include "Core/Physics/Rigidbody.h"

#include <cmath>

namespace Physics
{
	Rigidbody::Rigidbody(const Vec& position, float mass)
		: position{ position }
	{
		setMass(mass);
	}

	void Rigidbody::simulate(float deltaTime)
	{
		position += velocity * deltaTime;
		auto resAcc = acceleration;
		resAcc += accumulatedForces * massInverse;
		velocity += resAcc * deltaTime;
		velocity *= std::pow(damping, deltaTime);
		resetForces();
	}

}