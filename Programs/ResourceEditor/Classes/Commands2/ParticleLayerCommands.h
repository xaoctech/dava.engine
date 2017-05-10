#pragma once

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
    struct FlowParams
    {
        DAVA::FilePath spritePath;
        bool enableFlow = false;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowSpeedOverLife;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowOffsetOverLife;
    };

    CommandChangeFlowProperties(DAVA::ParticleLayer* layer_, FlowParams&& flowParameters);

    void Undo() override;
    void Redo() override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    void ApplyParams(FlowParams& params);

    FlowParams newParams;
    FlowParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};

class CommandChangeNoiseProperties : public RECommand
{
public:
    CommandChangeNoiseProperties(DAVA::ParticleLayer* layer, const DAVA::FilePath& noisePath, bool enableNoise, bool isNoiseAffectFlow, RefPtr<PropertyLine<float32>> noiseScale, bool useNoiseScroll, RefPtr<PropertyLine<float32>> noiseUScrollSpeed, RefPtr<PropertyLine<float32>> noiseVScrollSpeed);

    void Undo() override;
    void Redo() override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    struct NoiseParams
    {
        DAVA::FilePath noisePath;
        bool enableNoise = false;
        bool isNoiseAffectFlow = false;
        RefPtr<PropertyLine<float32>> noiseScale;
        bool useNoiseScroll = false; 
        RefPtr<PropertyLine<float32>> noiseUScrollSpeed;
        RefPtr<PropertyLine<float32>> noiseVScrollSpeed;
    };

    void ApplyParams(NoiseParams& params);

    NoiseParams newParams;
    NoiseParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};
