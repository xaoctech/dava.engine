#ifndef __DAVAENGINE_COMPONENT_TRANSFOERM__
#define __DAVAENGINE_COMPONENT_TRANSFOERM__

#include "Entity/Component.h"
#include "Math/Matrix4.h"

namespace DAVA
{
	DECLARE_COMPONENT(TransformComponent);  

	void TransformComponent::Register()
	{   
		RegisterData<Matrix4>("transform");
	}

};

#endif //__DAVAENGINE_COMPONENT_TRANSFOERM__