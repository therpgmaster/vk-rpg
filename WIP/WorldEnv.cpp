#include "Core/World/WorldEnv.h"
#include "Core/World/ECS/Actor.h"


void WorldEnv::addActor(std::unique_ptr<Actor> actor) 
{
	actors.push_back(std::move(actor));
}