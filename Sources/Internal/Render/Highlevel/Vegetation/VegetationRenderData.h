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

typedef int32 VegetationIndex;
#define VEGETATION_INDEX_TYPE EIF_32

struct VegetationVertex
{
    Vector3 coord;
    Vector3 normal;
    Vector3 binormal;
    Vector3 tangent;
    Vector2 texCoord0;
};

/////////////////////////////////////////////////////////////////////////////////

struct SortedBufferItem
{
    RenderDataObject* rdo;
    RenderDataObject* rdoAttachment;
    Vector3 sortDirection;
        
    inline SortedBufferItem();
    inline SortedBufferItem(const SortedBufferItem& src);
    inline ~SortedBufferItem();
    inline void SetRenderDataObject(RenderDataObject* dataObject);
    inline void SetRenderDataObjectAttachment(RenderDataObject* dataObject);
};

/////////////////////////////////////////////////////////////////////////////////

struct LayerParams
{
    uint32 maxClusterCount;
    float32 instanceRotationVariation; //in angles
    float32 instanceScaleVariation; //0...1. "0" means no variation, "1" means variation from 0 to full scale
};

/////////////////////////////////////////////////////////////////////////////////
    
class VegetationRenderData
{
public:

    VegetationRenderData();
    ~VegetationRenderData();

    inline Vector<VegetationVertex>& GetVertices();
    inline Vector<VegetationIndex>& GetIndices();
    inline RenderDataObject* GetRenderDataObject();
    inline Vector<Vector<Vector<SortedBufferItem> > >& GetIndexBuffers();
    inline NMaterial* GetMaterial();
    inline void SetMaterial(NMaterial* mat);
    
    void CreateRenderData();
    void ReleaseRenderData();
    
    Vector<Vector<uint32> > instanceCount; //layer - lod
    Vector<Vector<uint32> > vertexCountPerInstance; //layer - lod
    Vector<Vector<uint32> > polyCountPerInstance; //layer - lod

private:
    
    NMaterial* material;
    Vector<VegetationVertex> vertexData;
    Vector<VegetationIndex> indexData;
    RenderDataObject* vertexRenderDataObject;
    Vector<Vector<Vector<SortedBufferItem> > > indexRenderDataObject; //resolution - cell - direction
};

/////////////////////////////////////////////////////////////////////////////////

inline Vector<VegetationVertex>& VegetationRenderData::GetVertices()
{
    return vertexData;
}

inline Vector<VegetationIndex>& VegetationRenderData::GetIndices()
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

inline NMaterial* VegetationRenderData::GetMaterial()
{
    return material;
}
    
inline void VegetationRenderData::SetMaterial(NMaterial* mat)
{
    if(mat != material)
    {
        SafeRelease(material);
        material = SafeRetain(mat);
    }
}

/////////////////////////////////////////////////////////////////////////////////

inline SortedBufferItem::SortedBufferItem()
{
    rdo = NULL;
    rdoAttachment = NULL;
}

inline SortedBufferItem::SortedBufferItem(const SortedBufferItem& src)
{
    rdo = SafeRetain(src.rdo);
    rdoAttachment = SafeRetain(src.rdoAttachment);
    sortDirection = src.sortDirection;
}

inline SortedBufferItem::~SortedBufferItem()
{
    SafeRelease(rdoAttachment);
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

inline void SortedBufferItem::SetRenderDataObjectAttachment(RenderDataObject* dataObject)
{
    if(dataObject != rdoAttachment)
    {
        SafeRelease(rdoAttachment);
        rdoAttachment = SafeRetain(dataObject);
    }
}

};

#endif /* defined(__Framework__VegetationRenderData__) */
