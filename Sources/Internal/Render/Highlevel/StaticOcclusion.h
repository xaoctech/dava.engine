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
#ifndef __DAVAENGINE_STATIC_OCCLUSION__
#define __DAVAENGINE_STATIC_OCCLUSION__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Render/Texture.h"
#include "Render/OcclusionQuery.h"

namespace DAVA
{
class Camera;
class StaticOcclusionRenderPass;
class RenderObject;
class RenderHierarchy;
class RenderPassBatchArray;
class OcclusionQueryManager;
class RenderBatch;
    
class StaticOcclusion
{
public:
    StaticOcclusion();
    ~StaticOcclusion();
    
    void BuildOcclusionInParallel(const AABBox3 & occlusionAreaRect, uint32 xBlockCount, uint32 yBlockCount, uint32 zBlockCount,
                                  Vector<RenderObject*> & renderObjectArray,
                                  RenderHierarchy * renderHierarchy);
    //uint32 CameraToCellIndex(Camera * camera);
    inline OcclusionQueryManager & GetOcclusionQueryManager();
    uint32 * GetCellVisibilityData(Camera * camera);
    uint32 RenderFrame();
    
    void RecordFrameQuery(RenderBatch * batch, OcclusionQueryManagerHandle handle);
    
private:
    AABBox3 GetCellBox(uint32 x, uint32 y, uint32 z);
    
    Vector<RenderObject*> renderObjectArray;
    RenderHierarchy * renderHierarchy;
    RenderPassBatchArray * renderPassBatchArray;
    OcclusionQueryManager manager;
    Vector<std::pair<RenderBatch*, OcclusionQueryManagerHandle> > recordedBatches;
    Map<RenderObject*, uint32> frameLocalOccludedInfo;
    Map<RenderObject*, uint32> frameGlobalOccludedInfo;
    
    AABBox3  occlusionAreaRect;
    uint32 xBlockCount;
    uint32 yBlockCount;
    uint32 zBlockCount;
    uint32 objectCount;
    uint32 currentFrameX;
    uint32 currentFrameY;
    uint32 currentFrameZ;
    Camera * cameras[6];
    StaticOcclusionRenderPass * staticOcclusionRenderPass;
    
    uint32 * visibilityData;
};
    
inline OcclusionQueryManager & StaticOcclusion::GetOcclusionQueryManager()
{
    return manager;
}
    
};

#endif //__DAVAENGINE_STATIC_OCCLUSION__
