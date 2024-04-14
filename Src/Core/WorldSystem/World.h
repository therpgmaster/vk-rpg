#pragma once
#include "Core/Types/CommonTypes.h"
#include "Core/WorldSystem/Sector.h"

#include <stdint.h>
#include <memory>
#include <vector>

namespace EngineCore 
{ 
	class EngineDevice;
	class Camera;
	class EngineApplication;
}

namespace WorldSystem
{
	class World
	{
		static constexpr uint32_t SECTOR_SIZE = 50000; //800000;
	public:
		World(EngineCore::EngineDevice& device, EngineCore::EngineApplication& engine);

		void createDemoSectorContent();
		// checks whether we have moved into a new sector
		void sectorUpdate(EngineCore::Camera& camera);

		const SectorCoord& getLocalSectorCoordinate() const;
		void setLocalSectorCoordinate(const SectorCoord& coordNew);
		static Vec sectorToAbsolute(const SectorCoord& sector, Vec offset = Vec::zero());
		// returns the real physical location of the current sector center, in world units
		Vec getLocalSectorOriginAbsolute() const;
		uint32_t getSectorSize() const { return SECTOR_SIZE; }
		std::vector<std::unique_ptr<Sector>>& getLoadedSectors() { return sectors; }
		Sector& getPersistentSector() { return *sectors[0].get(); }


	private:
		// currently loaded sectors, index 0 is the persistent sector
		std::vector<std::unique_ptr<Sector>> sectors;
		std::unique_ptr<SectorCoord> localSectorCoord;

		bool updateSectorCoord(Vec& pos);
		Sector* getSector(const SectorCoord& coord);
		Sector& loadSector(const SectorCoord& sectorPosition);
		void forgetSector(const SectorCoord& coord);

	private:
		EngineCore::EngineDevice& device;
		EngineCore::EngineApplication& engine;
		
	};

}