#ifndef __DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__
#define __DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__

#include "Base/BaseTypes.h"
#include "Entity/EntityManager.h"
#include "Scene3D/Scene.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Utils/StringFormat.h"
#include "Render/Material.h"

namespace DAVA
{

class MeshInstanceDrawSystem
{
public:

	static void Draw(MeshInstanceNode * meshInstance)
	{
		uint32 meshesSize = (uint32)meshInstance->polygroups.size();

		for (uint32 k = 0; k < meshesSize; ++k)
		{
			PolygonGroupWithMaterial * polygroup = meshInstance->polygroups[k];
			Material * material = polygroup->material;
			
			if (material->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
			{
				MeshInstanceNode::LightmapData * data = meshInstance->GetLightmapDataForIndex(k);
				if(data && data->lightmap)
				{
					material->SetSetupLightmap(false);
					material->textures[Material::TEXTURE_DECAL] = data->lightmap;
					material->uvOffset = data->uvOffset;
					material->uvScale = data->uvScale;
				}
				else
				{
					material->SetSetupLightmap(true);
					material->SetSetupLightmapSize(meshInstance->GetCustomProperties()->GetInt32(Format("#%d.lightmap.size", k), 128));
				}
			}

			polygroup->material->Draw(polygroup->GetPolygonGroup(), meshInstance->materialState);
		}
	}

	static void Run(Scene * scene)
	{
		TemplatePool<uint32> * visibilityFlags = scene->entityManager->GetLinkedTemplatePools<uint32>("flags");

		Matrix4 prevMatrix = RenderManager::Instance()->GetMatrix(RenderManager::MATRIX_MODELVIEW); 

		while(visibilityFlags)
		{
			EntityFamily * family = visibilityFlags->GetEntityFamily();
			TemplatePool<MeshInstanceNode*> * meshInstances = (TemplatePool<MeshInstanceNode*>*)family->GetPoolByDataName("meshInstanceNode");
			TemplatePool<Matrix4> * transforms = (TemplatePool<Matrix4>*)family->GetPoolByDataName("transform");

			int32 count = visibilityFlags->GetCount();
			uint32 * flag = visibilityFlags->GetPtr(0);
			MeshInstanceNode ** meshInstancePointer = meshInstances->GetPtr(0);
			Matrix4 * transform = transforms->GetPtr(0);
			for(int32 i = 0; i < count; ++i)
			{
				if((*flag) & SceneNode::NODE_CLIPPED_THIS_FRAME)
				{
					Matrix4 meshFinalMatrix = (*transform) * prevMatrix;
					RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, meshFinalMatrix);
					Draw(*meshInstancePointer);
				}

				transform++;
				meshInstancePointer++;
				flag++;
			}

			visibilityFlags = visibilityFlags->next;
		}

		RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, prevMatrix);
	}
};

};

#endif //__DAVAENGINE_SYSTEM_DRAW_MESHINSTANCE__
