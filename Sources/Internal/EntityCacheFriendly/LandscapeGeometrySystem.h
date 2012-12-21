#ifndef __DAVAENGINE_SYSTEM_LANDSCAPE_GEOMETRY__
#define __DAVAENGINE_SYSTEM_LANDSCAPE_GEOMETRY__

#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/LandscapeNode.h"

namespace DAVA
{

	class LandscapeGeometrySystem
	{
	public:
		static void Run(Scene * scene)
		{
			TemplatePool<LandscapeNode*> * landscapes = scene->entityManager->GetLinkedTemplatePools<LandscapeNode*>("landscapeNode");


			while(landscapes)
			{
				int32 count = landscapes->GetCount();
				LandscapeNode ** landscape = landscapes->GetPtr(0);
				for(int32 i = 0; i < count; ++i)
				{
					(*landscape)->Draw();

					landscape++;
				}

				landscapes = landscapes->next;
			}
		}
	};

};

#endif //__DAVAENGINE_SYSTEM_LANDSCAPE_GEOMETRY__