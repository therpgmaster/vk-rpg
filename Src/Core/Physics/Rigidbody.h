#pragma once
#include "Core/Types/CommonTypes.h"

namespace Physics
{

	class Rigidbody 
	{
	public:
		Rigidbody(const Vec& position, float mass);

		const Vec& getPosition() const { return position; }
		void setPosition(const Vec& p) { position = p; }

		const Vec& getVelocity() const { return velocity; }
		void setVelocity(const Vec& v) { velocity = v; }

		float getMass() const { return 1.f / massInverse; }
		float getMassInverse() const { return massInverse; }
		void setMass(float mass) { massInverse = 1.f / mass; }
		
		
		
		void applyForce(const Vec& f) { accumulatedForces += f; }
		void simulate(float deltaTime);
		void resetForces() { accumulatedForces.zero(); }

	private:
		Vec position;
		Vec velocity;
		Vec acceleration;
		Vec accumulatedForces;
		float massInverse;
		float damping;

		

	};

}