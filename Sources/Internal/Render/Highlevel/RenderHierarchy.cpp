/*==================================================================================
 Copyright (c) 2008, DAVA Consulting, LLC
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the DAVA Consulting, LLC nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
 Revision History:
 * Created by Vitaliy Borodovsky
 =====================================================================================*/
#include "Render/Highlevel/RenderHierarchy.h"
#include "Render/Highlevel/RenderBatchArray.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/SpatialTree.h"

namespace DAVA
{
	
	RenderHierarchy::RenderHierarchy()
	{
		spatialTree = NULL;
	}
	
	RenderHierarchy::~RenderHierarchy()
	{
		SafeDelete(spatialTree);
	}
	
    
	void RenderHierarchy::AddRenderObject(RenderObject * object)
	{
		/*on add calculate valid world bbox*/
		object->RecalculateWorldBoundingBox();
		
		if (spatialTree && !object->GetBoundingBox().IsEmpty())
		{
			if (!(object->GetFlags() & RenderObject::ALWAYS_CLIPPING_VISIBLE))
				spatialTree->AddObject(object);
		}
		
		renderObjectArray.push_back(object);
	}
	
	void RenderHierarchy::RemoveRenderObject(RenderObject *renderObject)
	{
		if (renderObject->GetTreeNodeIndex() != INVALID_TREE_NODE_INDEX)
			spatialTree->RemoveObject(renderObject);
		
		uint32 size = renderObjectArray.size();
		for (uint32 k = 0; k < size; ++k)
		{
			if (renderObjectArray[k] == renderObject)
			{
				renderObjectArray[k] = renderObjectArray[size - 1];
				renderObjectArray.pop_back();
				return;
			}
		}
		DVASSERT(0 && "Failed to find object");
	}
	
	void RenderHierarchy::ObjectUpdated(RenderObject * renderObject)
	{
		DVASSERT(spatialTree);
		if(spatialTree)
		{
			spatialTree->ObjectUpdated(renderObject);
		}
	}
	
	void RenderHierarchy::CreateSpatialTree()
	{
		SafeDelete(spatialTree);
		AABBox3 worldBox;
		uint32 size = renderObjectArray.size();
		for (uint32 pos = 0; pos < size; ++pos)
		{
			worldBox.AddAABBox(renderObjectArray[pos]->GetWorldBoundingBox());
			/*if (RenderObject::TYPE_LANDSCAPE == renderObjectArray[pos]->GetType())
			 worldBox = renderObjectArray[pos]->GetWorldBoundingBox();*/
			renderObjectArray[pos]->SetTreeNodeIndex(INVALID_TREE_NODE_INDEX);
		}
		if (worldBox.IsEmpty())
			worldBox = AABBox3(Vector3(0,0,0), Vector3(0,0,0));
		spatialTree = new QuadTree(worldBox, 10);
		for (uint32 pos = 0; pos < size; ++pos)
		{
			if (!(renderObjectArray[pos]->GetFlags()&RenderObject::ALWAYS_CLIPPING_VISIBLE))
			{
				spatialTree->AddObject(renderObjectArray[pos]);
			}
			
		}
		
	}
	
	void RenderHierarchy::DebugDrawSpatialTree()
	{
		if (spatialTree)
			spatialTree->DebugDraw();
	}
	
    
	void RenderHierarchy::Clip(Camera * camera, bool updateNearestLights, RenderPassBatchArray * renderPassBatchArray)
	{
		if (!spatialTree)
			CreateSpatialTree();
		
		if (spatialTree)
			spatialTree->UpdateTree();
		
		Frustum * frustum = camera->GetFrustum();
		int32 objectsToClip = 0;
		uint32 size = renderObjectArray.size();
		for (uint32 pos = 0; pos < size; ++pos)
		{
			RenderObject * node = renderObjectArray[pos];
						
			if ((node->GetFlags() & RenderObject::CLIPPING_VISIBILITY_CRITERIA) != RenderObject::CLIPPING_VISIBILITY_CRITERIA)
			{
				continue;
			}
			
			node->RemoveFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
			
			if (node->GetTreeNodeIndex() == INVALID_TREE_NODE_INDEX) //process clipping if not in spatial tree for some reason (eg. no SpatialTree)
			{
				if ((RenderObject::ALWAYS_CLIPPING_VISIBLE & node->GetFlags()) || frustum->IsInside(node->GetWorldBoundingBox()))
				{
					node->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
				}
			}
			else
				objectsToClip++;
			
		}
		
        //uint32 addedToRendering = 0;
		if (spatialTree)
        {
			spatialTree->ProcessClipping(frustum);
            //addedToRendering = 10000000;
		}
		
		for (uint32 pos = 0; pos < size; ++pos)
		{
			RenderObject * renderObject = renderObjectArray[pos];
			
			uint32 flags = renderObject->GetFlags();
            bool isObjectVisible = ((flags & RenderObject::VISIBILITY_CRITERIA) == RenderObject::VISIBILITY_CRITERIA);

			if (!isObjectVisible)
				continue;
			
            //addedToRendering ++;
            
			uint32 batchCount = renderObject->GetRenderBatchCount();
			for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
			{
				RenderBatch * batch = renderObject->GetRenderBatch(batchIndex);
				NMaterial * material = batch->GetMaterial();
				if (material)
				{
					const FastNameSet & layers = material->GetRenderLayers();
					FastNameSet::Iterator layerEnd = layers.End();
					for (FastNameSet::Iterator layerIt = layers.Begin(); layerIt != layerEnd; ++layerIt)
					{
						renderPassBatchArray->AddRenderBatch(layerIt.GetKey(), batch);
					}
				}
			}
		}
	}
    
};