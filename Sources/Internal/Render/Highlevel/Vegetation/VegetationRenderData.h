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

#ifndef __DAVAENGINE_VEGETATIONRENDERDATA_H__
#define __DAVAENGINE_VEGETATIONRENDERDATA_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

#include "Render/RenderDataObject.h"

namespace DAVA
{

struct VegetationVertex
{
    Vector3 coord;
    Vector3 normal;
    Vector3 binormal;
    Vector3 tangent;
    Vector2 texCoord0;
    Vector2 texCoord1;
};

struct SortedBufferItem
{
    RenderDataObject* rdo;
    Vector3 sortDirection;
        
    inline SortedBufferItem();
    inline SortedBufferItem(const SortedBufferItem& src);
    inline ~SortedBufferItem();
    inline void SetRenderDataObject(RenderDataObject* dataObject);
};
    
class VegetationRenderData
{
public:

    VegetationRenderData();
    ~VegetationRenderData();

    inline Vector<VegetationVertex>& GetVertices();
    inline Vector<int16>& GetIndices();
    inline RenderDataObject* GetRenderDataObject();
    inline Vector<Vector<Vector<SortedBufferItem> > >& GetIndexBuffers();
    
    void CreateRenderData();
    void ReleaseRenderData();

private:
        
    Vector<VegetationVertex> vertexData;
    Vector<int16> indexData;
    RenderDataObject* vertexRenderDataObject;
    Vector<Vector<Vector<SortedBufferItem> > > indexRenderDataObject; //resolution - cell - direction
};
};

inline Vector<VegetationVertex>& VegetationRenderData::GetVertices()
{
    return vertexData;
}

inline Vector<int16>& VegetationRenderData::GetIndices()
{
    return indexData;
}

inline RenderDataObject* VegetationRenderData::GetRenderDataObject()
{
    return vertexRenderDataObject;
}

inline Vector<Vector<Vector<SortedBufferItem> > >& VegetationRenderData::GetIndexBuffers()
{
    return indexRenderDataObject;
}

inline VegetationRenderData::PolygonSortData::PolygonSortData()
{
    indices[0] = indices[1] = indices[2] = -1;
    cameraDistance = -1.0f;
}

inline SortedBufferItem::SortedBufferItem()
{
    rdo = NULL;
}

inline SortedBufferItem::SortedBufferItem(const SortedBufferItem& src)
{
    rdo = SafeRetain(src.rdo);
    sortDirection = src.sortDirection;
}

inline SortedBufferItem::~SortedBufferItem()
{
    SafeRelease(rdo);
}

inline void SortedBufferItem::SetRenderDataObject(RenderDataObject* dataObject)
{
    if(dataObject != rdo)
    {
        SafeRelease(rdo);
        rdo = SafeRetain(dataObject);
    }
}

#endif /* defined(__Framework__VegetationRenderData__) */
