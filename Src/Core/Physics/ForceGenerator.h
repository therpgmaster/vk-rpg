#pragma once
#include <memory>
#include <vector>

namespace Physics
{
	class Rigidbody;

	class ForceGenerator 
	{
	public:
		void addBody(const std::shared_ptr<Rigidbody>& body) { bodies.push_back(body); }
		void applyForces(float deltaTime);

	protected:
		virtual void force(Rigidbody& body, float deltaTime) = 0;

		std::vector<std::shared_ptr<Rigidbody>> bodies;
	};


	class SpringForceGenerator : public ForceGenerator 
	{
	public:
		SpringForceGenerator(const std::shared_ptr<Rigidbody>& other, float restLength, float springConstant);

	protected:
		void force(Rigidbody& body, float deltaTime) override;

		std::shared_ptr<Rigidbody> other;
		float restLength;
		float springConstant;
		
	};

}
