#include "Core/Physics/Collision.h"
#include "Core/Physics/Rigidbody.h"

namespace Physics
{

	void Collision::resolve(float deltaTime) 
	{ 
		resolveVelocity(deltaTime);
		resolvePenetration(deltaTime);
	}

	float Collision::getSeparatingVelocity() const
	{
		Vec relativeVelocity = bodies[0]->getVelocity();
		if (bodies[1]) relativeVelocity -= bodies[1]->getVelocity();
		return (relativeVelocity * normal).getMagnitude(); // is magnitude correct to use here? if everything works, probably yes
	}

	void Collision::resolveVelocity(float deltaTime)
	{
		float separatingVelocity = getSeparatingVelocity();
		if (separatingVelocity > 0) { return; }

		float newSepVelocity = -separatingVelocity * restitution;
		float deltaVelocity = newSepVelocity - separatingVelocity;

		float totalInverseMass = bodies[0]->getMassInverse();
		if (bodies[1]) { totalInverseMass += bodies[1]->getMassInverse(); }

		if (totalInverseMass <= 0) return; // infinite mass

		float impulse = deltaVelocity / totalInverseMass;
		Vec impulsePerInvMass = normal * impulse;

		// apply impulses
		bodies[0]->setVelocity(bodies[0]->getVelocity() + impulsePerInvMass * bodies[0]->getMassInverse());
		if (bodies[1])
		{
			bodies[1]->setVelocity(bodies[1]->getVelocity() + impulsePerInvMass * -bodies[1]->getMassInverse());
		}
	}

	void Collision::resolvePenetration(float deltaTime) 
	{
		if (penetrationDepth <= 0) return;

		float totalInverseMass = bodies[0]->getMassInverse();
		if (bodies[1]) { totalInverseMass += bodies[1]->getMassInverse(); }
		
		if (totalInverseMass <= 0) return; // infinite mass

		Vec movePerInvMass = normal * (-penetrationDepth / totalInverseMass);
		
		// separate
		bodies[0]->setPosition(bodies[0]->getPosition() + movePerInvMass * bodies[0]->getMassInverse());
		if (bodies[1])
		{
			bodies[1]->setPosition(bodies[1]->getPosition() + movePerInvMass * bodies[1]->getMassInverse());
		}
	}



}