#ifndef __DAVAENGINE_COMPONENT_VISIBILITY_AABBOX__
#define __DAVAENGINE_COMPONENT_VISIBILITY_AABBOX__

#include "Entity/Component.h"
#include "Math/AABBox3.h"

namespace DAVA
{

DECLARE_COMPONENT(VisibilityAABBoxComponent);  

void VisibilityAABBoxComponent::Register()
{   
	RegisterData<AABBox3>("meshAABox");
	RegisterData<uint32>("flags");
}

};

#endif //__DAVAENGINE_COMPONENT_VISIBILITY_AABBOX__