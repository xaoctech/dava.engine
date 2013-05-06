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

namespace DAVA
{

RenderHierarchy::RenderHierarchy()
{
}

RenderHierarchy::~RenderHierarchy()
{
}

    
void RenderHierarchy::AddRenderObject(RenderObject * object)
{
    renderObjectArray.push_back(object);
}

void RenderHierarchy::RemoveRenderObject(RenderObject *renderObject)
{
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
    
void RenderHierarchy::Clip(Camera * camera, bool updateNearestLights, RenderPassBatchArray * renderPassBatchArray)
{
    int32 objectsCulled = 0;
    Frustum * frustum = camera->GetFrustum();
    uint32 size = renderObjectArray.size();
    for (uint32 pos = 0; pos < size; ++pos)
    {
        RenderObject * renderObject = renderObjectArray[pos];
        //renderObject->AddFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
        
        uint32 flags = renderObject->GetFlags();
        if ((flags & RenderObject::VISIBILITY_CRITERIA) != RenderObject::VISIBILITY_CRITERIA)
            continue;
        
        if (frustum->IsInside(renderObject->GetWorldBoundingBox()))
        {
            //renderObject->RemoveFlag(RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME);
            uint32 batchCount = renderObject->GetRenderBatchCount();;
            for (uint32 batchIndex = 0; batchIndex < batchCount; ++batchIndex)
            {
                RenderBatch * batch = renderObject->GetRenderBatch(batchIndex);
                NMaterial * material = batch->GetMaterial();
                const FastNameSet & layers = material->GetRenderLayers();
                if (material)
                {
                    FastNameSet::Iterator layerEnd = layers.End();
                    for (FastNameSet::Iterator layerIt = layers.Begin(); layerIt != layerEnd; ++layerIt)
                    {
                        renderPassBatchArray->AddRenderBatch(layerIt.GetKey(), batch);
                    }
                }
            }
            objectsCulled++;
        }
    }
}
    
};