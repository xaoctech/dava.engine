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
#include "Render/Highlevel/RenderObject.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
RenderObject::RenderObject()
    :   type(TYPE_RENDEROBJECT)
    ,   flags(VISIBLE)
    ,   removeIndex(-1)
    ,   debugFlags(0)
    ,   worldTransform(0)
{
    
}
    
RenderObject::~RenderObject()
{
	uint32 size = renderBatchArray.size();
	for(uint32 i = 0; i < size; ++i)
	{
		DVASSERT(renderBatchArray[i]->GetOwnerLayer() == 0);
		renderBatchArray[i]->Release();
	}
}
  
void RenderObject::AddRenderBatch(RenderBatch * batch)
{
	batch->Retain();
	batch->SetRenderObject(this);
    renderBatchArray.push_back(batch);
    if (removeIndex != -1)
    {
        
    }
    
    const AABBox3 & boundingBox = batch->GetBoundingBox();
//    DVASSERT(boundingBox.min.x != AABBOX_INFINITY &&
//             boundingBox.min.y != AABBOX_INFINITY &&
//             boundingBox.min.z != AABBOX_INFINITY);
    
    bbox.AddAABBox(boundingBox);
}

void RenderObject::RemoveRenderBatch(RenderBatch * batch)
{
    batch->SetRenderObject(0);
	batch->Release();
}
    
uint32 RenderObject::GetRenderBatchCount()
{
    return (uint32)renderBatchArray.size();
}
RenderBatch * RenderObject::GetRenderBatch(uint32 batchIndex)
{
    return renderBatchArray[batchIndex];
}

RenderObject * RenderObject::Clone(RenderObject *newObject)
{
	if(!newObject)
	{
		DVASSERT_MSG(IsPointerToExactClass<RenderObject>(this), "Can clone only RenderObject");
		newObject = new RenderObject();
	}

	newObject->type = type;
	newObject->flags = flags;
	newObject->debugFlags = debugFlags;
	//ro->bbox = bbox;
	//ro->worldBBox = worldBBox;

	uint32 size = GetRenderBatchCount();
	for(uint32 i = 0; i < size; ++i)
	{
		newObject->AddRenderBatch(GetRenderBatch(i)->Clone());
	}
    newObject->ownerDebugInfo = ownerDebugInfo;

	return newObject;
}


};
