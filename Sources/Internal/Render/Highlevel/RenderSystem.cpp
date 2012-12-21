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

namespace DAVA
{

RenderSystem::RenderSystem()
    : entityObjectMap(2048 /* size of hash table */, 0 /* defaut value */)
{
    // Build forward renderer.
    renderPasses.push_back(new RenderPass("ZPrePass"));
    renderPasses.push_back(new RenderPass("ForwardRenderPass"));

    renderLayers.push_back(new RenderLayer("OpaqueRenderLayer"));
    renderLayers.push_back(new RenderLayer("TransclucentRenderLayer"));
}

RenderSystem::~RenderSystem()
{
    uint32 layersSize = (uint32)renderLayers.size();
    for (uint32 k = 0; k < layersSize; ++k)
    {
        SafeDelete(renderLayers[k]);
    }
    renderPasses.clear();

    uint32 size = (uint32)renderPasses.size();
    for (uint32 k = 0; k < size; ++k)
    {
        SafeDelete(renderPasses[k]);
    }
    renderPasses.clear();
}
    
void RenderSystem::AddEntity(SceneNode * entity)
{
    RenderObject * renderObject = entity->GetRenderComponent()->GetRenderObject();
    if (!renderObject)return;

    entityObjectMap.Insert(entity, renderObject);
    renderObjectArray.push_back(renderObject);
    renderObject->SetRemoveIndex((uint32)(renderObjectArray.size() - 1));
    
    AddRenderObject(renderObject);
}

void RenderSystem::RemoveEntity(SceneNode * entity)
{
    RenderObject * renderObject = entityObjectMap.Value(entity);
    if (!renderObject)return;

    renderObjectArray[renderObject->GetRemoveIndex()] = renderObjectArray[renderObjectArray.size() - 1];
    entityObjectMap.Remove(entity);
    RemoveRenderObject(renderObject);
}

void RenderSystem::AddRenderObject(RenderObject * renderObject)
{
//    uint32 
//    
//    for (uint32 layerIndex = 0; layerIndex < layerCount; ++layerIndex)
//    {
//    
//    }
}

void RenderSystem::RemoveRenderObject(RenderObject * renderObject)
{
    
}
    
    

    
void RenderSystem::Process()
{
//    //
//    uint32 size = (uint32)renderObjectArray.size();
//    for (uint32 k = 0; k < renderObjectArray.size(); ++k)
//    {
//        renderObjectArray[k]->Draw();
//    }
    
    uint32 size = (uint32)renderPasses.size();
    for (uint32 k = 0; k < size; ++k)
    {
        renderPasses[k]->Draw();
    }
}

    
};