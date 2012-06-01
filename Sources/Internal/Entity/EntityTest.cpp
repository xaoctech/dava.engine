#include "DAVAEngine.h"
#include "PoolSystem.h"

using namespace DAVA;

void EntityTest()
{
	new PoolSystem();

	PoolSystem::Instance()->CreatePool<AABBox3>("meshAABBox");

	PoolSystem::Instance()->Release();
}
