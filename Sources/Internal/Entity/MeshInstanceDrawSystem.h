#ifndef __DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__
#define __DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__

#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/MeshInstanceNode.h"

namespace DAVA
{

class MeshInstanceDrawSystem
{
public:
	static void Run(Scene * scene)
	{
		TemplatePool<uint32> * visibilityFlags = scene->entityManager->GetLinkedTemplatePools<uint32>("meshVisibilityFlag");

		while(visibilityFlags)
		{
			EntityFamily * family = visibilityFlags->GetEntityFamily();
			TemplatePool<MeshInstanceNode*> * meshInstances = (TemplatePool<MeshInstanceNode*>*)family->GetPoolByDataName("meshInstanceNode");

			int32 count = visibilityFlags->GetCount();
			uint32 * flag = visibilityFlags->GetPtr(0);
			MeshInstanceNode ** meshInstance = meshInstances->GetPtr(0);
			for(int32 i = 0; i < count; ++i)
			{
				if(!(*flag))
				{
					(*meshInstance)->Draw();
				}

				meshInstance++;
				flag++;
			}

			visibilityFlags = visibilityFlags->next;
		}
	}
};

};

#endif //__DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__
