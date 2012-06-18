#include "Entity/VisibilityAABBoxSystem.h"
#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Frustum.h"

namespace DAVA
{

void DAVA::VisibilityAABBoxSystem::Run(Scene * scene)
{
	Frustum * frustum = scene->GetClipCamera()->GetFrustum(); 
	int32 objectsCulled = 0;

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
			*flag = !frustum->IsInside(box);
			if(*flag)
			{
				objectsCulled++;
			}

			box++;
			flag++;
		}

		boxes = boxes->next;
	}

	Logger::Info("culled %d objects", objectsCulled);
}

};
