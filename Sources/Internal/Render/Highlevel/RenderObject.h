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
#ifndef __DAVAENGINE_SCENE3D_RENDEROBJECT_H__
#define	__DAVAENGINE_SCENE3D_RENDEROBJECT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"

namespace DAVA
{

/*
class RenderCallInstance
{
public:
    VBO *
    IBO *
    NMaterial *
    uint32 start;
    uint32 count;
    uint32 primitiveType;
};
*/
    
/*
    Types of possible render objects
 
    
    - Mesh(Static)
    - Mesh(Dynamic)
    - 
 
 */

class RenderBatch;
    
class RenderObject : public BaseObject
{
public:
    RenderObject();
    virtual ~RenderObject();
    
    inline void SetRemoveIndex(uint32 removeIndex);
    inline uint32 GetRemoveIndex();
    
    void AddRenderBatch(RenderBatch * batch);
    void RemoveRenderBatch(RenderBatch * batch);
    
    uint32 GetRenderBatchCount();
    RenderBatch * GetRenderBatch(uint32 batchIndex);
    
    inline void SetFlags(uint32 _flags) { flags = _flags; }
    inline uint32 GetFlags() { return flags; }
    
    inline void SetAABBox(const AABBox3 & bbox);
    inline void SetBSphere(const Sphere & sphere);
    
private:
    uint32 flags;
    uint32 debugFlags;
    uint32 removeIndex;
//    AABBox3 bbox;
//    Sphere bsphere;
    
    Vector<RenderBatch*> renderBatchArray;    
};

inline uint32 RenderObject::GetRemoveIndex()
{
    return removeIndex;
}
    
inline void RenderObject::SetRemoveIndex(uint32 _removeIndex)
{
    removeIndex = _removeIndex;
}

    
} // ns

#endif	/* __DAVAENGINE_SCENE3D_RENDEROBJECT_H__ */

