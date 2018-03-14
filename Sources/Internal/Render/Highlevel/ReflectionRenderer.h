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
    enum : uint32
    {
        CUBEMAP_QUALITY_LEVELS = 4
    };

public:
    ReflectionRenderer(RenderSystem* _renderSystem);
    ~ReflectionRenderer();

    void RegisterReflectionProbe(ReflectionProbe* reflectionProbe);
    void UnregisterReflectionProbe(ReflectionProbe* reflectionProbe);

    void UpdateProbe(ReflectionProbe* probe);

    void Draw(Camera* camera);

    void InvalidateMaterials();

    void SetDebugDrawProbe(ReflectionProbe* probe);

    Texture* GetSpecularConvolution2();

private:
    struct SphericalHarmonicsUpdate
    {
        rhi::HTexture targetTexture;
        ReflectionProbe* probe = nullptr;
        uint32 countdown = 0;
    };

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

    SphericalHarmonicsUpdate EnqueueSphericalHarmonicsUpdate(ReflectionProbe* probe);
    void RetrieveUpdatedSphericalHarmonics();

    void AllocateProbeTexture(ReflectionProbe* probe);
    void ReleaseProbeTexture(ReflectionProbe* probe);

    void UpdateProbeMaterialBindings(ReflectionProbe* probe);

    void EnumerateMaterials(Set<NMaterial*>& materials);
    Texture* CreateCubeTextureForReflection(uint32 size, uint32 mipCount, PixelFormat format);

    void RenderReflectionProbe(ReflectionProbe* probe);
    void DrawDebugInfo();

private:
    RenderSystem* renderSystem = nullptr;
    RenderPass* reflectionPass = nullptr;

    ReflectionProbe* globalReflectionProbe = nullptr;
    ReflectionProbe* debugDrawProbe = nullptr;
    Vector<ReflectionProbe*> localReflectionProbes;
    Vector<ReflectionProbeToUpdate> probesForUpdate;
    Texture* temporaryFramebuffer = nullptr;
    Texture* downsampledFramebuffer = nullptr; // temporary convolution textures
    Texture* globalProbeSpecularConvolution = nullptr;

    Vector<SphericalHarmonicsUpdate> shUpdateQueue;
    Vector<Texture*> reflectionTextureCache;
    List<Texture*> textureCache[CUBEMAP_QUALITY_LEVELS];
    Vector<Texture*> allCacheTextures;
    ScopedPtr<NMaterial> debugMaterial;
    QuadRenderer quadRenderer;
    Vector4 emptySphericalHarmonicsArray[9]{};
    uint64 invalidateCallback = 0;
};
}
