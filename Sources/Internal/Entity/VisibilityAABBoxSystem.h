#ifndef __DAVAENGINE_SYSTEM_VISIBILITY_AABBOX__
#define __DAVAENGINE_SYSTEM_VISIBILITY_AABBOX__

#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Frustum.h"

namespace DAVA
{

class VisibilityAABBoxSystem
{
public:
	static void Run(Scene * scene)
	{
		Frustum * frustum = scene->GetClipCamera()->GetFrustum(); 

		TemplatePool<AABBox3> * boxes = scene->entityManager->GetLinkedTemplatePools<AABBox3>("meshAABox");
		while(boxes)
		{
			EntityFamily * family = boxes->GetEntityFamily();
			TemplatePool<uint32> * visibilityFlags = (TemplatePool<uint32>*)family->GetPoolByDataName("meshVisibilityFlag");

			int32 count = boxes->GetCount();
			AABBox3 * box = boxes->GetPtr(0);
			uint32 * flag = visibilityFlags->GetPtr(0);

			for(int32 i = 0; i < count; ++i)
			{
				*flag = frustum->IsInside(box);

				box++;
				flag++;
			}

			boxes = boxes->next;
		}
	}
};

};

#endif //__DAVAENGINE_SYSTEM_VISIBILITY_AABBOX__