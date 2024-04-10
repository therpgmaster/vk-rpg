#pragma once
#include <stdint.h>
#include <vector>
#include <memory>

namespace EngineCore
{
	class Primitive;
}

namespace WorldSystem
{
	class SectorCoord
	{
	public:
		SectorCoord();
		SectorCoord(intmax_t x, intmax_t y, intmax_t z);
		intmax_t x, y, z;
		bool operator==(const SectorCoord& s) const { return x == s.x && y == s.y && z == s.z; } // ==
		bool operator!=(const SectorCoord& s) const { return !(s == *this); } // !=
		SectorCoord operator+(const SectorCoord& s) const { return SectorCoord{ x + s.x, y + s.y, z + s.z }; } // +
		SectorCoord operator-(const SectorCoord& s) const { return SectorCoord{ x - s.x, y - s.y, z - s.z }; } // -
		SectorCoord operator+=(const SectorCoord& s) { *this = s + *this; return *this; } // +=
		SectorCoord operator-=(const SectorCoord& s) { *this = s - *this; return *this; } // -=
	};

	class Sector
	{
	public:
		Sector(const SectorCoord& coord);

		SectorCoord coordinates;
		std::vector<std::unique_ptr<EngineCore::Primitive>> primitives;
		bool isCulled = false;

	};

}