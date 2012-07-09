#ifndef __DAVAENGINE_COMPONENT_MESHINSTANCE__
#define __DAVAENGINE_COMPONENT_MESHINSTANCE__

#include "Entity/Component.h"

namespace DAVA
{
	DECLARE_COMPONENT(MeshInstanceComponent);  

	class MeshInstanceNode;
	void MeshInstanceComponent::Register()
	{   
		RegisterData<MeshInstanceNode*>("meshInstanceNode");
	}

};

#endif //__DAVAENGINE_COMPONENT_MESHINSTANCE__
