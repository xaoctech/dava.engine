#include "DAVAEngine.h"
#include "PoolSystem.h"
#include "EntityManager.h"

using namespace DAVA;

void EntityTest()
{
	new PoolSystem();
	new EntityManager();

	//PoolSystem::Instance()->CreatePool<AABBox3>("meshAABBox");
	Entity * entity = EntityManager::Instance()->CreateEntity();
	//entity->AddComponent("AabboxVisibility");
	//TODO: split Component to Component and ComponentDescriptor

	EntityManager::Instance()->Release();
	PoolSystem::Instance()->Release();
}
