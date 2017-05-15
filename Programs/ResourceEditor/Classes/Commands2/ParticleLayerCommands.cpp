#include "Commands2/ParticleLayerCommands.h"
#include "Commands2/RECommandIDs.h"

#include "FileSystem/FilePath.h"
#include "Particles/ParticleLayer.h"

using namespace DAVA;

CommandChangeLayerMaterialProperties::CommandChangeLayerMaterialProperties(ParticleLayer* layer_, const FilePath& spritePath, eBlending blending, bool enableFog, bool enableBlending)
    : RECommand(CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES, "Change Layer properties")
    , layer(layer_)
{
    newParams.spritePath = spritePath;
    newParams.blending = blending;
    newParams.enableFog = enableFog;
    newParams.enableBlending = enableBlending;

    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        oldParams.spritePath = layer->spritePath;
        oldParams.blending = layer->blending;
        oldParams.enableFog = layer->enableFog;
        oldParams.enableBlending = layer->enableFrameBlend;
    }
}

void CommandChangeLayerMaterialProperties::Redo()
{
    ApplyParams(newParams);
}

void CommandChangeLayerMaterialProperties::Undo()
{
    ApplyParams(oldParams);
}

void CommandChangeLayerMaterialProperties::ApplyParams(const CommandChangeLayerMaterialProperties::LayerParams& params)
{
    if (layer != nullptr)
    {
        layer->SetSprite(params.spritePath);
        layer->blending = params.blending;
        layer->enableFog = params.enableFog;
        layer->enableFrameBlend = params.enableBlending;
    }
}

DAVA::ParticleLayer* CommandChangeLayerMaterialProperties::GetLayer() const
{
    return layer;
}

CommandChangeFlowProperties::CommandChangeFlowProperties(ParticleLayer* layer_, CommandChangeFlowProperties::FlowParams&& params)
    :RECommand(CMDID_PARTICLE_LAYER_CHANGED_FLOW_VALUES, "Change Flow Properties")
    , layer(layer_)
    , newParams(params)
{
    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        oldParams.spritePath = layer->spritePath;
        oldParams.enableFlow = layer->enableFlow;
        oldParams.flowSpeedOverLife = layer->flowSpeedOverLife;
        oldParams.flowOffsetOverLife = layer->flowOffsetOverLife;
    }
}

void CommandChangeFlowProperties::Undo()
{
    ApplyParams(oldParams);
}

void CommandChangeFlowProperties::Redo()
{
    ApplyParams(newParams);
}

DAVA::ParticleLayer* CommandChangeFlowProperties::GetLayer() const
{
    return layer;
}

void CommandChangeFlowProperties::ApplyParams(FlowParams& params)
{
    if (layer != nullptr)
    {
        layer->enableFlow = params.enableFlow;
        layer->SetFlowmap(params.spritePath);
        PropertyLineHelper::SetValueLine(layer->flowSpeedOverLife, params.flowSpeedOverLife);
        PropertyLineHelper::SetValueLine(layer->flowOffsetOverLife, params.flowOffsetOverLife);
    }
}

CommandChangeNoiseProperties::CommandChangeNoiseProperties(DAVA::ParticleLayer* layer_, NoiseParams&& params)
    : RECommand(CMDID_PARTICLE_LAYER_CHANGED_NOISE_VALUES, "Change Noise Properties")
    , layer(layer_)
    , newParams(params)
{
    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        oldParams.noisePath = layer->noisePath;
        oldParams.enableNoise = layer->enableNoise;
        oldParams.noiseScaleOverLife = layer->noiseScaleOverLife;
        oldParams.enableNoiseScroll = layer->enableNoiseScroll;
        oldParams.noiseUScrollSpeed = layer->noiseUScrollSpeed;
        oldParams.noiseVScrollSpeed = layer->noiseVScrollSpeed;
    }
}

void CommandChangeNoiseProperties::Undo()
{
    ApplyParams(oldParams);
}

void CommandChangeNoiseProperties::Redo()
{
    ApplyParams(newParams);
}

DAVA::ParticleLayer* CommandChangeNoiseProperties::GetLayer() const
{
    return layer;
}

void CommandChangeNoiseProperties::ApplyParams(NoiseParams& params)
{
    if (layer != nullptr)
    {
        layer->enableNoise = params.enableNoise;
        layer->enableNoiseScroll = params.enableNoiseScroll;
        layer->SetNoise(params.noisePath);
        PropertyLineHelper::SetValueLine(layer->noiseScaleOverLife, params.noiseScaleOverLife);
        PropertyLineHelper::SetValueLine(layer->noiseUScrollSpeed, params.noiseUScrollSpeed);
        PropertyLineHelper::SetValueLine(layer->noiseVScrollSpeed, params.noiseVScrollSpeed);
    }
}
