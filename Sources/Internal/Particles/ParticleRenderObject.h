#pragma once

#include "ParticleGroup.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/DynamicBufferAllocator.h"

namespace rhi
{
class VertexLayout;
};

namespace DAVA
{
class ParticleRenderObject : public RenderObject
{
    ParticleEffectData* effectData;
    //Vector<ParticleRenderGroup*> renderGroupCache;
    Vector<RenderBatch*> renderBatchCache;

    //void AppendParticleGroup(const ParticleGroup &group, ParticleRenderGroup *renderGroup, const Vector3& cameraDirection);
    void AppendParticleGroup(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, const Vector3& cameraDirection, Vector3* basisVectors);
    void AppendRenderBatch(NMaterial* material, uint32 particlesCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer);
    void PrepareRenderData(Camera* camera);
    Vector<uint16> indices;
    uint32 sortingOffset;

    uint32 currRenderBatchId;

public:
    ParticleRenderObject(ParticleEffectData* effect);
    ~ParticleRenderObject();

    void PrepareToRender(Camera* camera) override;

    void SetSortingOffset(uint32 offset);

    void BindDynamicParameters(Camera* camera) override;
    void RecalcBoundingBox() override
    {
    }
    void RecalculateWorldBoundingBox() override
    {
        worldBBox = bbox;
    }

private:
    void GenerateRegularLayout(rhi::VertexLayout& layout);
    int32 CalculateParticleCount(const ParticleGroup& group);

    uint32 regularVertexLayoutId = 0;
    uint32 frameBlendVertexLayoutId = 0;
    uint32 flowVertexLayoutId = 0;
    uint32 frameBlendFlowVertexLayoutId = 0;
};
}
