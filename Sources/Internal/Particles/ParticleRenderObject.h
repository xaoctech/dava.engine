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
struct ParticleLayer;
class Camera;

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
    enum eParticlePropsOffsets
    {
        FRAME_BLEND = 0,
        FLOW,
        NOISE,
        NOISE_SCROLL,
        FRESNEL_TO_ALPHA
    };

    struct LayoutElement
    {
        rhi::VertexSemantics usage;
        uint32 usageIndex = 0;
        rhi::VertexDataType type;
        uint32 dimension = 0;

        LayoutElement() = default;

        LayoutElement(rhi::VertexSemantics usage_, uint32 usageIndex_, rhi::VertexDataType type_, uint32 dimension_)
            : usage(usage_)
            , usageIndex(usageIndex_)
            , type(type_)
            , dimension(dimension_)
        {
        }
    };
    Map<uint32, LayoutElement> layoutsData;

    uint32 GetVertexStride(ParticleLayer* layer);
    void GenerateBaseLayout(rhi::VertexLayout& layout);
    int32 CalculateParticleCount(const ParticleGroup& group);
    uint32 SelectLayout(const ParticleLayer& layer);

    uint32 regularVertexLayoutId = 0;
    uint32 frameBlendVertexLayoutId = 0;
    uint32 flowVertexLayoutId = 0;
    uint32 frameBlendFlowVertexLayoutId = 0;
    Map<uint32, uint32> layoutMap;

    float FresnelShlick(float32 nDotVInv, float32 bias, float32 power);
};

inline float ParticleRenderObject::FresnelShlick(float32 nDotVInv, float32 bias, float32 power)
{
    return bias + (1.0f - bias) * pow(1.0f - nDotVInv, power);
}
}
