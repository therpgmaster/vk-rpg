#include "Core/WorldSystem/Sector.h"
#include "Core/Primitive.h"

namespace WorldSystem
{
	SectorCoord::SectorCoord() : x{ 0 }, y{ 0 }, z{ 0 } {};
	SectorCoord::SectorCoord(intmax_t x, intmax_t y, intmax_t z) : x{ x }, y{ y }, z{ z } {};

	Sector::Sector(const SectorCoord& coord)
		: coordinates{ coord }
	{}



}

