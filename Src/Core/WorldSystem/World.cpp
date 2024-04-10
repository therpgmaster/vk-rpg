#include "Core/GPU/Device.h"
#include "Core/WorldSystem/World.h"
#include "Core/Camera.h"
#include "Core/Primitive.h"


#include <cmath>
#include <algorithm>
#include <iostream>


namespace WorldSystem
{

	World::World(EngineCore::EngineDevice& device)
		: device{ device }, localSectorCoord{ nullptr }
	{
		// create the persistent world sector
		sectors.push_back(std::make_unique<Sector>(SectorCoord(0, 0, 0)));
	}


	void World::sectorUpdate(EngineCore::Camera& camera)
	{
		if (updateSectorCoord(camera.transform.translation));
		{
			// new local sector entered
			loadSector(getLocalSectorCoordinate());
		}
	}

	bool World::updateSectorCoord(Vec& pos)
	{
		SectorCoord coordNew = getLocalSectorCoordinate();
		Vec newLocalPos = pos;
		auto& S = World::SECTOR_SIZE;
		if (pos.x > S)
		{
			coordNew.x += static_cast<intmax_t>(std::floor(pos.x / S));
			newLocalPos.x = (S - (pos.x - S)) * -1;
		}
		else if (pos.x < -S)
		{
			coordNew.x -= static_cast<intmax_t>(std::floor(std::abs(pos.x) / S));
			newLocalPos.x = (S - (std::abs(pos.x) - S));
		}

		if (pos.y > S)
		{
			coordNew.y += static_cast<intmax_t>(std::floor(pos.y / S));
			newLocalPos.y = (S - (pos.y - S)) * -1;
		}
		else if (pos.y < -S)
		{
			coordNew.y -= static_cast<intmax_t>(std::floor(std::abs(pos.y) / S));
			newLocalPos.y = (S - (std::abs(pos.y) - S));
		}

		if (pos.z > S)
		{
			coordNew.z += static_cast<intmax_t>(std::floor(pos.z / S));
			newLocalPos.z = (S - (pos.z - S)) * -1;
		}
		else if (pos.z < -S)
		{
			coordNew.z -= static_cast<intmax_t>(std::floor(std::abs(pos.z) / S));
			newLocalPos.z = (S - (std::abs(pos.z) - S));
		}

		bool enteredNewSector = (coordNew != getLocalSectorCoordinate());
		pos = newLocalPos;
		setLocalSectorCoordinate(coordNew);
		return enteredNewSector;
	}

	Sector& World::loadSector(const SectorCoord& sectorPosition)
	{
		// TODO: allow loading arbitrary sectors from file
		if (sectorPosition != SectorCoord(0,0,0))
		{
			std::cout << "sector loading not implemented for " << sectorPosition.x << ", " << sectorPosition.y << ", " << sectorPosition.z;
			return *sectors.back().get();
		}
		else
		{
			createDemoSector();
		}

		return *sectors.back().get();
	}

	const SectorCoord& World::getLocalSectorCoordinate() const 
	{
		return *localSectorCoord.get();
	}

	void World::setLocalSectorCoordinate(const SectorCoord& coordNew)
	{
		*localSectorCoord.get() = coordNew;
	}

	Vec World::sectorToAbsolute(const SectorCoord& sector, Vec offset)
	{ 
		return Vec(
			(sector.x * SECTOR_SIZE) + offset.x, 
			(sector.y * SECTOR_SIZE) + offset.y, 
			(sector.z * SECTOR_SIZE) + offset.z);
	}


	Vec World::getLocalSectorOriginAbsolute() const
	{
		return sectorToAbsolute(getLocalSectorCoordinate(), Vec::zero());
	}

	void World::forgetSector(const SectorCoord& coord)
	{
		auto it = std::remove_if(sectors.begin(), sectors.end(), [coord](const std::unique_ptr<Sector>& s) { return s->coordinates == coord; });
		assert(it != sectors.end() && "attempted to remove an unknown world sector");
		assert(it->get()->coordinates != getLocalSectorCoordinate() && "attempted to remove the local world sector");
		sectors.erase(it, sectors.end());
	}

	Sector* World::getSector(const SectorCoord& coord)
	{
		auto it = std::find_if(sectors.begin(), sectors.end(), [coord](const std::unique_ptr<Sector>& s) { return s->coordinates == coord; });
		if (it == sectors.end()) { return nullptr ; }
		return it->get();
	}

	void World::createDemoSector()
	{
		sectors.push_back(std::make_unique<Sector>(SectorCoord(0,0,0)));
		auto& sector = *sectors.back();
		setLocalSectorCoordinate(sector.coordinates);

		uint32_t i = 0;
		while (i < 100)
		{
			EngineCore::Primitive::MeshBuilder builder2{};
			builder2.loadFromFile(makePath("Meshes/6star.obj")); // TODO: hardcoded path
			sector.primitives.push_back(std::make_unique<EngineCore::Primitive>(device, builder2));
			sector.primitives.back()->getTransform().translation = Vec{ 17.f + (i * 17.f), 0.f, 0.f };
			sector.primitives.back()->getTransform().scale = 30.f;
			if (i == 1)
			{
				sector.primitives.back()->getTransform().scale *= 5.f;
				sector.primitives.back()->getTransform().translation.x += 300.f;
			}

			i++;
		}
	}

	

}

