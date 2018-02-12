#pragma once

#include "Render/RHI/rhi_Type.h"
#include "Base/RefPtr.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/ReflectionProbe.h"
#include "Render/Highlevel/QuadRenderer.h"

/*
    TODO:
    1. Local static probes with correction and serialisation to disk or pregeneration.
    2. Dynamic updates of probes, according to the cycle of their updates.
    3. Priority of visible probes.
 */

namespace DAVA
{
class ReflectionProbe;
class ReflectionRenderer
{
public:
    ReflectionRenderer(RenderSystem* _renderSystem);
    ~ReflectionRenderer();

    void RegisterReflectionProbe(ReflectionProbe* reflectionProbe);
    void UnregisterReflectionProbe(ReflectionProbe* reflectionProbe);

    void UpdateProbe(ReflectionProbe* probe);

    void Draw(Camera* camera);
    void SetDebugDrawEnabled(bool debugDraw);
    void InvalidateMaterials();

    void UpdateGlobalLightProbe();

    // void SaveCubemap(const FilePath & filePath, Texture * cubemap, uint32 mipmapLevelCount);

    Texture* GetSpecularConvolution2();

private:
    const uint32 MAX_NUMBER_OF_PROBES_TO_UPDATE_PER_FRAME = 1;

    void EnumerateMaterials(Set<NMaterial*>& materials);
    Texture* CreateCubeTextureForReflection(uint32 size, uint32 mipCount, PixelFormat format = PixelFormat::FORMAT_RGBA16F);

    ReflectionProbe* globalReflectionProbe = nullptr;
    RenderSystem* renderSystem = nullptr;
    RenderPass* reflectionPass = nullptr;

    Vector<ReflectionProbe*> reflectionProbeArray;

    Texture* reflectionMainFBO = nullptr;
    Texture* reflectionTemporaryFBO = nullptr; // temporary convolution textures
    Texture* globalProbeSpecularConvolution = nullptr;

    rhi::HTexture sphericalHarmonicsTexture = rhi::HTexture();
    Vector4 globalDiffuseSphericalHarmonics[9]{};
    int32 sphericalHarmonicsGrabCountdown = -1;

    Vector<Texture*> reflectionTextureCache;

    static const uint32 CUBEMAP_QUALITY_LEVELS = 4;
    static const uint32 maxCacheCubemapOnEachLevel[CUBEMAP_QUALITY_LEVELS]; // = {4, 8, 16, 32};
    static const uint32 cacheCubemapFaceSize[CUBEMAP_QUALITY_LEVELS]; // = {256, 128, 64, 32};
    uint32 cacheCubemapOffset[CUBEMAP_QUALITY_LEVELS];
    List<Texture*> textureCache[CUBEMAP_QUALITY_LEVELS];
    Vector<Texture*> allCacheTextures;

    void AllocateTextureFromCacheAndRender(ReflectionProbe* probe);
    void ReleaseTextureToCache(ReflectionProbe* probe);

    void PrerenderReflections(ReflectionProbe* probe, uint32 layerFilter);
    void GenerateReflectionProbe(ReflectionProbe* probe, Texture* reflectionTextureTarget);
    void GenerateDiffuseProbe(ReflectionProbe* probe);

    void UpdateProbeMaterialBindings(ReflectionProbe* probe);

    ScopedPtr<NMaterial> debugMaterial;
    QuadRenderer quadRenderer;
    bool debugDrawEnabled = false;

    struct ReflectionProbeToUpdate
    {
        ReflectionProbeToUpdate() = default;
        ReflectionProbeToUpdate(ReflectionProbe* reflectionProbe_, uint32 priority_)
            : reflectionProbe(reflectionProbe_)
            , priority(priority_)
        {
        }
        bool operator==(const ReflectionProbeToUpdate& r) const
        {
            return reflectionProbe == r.reflectionProbe;
        }

        bool operator<(const ReflectionProbeToUpdate& r) const
        {
            return priority < r.priority;
        }

        ReflectionProbe* reflectionProbe = nullptr;
        uint32 priority = 0;
    };

    std::set<ReflectionProbeToUpdate> probesForUpdate;
    uint64 invalidateCallback = 0;
};

inline void ReflectionRenderer::SetDebugDrawEnabled(bool debugDraw)
{
    debugDrawEnabled = debugDraw;
}
}
