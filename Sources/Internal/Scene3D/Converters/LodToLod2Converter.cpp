/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "LodToLod2Converter.h"

#pragma message("LodToLod2Converter remove static int32 emptyEntities = 0;")
static int32 emptyEntities = 0;

void LodToLod2Converter::ConvertLodToV2(Entity * scene)
{
	emptyEntities = 0;
	LodSystem::UpdateEntitiesAfterLoad(scene);
	SearchForLod(scene);
}

void LodToLod2Converter::SearchForLod(Entity * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		Entity * childNode = currentNode->GetChild(c);
		SearchForLod(childNode);
		bool wasReplace = MergeLod(childNode);
		if(wasReplace)
		{
			c--;
		}
	}
}

bool LodToLod2Converter::MergeLod(Entity * entity)
{
	bool res = false;

	Set<Entity*> entitiesToRemove;

	LodComponent * lod = GetLodComponent(entity);
	if(lod)
	{
		RenderComponent * rc = GetRenderComponent(entity);
		RenderObject * ro = 0;
		if(!rc)
		{
			ro = new Mesh();
			rc = new RenderComponent(ro);
			ro->Release();

			ro->SetAABBox(AABBox3(Vector3(0, 0, 0), Vector3(0, 0, 0)));
			entity->AddComponent(rc);
		}
		else
        {
            ro = rc->GetRenderObject();
		}

        DVASSERT(ro);

		Vector<LodComponent::LodData*> lodData;
		lod->GetLodData(lodData);
		uint32 size = lodData.size();
		for(uint32 i = 0; i < size; ++i)
		{
			LodComponent::LodData * data = lodData[i];
			uint32 entitiesCount = data->nodes.size();
			for(uint32 j = 0; j < entitiesCount; ++j)
			{
				emptyEntities++;
				Entity * sourceEntity = data->nodes[j];
				TransformComponent * sourceTransform = GetTransformComponent(sourceEntity);
				RenderObject * sourceRenderObject = GetRenderObject(sourceEntity);
				if(sourceRenderObject)
				{
                    if (sourceTransform->GetLocalTransform() != Matrix4::IDENTITY)
                    {
                        PolygonGroup * pg = sourceRenderObject->GetRenderBatchCount() > 0 ? sourceRenderObject->GetRenderBatch(0)->GetPolygonGroup() : 0;
                        if(pg && bakedPolygonGroups.end() == bakedPolygonGroups.find(pg))
                        {
                            sourceRenderObject->BakeTransform(sourceTransform->GetLocalTransform());
                            bakedPolygonGroups.insert(pg);
                        }
                    }
					
					uint32 sourceRenderBatchCount = sourceRenderObject->GetRenderBatchCount();
					for(uint32 k = 0; k < sourceRenderBatchCount; ++k)
					{
						RenderBatch * sourceRenderBatch = sourceRenderObject->GetRenderBatch(k);
						ro->AddRenderBatch(sourceRenderBatch, data->layer, -1);
					}
				}

				if(sourceEntity->GetChildrenCount() == 0)
				{
#pragma message("LodToLod2Converter::MergeLod maybe merge other components")
					entitiesToRemove.insert(sourceEntity);
				}
				else
				{
					RenderComponent * sourceRenderComponent = GetRenderComponent(sourceEntity);
					sourceEntity->RemoveComponent(Component::RENDER_COMPONENT);
				}

				//remove!!!
				data->nodes.clear();
			}
		}

		ro->RecalcBoundingBox();
	}

#pragma message("LodToLod2Converter::MergeLod removing VisualSceneNode")
	if(entity->GetName() == "VisualSceneNode")
	{
		entitiesToRemove.insert(entity);
		res = true;
	}


	Set<Entity*>::iterator itEnd = entitiesToRemove.end();
	for(Set<Entity*>::iterator it = entitiesToRemove.begin(); it != itEnd; ++it)
	{
        (*it)->GetParent()->RemoveNode(*it);
	}

	return res;
}
