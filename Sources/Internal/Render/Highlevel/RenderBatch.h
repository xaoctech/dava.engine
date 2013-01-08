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
#ifndef __DAVAENGINE_SCENE3D_RENDER_BATCH_H__
#define	__DAVAENGINE_SCENE3D_RENDER_BATCH_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/FastName.h"
#include "Render/RenderBase.h"
#include "Base/BaseMath.h"

namespace DAVA
{

/*
class RenderCallInstance
{
public:
    VBO *
    IBO *
    NMaterialInstance *
    NMaterial *
    uint32 start;
    uint32 count;
    uint32 primitiveType;
};
*/
class Material;
class PolygonGroup;
class RenderLayer;
class RenderDataObject;
class Camera;
class RenderObject;

class RenderBatch : public BaseObject
{
public:
    RenderBatch();
    virtual ~RenderBatch();
    
    // TEMPORARY
    virtual const FastName & GetOwnerLayerName();
    
    void SetPolygonGroup(PolygonGroup * _polygonGroup);
    inline PolygonGroup * GetPolygonGroup();
    
    void SetRenderDataObject(RenderDataObject * _renderDataObject);
    inline RenderDataObject * GetRenderDataObject();
    
    void SetMaterial(Material * _material);
    inline Material * GetMaterial();

	void SetRenderObject(RenderObject * renderObject);
	inline RenderObject * GetRenderObject();
    
    inline void SetStartIndex(uint32 _startIndex);
    inline void SetIndexCount(uint32 _indexCount);
    
    inline void SetRemoveIndex(RenderLayer * _ownerLayer, uint32 _removeIndex);
    inline uint32 GetRemoveIndex();
    inline RenderLayer * GetOwnerLayer();

    virtual void Draw(Camera * camera);
    
    const AABBox3 & GetBoundingBox() const;
private:
    PolygonGroup * dataSource;
    RenderDataObject * renderDataObject;   // Probably should be replaced to VBO / IBO, but not sure
    Material * material;                    // Should be replaced to NMaterial
	RenderObject * renderObject;
    
    uint32 startIndex;
    uint32 indexCount;
    ePrimitiveType type;
    
    RenderLayer * ownerLayer;
    uint32 removeIndex;
};

inline PolygonGroup * RenderBatch::GetPolygonGroup()
{
    return dataSource;
}
    
inline RenderDataObject * RenderBatch::GetRenderDataObject()
{
    return renderDataObject;
}

inline Material * RenderBatch::GetMaterial()
{
    return material;
}

inline RenderObject * RenderBatch::GetRenderObject()
{
	return renderObject;
}

inline void RenderBatch::SetStartIndex(uint32 _startIndex)
{
    startIndex = _startIndex;
}
    
inline void RenderBatch::SetIndexCount(uint32 _indexCount)
{
    indexCount = _indexCount;
}
    
    
    
inline uint32 RenderBatch::GetRemoveIndex()
{
    return removeIndex;
}
    
inline RenderLayer * RenderBatch::GetOwnerLayer()
{
    return ownerLayer;
}

inline void RenderBatch::SetRemoveIndex(RenderLayer * _ownerLayer, uint32 _removeIndex)
{
    ownerLayer = _ownerLayer;
    removeIndex = _removeIndex;
}
    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDER_BATCH_H__ */

