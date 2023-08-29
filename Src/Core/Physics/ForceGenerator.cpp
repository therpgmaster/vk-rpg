#include "Core/Physics/ForceGenerator.h"
#include "Core/Physics/Rigidbody.h"

namespace Physics
{

	void ForceGenerator::applyForces(float deltaTime)
	{
		for (auto& b : bodies) { force(*b.get(), deltaTime); }
	}


	SpringForceGenerator::SpringForceGenerator(const std::shared_ptr<Rigidbody>& other, float restLength, float springConstant)
		: other{ other }, restLength{ restLength }, springConstant{ springConstant } {}

	void SpringForceGenerator::force(Rigidbody& body, float deltaTime)
	{
		Vec f = body.getPosition();
		f -= other->getPosition();
		float magnitude = f.getMagnitude();
		magnitude = std::abs(magnitude - restLength);
		magnitude *= springConstant;

		f.normalize();
		f *= -magnitude;
		body.applyForce(f);
	}

}