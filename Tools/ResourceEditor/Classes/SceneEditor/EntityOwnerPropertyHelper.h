#ifndef __ENTITY_OWNER_PROPERTY_HELPER__
#define __ENTITY_OWNER_PROPERTY_HELPER__

#include "DAVAEngine.h"
#include "Scene3D/Entity.h"

namespace DAVA {
	//entityOwnerPropertyHelper
class EntityOwnerPropertyHelper: public DAVA::StaticSingleton<EntityOwnerPropertyHelper>
{
public:
	void UpdateEntityOwner(Entity* entity);
};

};

#endif /* defined(__ENTITY_OWNER_PROPERTY_HELPER__) */