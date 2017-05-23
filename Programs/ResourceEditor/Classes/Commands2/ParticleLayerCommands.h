#pragma once

#include "Commands2/Base/RECommand.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "Base/RefPtr.h"
#include "Particles/ParticlePropertyLine.h"

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
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowSpeed;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowSpeedVariation;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowOffset;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> flowOffsetVariation;
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
    struct NoiseParams
    {
        DAVA::FilePath noisePath;
        bool enableNoise = false;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseScale;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseScaleVariation;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseScaleOverLife;
        bool enableNoiseScroll = false;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseUScrollSpeed;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseUScrollSpeedVariation;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseUScrollSpeedOverLife;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseVScrollSpeed;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseVScrollSpeedVariation;
        DAVA::RefPtr<DAVA::PropertyLine<DAVA::float32>> noiseVScrollSpeedOverLife;
    };

    CommandChangeNoiseProperties(DAVA::ParticleLayer* layer, NoiseParams&& params);

    void Undo() override;
    void Redo() override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    void ApplyParams(NoiseParams& params);

    NoiseParams newParams;
    NoiseParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};

class CommandChangeFresnelToAlphaProperties : public RECommand
{
public:
    struct FresnelToAlphaParams
    {
        bool useFresnelToAlpha = false;
        DAVA::float32 fresnelToAlphaBias = 0.0f;
        DAVA::float32 fresnelToAlphaPower = 0.0f;
    };

    CommandChangeFresnelToAlphaProperties(DAVA::ParticleLayer* layer, FresnelToAlphaParams&& params);

    void Undo() override;
    void Redo() override;

    DAVA::ParticleLayer* GetLayer() const;

private:
    void ApplyParams(FresnelToAlphaParams& params);

    FresnelToAlphaParams newParams;
    FresnelToAlphaParams oldParams;
    DAVA::ParticleLayer* layer = nullptr;
};