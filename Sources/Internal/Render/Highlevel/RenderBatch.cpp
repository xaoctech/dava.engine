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
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Material.h"
#include "Render/RenderDataObject.h"
#include "Render/3D/PolygonGroup.h"
#include "Scene3D/Camera.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
    
RenderBatch::RenderBatch()
    :   ownerLayer(0)
    ,   removeIndex(-1)
{
    dataSource = 0;
    renderDataObject = 0;
    material = 0;
    startIndex = 0;
    indexCount = 0;
    type = PRIMITIVETYPE_TRIANGLELIST;
	renderObject = 0;
}
    
RenderBatch::~RenderBatch()
{
}
    
void RenderBatch::SetPolygonGroup(PolygonGroup * _polygonGroup)
{
    dataSource = SafeRetain(_polygonGroup);
	aabbox = dataSource->GetBoundingBox();
}

void RenderBatch::SetRenderDataObject(RenderDataObject * _renderDataObject)
{
    renderDataObject = SafeRetain(_renderDataObject);
}

void RenderBatch::SetMaterial(Material * _material)
{
    material = SafeRetain(_material);
}

static const uint32 VISIBILITY_CRITERIA = RenderObject::VISIBLE | RenderObject::VISIBLE_AFTER_CLIPPING_THIS_FRAME;
    
void RenderBatch::Draw(Camera * camera)
{
	if(!renderObject)return;
    Matrix4 * worldTransformPtr = renderObject->GetWorldTransformPtr();
    if (!worldTransformPtr)return;
    
    uint32 flags = renderObject->GetFlags();
    if ((flags & VISIBILITY_CRITERIA) != VISIBILITY_CRITERIA)
        return;
	
    Matrix4 finalMatrix = (*worldTransformPtr) * camera->GetMatrix();
    RenderManager::Instance()->SetMatrix(RenderManager::MATRIX_MODELVIEW, finalMatrix);
    material->Draw(dataSource, 0);
}
    
    
const FastName & RenderBatch::GetOwnerLayerName()
{
    static FastName fn("OpaqueRenderLayer");
    return fn;
}

void RenderBatch::SetRenderObject(RenderObject * _renderObject)
{
	renderObject = _renderObject;
}

const AABBox3 & RenderBatch::GetBoundingBox() const
{
    return aabbox;
}


};
