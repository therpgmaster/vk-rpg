#pragma once
#include <memory>
#include <vector>

class WorldEnv
{
	class Actor; // forward-declaration
public:
	void addActor(std::unique_ptr<Actor> actor);

private:
	std::vector<std::unique_ptr<Actor>> actors;
};