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

namespace DAVA
{
    static FastName PASS_ZPRE_PASS("ZPrePass");
    static FastName PASS_FORWARD_PASS("ForwardPass");
    static FastName PASS_DEFERRED_PASS("DeferredPass");

RenderSystem::RenderSystem()
    : entityObjectMap(2048 /* size of hash table */, 0 /* defaut value */)
{
    // Build forward renderer.
    renderPassesMap.Insert("ZPrePass", new RenderPass("ZPrePass"));
    renderPassesMap.Insert("ForwardPass", new RenderPass("ForwardPass"));
    
    // renderPasses.push_back(new RenderPass("GBufferPass"));
    // renderPasses.push_back(new LightPrePass("LightPrePass"));
    // renderPasses.push_back(new ShadowMapPass("ShadowMapPass")):

    renderLayersMap.Insert("OpaqueRenderLayer", new RenderLayer("OpaqueRenderLayer"));
    renderLayersMap.Insert("TransclucentRenderLayer", new RenderLayer("TransclucentRenderLayer"));
    
    renderPassOrder.push_back(renderPassesMap[PASS_ZPRE_PASS]);
    renderPassOrder.push_back(renderPassesMap[PASS_FORWARD_PASS]);
    
    RenderPass * forwardPass = renderPassesMap[PASS_FORWARD_PASS];
    forwardPass->AddRenderLayer(renderLayersMap["OpaqueRenderLayer"]);
    forwardPass->AddRenderLayer(renderLayersMap["TransclucentRenderLayer"]);
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
    
void RenderSystem::ImmediateUpdate(SceneNode * entity)
{
    RenderObject * renderObject = entity->GetRenderComponent()->GetRenderObject();
    if (!renderObject)return;
    
    if (renderObject->GetRemoveIndex() == -1) // FAIL, SHOULD NOT HAPPEN
    {
        Logger::Error("Object in entity was replaced suddenly. ");
    }
    
    // Do we need updates??? 
}
    
void RenderSystem::AddEntity(SceneNode * entity)
{
    RenderObject * renderObject = entity->GetRenderComponent()->GetRenderObject();
    if (!renderObject)return;

    entityObjectMap.Insert(entity, renderObject);
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
    
    AddRenderObject(renderObject);
    
    uint32 renderBatchCount = renderObject->GetRenderBatchCount();
    for (uint32 k = 0; k < renderBatchCount; ++k)
    {
        RenderBatch * batch = renderObject->GetRenderBatch(k);
        batch->SetModelMatrix(entity->GetTransformComponent()->GetWorldTransform());
        AddRenderBatch(batch);
    }
}

void RenderSystem::RemoveEntity(SceneNode * entity)
{
    RenderObject * renderObject = entityObjectMap.Value(entity);
    if (!renderObject)return;

	RenderObject * lastRenderObject = renderObjectArray[renderObjectArray.size() - 1];
    renderObjectArray[renderObject->GetRemoveIndex()] = lastRenderObject;
    renderObjectArray.pop_back();
	lastRenderObject->SetRemoveIndex(renderObject->GetRemoveIndex());
    renderObject->SetRemoveIndex(-1);
    
    entityObjectMap.Remove(entity);
    RemoveRenderObject(renderObject);
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
    
void RenderSystem::Process()
{
//    //
//    uint32 size = (uint32)renderObjectArray.size();
//    for (uint32 k = 0; k < renderObjectArray.size(); ++k)
//    {
//        renderObjectArray[k]->Draw();
//    }
    
    uint32 size = (uint32)renderPassOrder.size();
    for (uint32 k = 0; k < size; ++k)
    {
        renderPassOrder[k]->Draw(camera);
    }
}

    
};