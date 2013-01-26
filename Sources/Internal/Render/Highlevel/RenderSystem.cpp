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
#include "Render/Highlevel/RenderSystem.h"
#include "Scene3D/SceneNode.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Frustum.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"

namespace DAVA
{
    static FastName PASS_ZPRE_PASS("ZPrePass");
    static FastName PASS_FORWARD_PASS("ForwardPass");
    static FastName SHADOW_VOLUME_PASS("ShadowVolumePass");
    static FastName PASS_DEFERRED_PASS("DeferredPass");

RenderSystem::RenderSystem()
{
    // Build forward renderer.
    renderPassesMap.Insert("ZPrePass", new RenderPass("ZPrePass"));
    renderPassesMap.Insert("ForwardPass", new RenderPass("ForwardPass"));
    renderPassesMap.Insert("ShadowVolumePass", new RenderPass("ShadowVolumePass"));

    
    // renderPasses.push_back(new RenderPass("GBufferPass"));
    // renderPasses.push_back(new LightPrePass("LightPrePass"));
    // renderPasses.push_back(new ShadowMapPass("ShadowMapPass")):

    renderLayersMap.Insert("OpaqueRenderLayer", new RenderLayer("OpaqueRenderLayer"));
    renderLayersMap.Insert("TransclucentRenderLayer", new RenderLayer("TransclucentRenderLayer"));
    renderLayersMap.Insert("ShadowVolumeLayer", new RenderLayer("ShadowVolumeLayer"));
    
    renderPassOrder.push_back(renderPassesMap[PASS_ZPRE_PASS]);
    renderPassOrder.push_back(renderPassesMap[PASS_FORWARD_PASS]);
    
    RenderPass * forwardPass = renderPassesMap[PASS_FORWARD_PASS];
    forwardPass->AddRenderLayer(renderLayersMap["OpaqueRenderLayer"]);
    forwardPass->AddRenderLayer(renderLayersMap["TransclucentRenderLayer"]);

    RenderPass * shadowVolumePass = renderPassesMap[SHADOW_VOLUME_PASS];
    shadowVolumePass->AddRenderLayer(renderLayersMap["ShadowVolumeLayer"]);
}

RenderSystem::~RenderSystem()
{
    //for (FastNameMap<RenderPass*>::Iterator )
    Logger::Error("Write functions to release data from HashMaps. Need Iterations for HashMap.");
    
    renderLayersMap.Clear();
    renderPassesMap.Clear();
//    uint32 layersSize = (uint32)renderLayers.size();
//    for (uint32 k = 0; k < layersSize; ++k)
//    {
//        SafeDelete(renderLayers[k]);
//    }
//    renderPasses.clear();
//
//    uint32 size = (uint32)renderPasses.size();
//    for (uint32 k = 0; k < size; ++k)
//    {
//        SafeDelete(renderPasses[k]);
//    }
//    renderPasses.clear();
}
    

void RenderSystem::RenderPermanent(RenderObject * renderObject)
{
    renderObject->Retain();
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
    
    AddRenderObject(renderObject);
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        AddRenderBatch(batch);
    }
}

void RenderSystem::RemoveFromRender(RenderObject * renderObject)
{
	uint32 renderBatchCount = renderObject->GetRenderBatchCount();
	for (uint32 k = 0; k < renderBatchCount; ++k)
	{
		RenderBatch * batch = renderObject->GetRenderBatch(k);
		RemoveRenderBatch(batch);
	}

	RenderObject * lastRenderObject = renderObjectArray[renderObjectArray.size() - 1];
    renderObjectArray[renderObject->GetRemoveIndex()] = lastRenderObject;
    renderObjectArray.pop_back();
	lastRenderObject->SetRemoveIndex(renderObject->GetRemoveIndex());
    renderObject->SetRemoveIndex(-1);
    
    RemoveRenderObject(renderObject);

	renderObject->Release();
}

void RenderSystem::AddRenderObject(RenderObject * renderObject)
{
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
    
}

void RenderSystem::AddRenderBatch(RenderBatch * renderBatch)
{
    // Get Layer Name
    FastName name = renderBatch->GetOwnerLayerName();

    RenderLayer * oldLayer = renderBatch->GetOwnerLayer();
    if (oldLayer != 0)
    {
        oldLayer->RemoveRenderBatch(renderBatch);
    }
    RenderLayer * layer = renderLayersMap[name];
    layer->AddRenderBatch(renderBatch);
}
    
void RenderSystem::RemoveRenderBatch(RenderBatch * renderBatch)
{
    RenderLayer * oldLayer = renderBatch->GetOwnerLayer();
    if (oldLayer != 0)
    {
        oldLayer->RemoveRenderBatch(renderBatch);
    }
}

void RenderSystem::ImmediateUpdateRenderBatch(RenderBatch * renderBatch)
{
    AddRenderBatch(renderBatch);
}
    
void RenderSystem::SetCamera(Camera * _camera)
{
    camera = _camera;
}

Camera * RenderSystem::GetCamera()
{
	return camera;
}
    
void RenderSystem::ProcessClipping()
{
    int32 objectBoxesUpdated = 0;
    List<RenderObject*>::iterator end = markedObjects.end();
    for (List<RenderObject*>::iterator it = markedObjects.begin(); it != end; ++it)
    {
        RenderObject * obj = *it;
        obj->GetBoundingBox().GetTransformedBox(*obj->GetWorldTransformPtr(), obj->GetWorldBoundingBox());
        FindNearestLights(obj);
        objectBoxesUpdated++;
    }
    markedObjects.clear();
    
//    List<RenderObject*>::iterator endLights = movedLights.end();
//    for (List<LightNode*>::iterator it = movedLights.begin(); it != endLights; ++it)
//    {
//        FindNearestLights(*it);
//    }
    if (movedLights.size() > 0)
    {
        FindNearestLights();
    }
    movedLights.clear();
    
    
    int32 objectsCulled = 0;
    
    Frustum * frustum = camera->GetFrustum();

    uint32 size = renderObjectArray.size();
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject * node = renderObjectArray[pos];
        node->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
        //Logger::Debug("Cull Node: %s rc: %d", node->GetFullName().c_str(), node->GetRetainCount());
        if (!frustum->IsInside(node->GetWorldBoundingBox()))
        {
            node->RemoveFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
            objectsCulled++;
        }
    }
}

void RenderSystem::MarkForUpdate(RenderObject * renderObject)
{
    markedObjects.push_back(renderObject);
}
  
void RenderSystem::MarkForUpdate(LightNode * lightNode)
{
    movedLights.push_back(lightNode);
}
    
void RenderSystem::RegisterForUpdate(IRenderUpdatable * updatable)
{
    objectsForUpdate.push_back(updatable);
}
    
void RenderSystem::UnregisterFromUpdate(IRenderUpdatable * updatable)
{
    uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
		if(objectsForUpdate[i] == updatable)
		{
			objectsForUpdate[i] = objectsForUpdate[size - 1];
			objectsForUpdate.pop_back();
			return;
		}
	}
}
    
void RenderSystem::FindNearestLights(RenderObject * renderObject)
{
    LightNode * nearestLight = 0;
    float32 squareMinDistance = 10000000.0f;
    Vector3 position = renderObject->GetWorldBoundingBox().GetCenter();
    
    uint32 size = lights.size();
    for (uint32 k = 0; k < size; ++k)
    {
        LightNode * light = lights[k];
        
        if (!light->IsDynamic())continue;
        
        const Vector3 & lightPosition = light->GetPosition();
        
        float32 squareDistanceToLight = (position - lightPosition).SquareLength();
        if (squareDistanceToLight < squareMinDistance)
        {
            squareMinDistance = squareDistanceToLight;
            nearestLight = light;
        }
    }
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        batch->GetMaterialInstance()->SetLight(0, nearestLight);
    }
}

void RenderSystem::FindNearestLights()
{
    uint32 size = (uint32)renderObjectArray.size();
    for (uint32 k = 0; k < size; ++k)
    {
        FindNearestLights(renderObjectArray[k]);
    }
}
    
void RenderSystem::AddLight(LightNode * light)
{
    lights.push_back(SafeRetain(light));
    FindNearestLights();
}
    
void RenderSystem::RemoveLight(LightNode * light)
{
    lights.erase(std::remove(lights.begin(), lights.end(), light), lights.end());
}


void RenderSystem::Update(float32 timeElapsed)
{
    ProcessClipping();
    
    uint32 size = objectsForUpdate.size();
	for(uint32 i = 0; i < size; ++i)
	{
        objectsForUpdate[i]->RenderUpdate(timeElapsed);
    }
}

void RenderSystem::Render()
{
    uint32 size = (uint32)renderPassOrder.size();
    for (uint32 k = 0; k < size; ++k)
    {
        renderPassOrder[k]->Draw(camera);
    }
}



    
};