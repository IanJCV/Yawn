#include "world.h"

inline World& World::Get()
{
	static World world;

	return world;
}

inline void World::Update(float dt)
{

}
