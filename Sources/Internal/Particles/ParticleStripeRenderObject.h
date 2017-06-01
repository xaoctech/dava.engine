#pragma once

#include "ParticleGroup.h"
#include "Render/Highlevel/RenderObject.h"

namespace DAVA
{
class ParticleStripeRenderObject : public RenderObject
{
public:
    ParticleStripeRenderObject(ParticleEffectData* effectData_);
    ~ParticleStripeRenderObject();

    void PrepareToRender(Camera* camera) override;
    void BindDynamicParameters(Camera* camera) override;
    void RecalcBoundingBox() override
    {}
    void RecalculateWorldBoundingBox() override
    {
        worldBBox = bbox;
    }

    void SetSortingOffset(uint32 offset);


private:
    struct StripeNode
    {
        float32 lifeime = 0.0f;
        Vector3 position = {};
        Vector3 speed = {};

        StripeNode(float32 lifetime_, Vector3 position_, Vector3 speed_) 
            : lifeime(lifetime_)
            , position(position_)
            , speed(speed_)
        {}

        StripeNode() 
        {}
    };
    List<StripeNode> stripeNodes;
    StripeNode baseNode;

    ParticleEffectData* effectData;
    uint32 layout = 0;
    uint32 stride = 0;
    RenderBatch* batch;
    NMaterial* mat;
    float32 spawnTimer = 0.0f;
    float32 gameTimer = 0.0f;
};
}