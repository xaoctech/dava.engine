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
    void AppendStripeParticle(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, const Vector3& cameraDirection, Vector3* basisVectors);
    void AppendRenderBatch(NMaterial* material, uint32 particlesCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer);
    void AppendRenderBatch(NMaterial* material, uint32 particlesCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer, const rhi::HIndexBuffer iBuffer);
    void PrepareRenderData(Camera* camera);
    bool CheckIfSimpleParticle(ParticleLayer* layer) const;
    void UpdateStripe(Particle* particle, ParticleLayer* layer, Vector3* basisVectors);

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

    struct StripeNode
    {
        float32 lifeime = 0.0f;
        Vector3 position = {};
        Vector3 speed = {};
        Vector3 right = {};

        StripeNode(float32 lifetime_, Vector3 position_, Vector3 speed_, Vector3 right_)
            : lifeime(lifetime_)
            , position(position_)
            , speed(speed_)
            , right(right_)
        {}

        StripeNode()
        {}
    };
    struct StripeData
    {
        Map<int32, List<StripeNode>> strpeNodes; // 1 stripe for basis.
        StripeNode baseNode = {};
        float32 spawnTimer = 0;
    };
    Map<Particle*, StripeData> stripes;
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

    float FresnelShlick(float32 nDotVInv, float32 bias, float32 power) const;
};

inline float ParticleRenderObject::FresnelShlick(float32 nDotVInv, float32 bias, float32 power) const
{
    return bias + (1.0f - bias) * pow(1.0f - nDotVInv, power);
}

inline bool ParticleRenderObject::CheckIfSimpleParticle(ParticleLayer* layer) const
{
    return layer->type == ParticleLayer::eType::TYPE_PARTICLES
        || layer->type == ParticleLayer::eType::TYPE_SINGLE_PARTICLE;
}
}
