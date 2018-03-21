#pragma once

#include "Render/RHI/rhi_Type.h"
#include "Base/RefPtr.h"
#include "Render/Material/NMaterial.h"
#include "Render/Highlevel/QuadRenderer.h"
#include "Functional/Signal.h"

namespace DAVA
{
class NMaterial;
class Window;
class PostEffectRenderer;
class HelperRenderer;

struct RhiTexture
{
    rhi::HTexture handle;
    Size2i size;
};

class PostEffectRenderer
{
public:
    enum class CombineMode : uint32
    {
        Separate,
        InplaceForward,
        InplaceDeferred
    };

private:
    enum : uint32
    {
        DebugPickerPoolSize = 3,
        DebugPickerSize = 4,
    };

public:
    PostEffectRenderer();
    ~PostEffectRenderer();

    void SetFrameContext(rhi::HTexture hdrImage, rhi::HTexture destination, const rhi::Viewport& vp);
    void Render(rhi::Handle destination, const rhi::Viewport& viewport);
    void InvalidateMaterials();
    void OnWindowSizeChanged(Window* w, Size2f windowSize, Size2f surfaceSize);

    rhi::HTexture GetHDRTarget();
    const Size2i& GetHDRTargetSize() const;

    /*
     * Reset luminance history to avoid flickering after significant camera changes (mostly position).
     */
    void ResetLuminanceHistory();

    struct DebugRenderer
    {
        bool drawHDRTarget = false;
        bool drawLuminance = false;
        bool drawAdaptation = false;
        bool drawHistogram = false;
        bool drawBloom = false;
        bool enableLightMeter = false;
        bool drawLightMeterMask = false;
        int32 debugRectSize = 128;
        Size2i debugRectOffset = { 5, 5 };
        Vector2 pointOfInterest = { 0, 0 };
        Vector4 debugPickerReadback[DebugPickerSize * DebugPickerSize];
        Vector4 debugPickedLuminance;
    };

    struct Settings
    {
        Asset<Texture> colorGradingTable;
        Asset<Texture> heatmapTable;
        Asset<Texture> lightMeterTable;

        float32 lightMeterTableWeight = 1.0f;
        Vector2 adaptationRange = Vector2{ 128.0, 2048.0f }; // +/- 2EV, exp2(12 - 3 - 2) .. exp2(12 - 3 + 2)
        Vector2 adaptationSpeed = Vector2{ 2.0f, 2.0f };

        Color bloomColor = Color::White;
        float32 bloomEVC = 0.0f; //bloom EV compensation

        bool enableColorGrading = false;
        bool enableToneMapping = false;
        bool enableHeatMap = false;
        bool enableTXAA = false;
        bool resetHistory = false;
    };

    DebugRenderer* GetDebugRenderer();
    Settings* GetSettings();

    void Combine(CombineMode mode, rhi::HPacketList pl = rhi::HPacketList());
    void DownsampleLuminance(rhi::HTexture srcTexture, const Size2i& srcTextureSize, int32 deltaPriority = 0);
    void DownsampleLuminanceInplace(rhi::HTexture srcTexture, const Size2i& srcTextureSize, int32 deltaPriority = 0);
    void Debug();

private:
    enum MaterialType : uint32
    {
        COMBINE,
        DEBUG,
        DEBUG_R16F,
        INIT_LUMINANCE,
        LUMINANCE,
        FINISH_LUMINANCE,
        LUMINANCE_HISTORY,
        COPY,
        RESET_LUMINANCE_HISTORY,
        BLOOM_THRESHOLD,
        GAUSS_BLUR,
        MUL_ADD_BLUR,
        LUMINANCE_COMBINED,

        MATERIALS_COUNT
    };

    Array<NMaterial*, MaterialType::MATERIALS_COUNT> materials;
    QuadRenderer quadRenderer;

    void BuildDebugPickerLuminance();
    void GetAverageLuminance();
    void GetHistogram();
    void Bloom();

    //bloom internal
    void Blur(RhiTexture* src, RhiTexture* dst, RhiTexture* tmp);
    void MulAddBlur(RhiTexture* src1, RhiTexture* src2, float32 mul1, float32 mul2, RhiTexture* dst);

    DebugRenderer debugRenderer;

    void InitResources(const Size2i& newSize);
    void DestroyResources(bool includeShared);
    Size2i hdrTargetSize;
    Size2i lastWindowSize;

    struct FrameContext
    {
        rhi::Handle hdrSource;
        rhi::Handle destination;
        rhi::Viewport viewport;
    } frameContext;

    struct HDRRenderer
    {
        rhi::HTexture hdrTarget;
        rhi::HTexture hdrPicker[DebugPickerPoolSize];
        uint32 debugPickerIndex = 0;
    };
    HDRRenderer hdrRenderer;

    struct AvgLogLumRenderer
    {
        Vector<rhi::HTexture> averageColorArray;
        Vector<float32> averageColorSize;
        rhi::HTexture luminanceHistory;
        rhi::HTexture luminancePrevious;
        rhi::HTexture luminanceTexture;
    };

    AvgLogLumRenderer allRenderer;
    void DumpAverageTexture(int32 index);

    //helper renderers
    enum RendererType
    {
        HISTOGRAM,
        BLOOM,
        TXAA,

        RENDERER_COUNT
    };
    Array<HelperRenderer*, RendererType::RENDERER_COUNT> renderers;

    Settings settings;
    rhi::HSamplerState linearSamplerState;
    bool sharedResourcesCreated = false;
};

inline rhi::HTexture PostEffectRenderer::GetHDRTarget()
{
    return hdrRenderer.hdrTarget;
}

inline const Size2i& PostEffectRenderer::GetHDRTargetSize() const
{
    return hdrTargetSize;
}

inline void PostEffectRenderer::ResetLuminanceHistory()
{
    settings.resetHistory = true;
}

inline PostEffectRenderer::DebugRenderer* PostEffectRenderer::GetDebugRenderer()
{
    return &debugRenderer;
}

inline PostEffectRenderer::Settings* PostEffectRenderer::GetSettings()
{
    return &settings;
}
}
