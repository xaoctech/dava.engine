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

CommandChangeFlowProperties::CommandChangeFlowProperties(ParticleLayer* layer_, const FilePath& spritePath, bool enableFlow, RefPtr<PropertyLine<float32>> flowSpeed, RefPtr<PropertyLine<float32>> flowOffset)
    :RECommand(CMDID_PARTICLE_LAYER_CHANGED_FLOW_VALUES, "Change Flow Properties")
    , layer(layer_)
{
    newParams.spritePath = spritePath;
    newParams.enableFlow = enableFlow;
    newParams.flowSpeed = flowSpeed;
    newParams.flowOffset = flowOffset;

    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        oldParams.spritePath = layer->spritePath;
        oldParams.enableFlow = layer->enableFlow;
        oldParams.flowSpeed = layer->flowSpeed;
        oldParams.flowOffset = layer->flowOffset;
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
        PropertyLineHelper::SetValueLine(layer->flowSpeed, params.flowSpeed);
        PropertyLineHelper::SetValueLine(layer->flowOffset, params.flowOffset);
    }
}
