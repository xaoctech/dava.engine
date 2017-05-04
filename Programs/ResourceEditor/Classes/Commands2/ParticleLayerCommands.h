#ifndef __PARTICLE_LAYER_COMMANDS_H__
#define __PARTICLE_LAYER_COMMANDS_H__

#include "Commands2/Base/RECommand.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{
struct ParticleLayer;
class FilePath;
}

class CommandChangeLayerMaterialProperties : public RECommand
{
public:
    CommandChangeLayerMaterialProperties(DAVA::ParticleLayer* layer, const DAVA::FilePath& spritePath, DAVA::eBlending blending, bool enableFog, bool enableBlending);

    void Undo() override;
    void Redo() override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    struct LayerParams
    {
        DAVA::FilePath spritePath;
        DAVA::eBlending blending = DAVA::BLENDING_NONE;
        bool enableFog = false;
        bool enableBlending = false;
    };

    void ApplyParams(const LayerParams& params);

private:
    LayerParams newParams;
    LayerParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};

class CommandChangeFlowProperties : public RECommand
{
public:
    CommandChangeFlowProperties(DAVA::ParticleLayer* layer_, const DAVA::FilePath& spritePath, bool enableFlow, RefPtr<PropertyLine<float32>> flowSpeed, RefPtr<PropertyLine<float32>> flowOffset);

    void Undo() override;
    void Redo() override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    struct FlowParams
    {
        DAVA::FilePath spritePath;
        bool enableFlow = false;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowSpeed;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowOffset;
    };

    void ApplyParams(FlowParams& params);

    FlowParams newParams;
    FlowParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};

#endif //__PARTICLE_LAYER_COMMANDS_H__