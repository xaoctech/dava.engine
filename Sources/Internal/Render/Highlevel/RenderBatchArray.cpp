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
#include "Render/Highlevel/RenderBatchArray.h"

namespace DAVA
{
    
RenderPassBatchArray::RenderPassBatchArray()
    : layerBatchArrayMap(8)
{
}
    
RenderPassBatchArray::~RenderPassBatchArray()
{
    HashMap<FastName, RenderLayerBatchArray*>::Iterator end = layerBatchArrayMap.End();
    for (HashMap<FastName, RenderLayerBatchArray*>::Iterator it = layerBatchArrayMap.Begin(); it != end; ++it)
    {
        RenderLayerBatchArray * layer = it.GetValue();
        SafeDelete(layer);
    }
}

void RenderPassBatchArray::Clear()
{
    HashMap<FastName, RenderLayerBatchArray*>::Iterator end = layerBatchArrayMap.End();
    for (HashMap<FastName, RenderLayerBatchArray*>::Iterator it = layerBatchArrayMap.Begin(); it != end; ++it)
    {
        it.GetValue()->Clear();
    }    
}

void RenderPassBatchArray::AddRenderBatch(const FastName & name, RenderBatch * renderBatch)
{
    RenderLayerBatchArray * layerBatchArray = layerBatchArrayMap.GetValue(name);
    if (!layerBatchArray)
    {
        layerBatchArray = new RenderLayerBatchArray();
        layerBatchArrayMap.Insert(name, layerBatchArray);
    }
    
    layerBatchArray->AddRenderBatch(renderBatch);
}
    
RenderLayerBatchArray * RenderPassBatchArray::Get(const FastName & name)
{
    return layerBatchArrayMap.GetValue(name);
}

    
    
RenderLayerBatchArray::RenderLayerBatchArray()
    : flags(0)
{
    flags = SORT_ENABLED | SORT_BY_MATERIAL;
}
    
RenderLayerBatchArray::~RenderLayerBatchArray()
{
    
}

void RenderLayerBatchArray::AddRenderBatch(RenderBatch * batch)
{
    DVASSERT(batch->GetRemoveIndex() == -1)
    renderBatchArray.push_back(batch);
    flags |= SORT_REQUIRED;
}

void RenderLayerBatchArray::Clear()
{
    renderBatchArray.clear();
}
    
bool RenderLayerBatchArray::MaterialCompareFunction(const RenderBatchSortItem & a, const RenderBatchSortItem & b)
{
    return a.sortingKey > b.sortingKey;
}

void RenderLayerBatchArray::Sort(Camera * camera)
{
    // Need sort
    if ((flags & SORT_THIS_FRAME) == SORT_THIS_FRAME)
    {
        uint32 renderBatchCount = (uint32)renderBatchArray.size();
        sortArray.resize(renderBatchCount);
        if (flags & SORT_BY_MATERIAL)
        {
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                RenderBatch * batch = renderBatchArray[k];;
                item.renderBatch = batch;
                item.sortingKey = ((pointer_size)renderBatchArray[k]->GetMaterial() & 0x0fffffff) | (batch->GetSortingKey() << 28);
            }
            
            std::stable_sort(sortArray.begin(), sortArray.end(), MaterialCompareFunction);
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                renderBatchArray[k] = item.renderBatch;
                //item.renderBatch->SetRemoveIndex(this, k);
                //removeIndex[item.renderBatch] = k;
            }
            flags &= ~SORT_REQUIRED;
        }
        
        if (flags & SORT_BY_DISTANCE)
        {
            Vector3 cameraPosition = camera->GetPosition();
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                RenderBatch * batch = renderBatchArray[k];;
                item.renderBatch = batch;

                item.sortingKey = 0;
                if (batch)
                {
                    RenderObject * renderObject = batch->GetRenderObject();
                    Vector3 position = renderObject->GetBoundingBox().GetCenter();
                    float32 distance = (position - cameraPosition).Length();
                    
                    item.sortingKey = (((uint32)distance) & 0x0fffffff) | (batch->GetSortingKey() << 24);
                }
            }
            
            std::stable_sort(sortArray.begin(), sortArray.end(), MaterialCompareFunction);
            
            for (uint32 k = 0; k < renderBatchCount; ++k)
            {
                RenderBatchSortItem & item = sortArray[k];
                renderBatchArray[k] = item.renderBatch;
                //item.renderBatch->SetRemoveIndex(this, k);
                //removeIndex[item.renderBatch] = k;
            }
            
            flags |= SORT_REQUIRED;
        }
    }
}
    
uint32 RenderLayerBatchArray::GetRenderBatchCount()
{
    return (uint32)renderBatchArray.size();
}

RenderBatch * RenderLayerBatchArray::Get(uint32 index)
{
    return renderBatchArray[index];
}

};
