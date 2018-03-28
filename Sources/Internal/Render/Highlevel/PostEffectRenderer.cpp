#include "Render/Highlevel/PostEffectRenderer.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"
#include "Render/RhiUtils.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Engine/Engine.h"
#include "Engine/Window.h"
#include "Math/MathHelpers.h"
#include "Time/SystemTimer.h"
#include "Debug/ProfilerGPU.h"
#include "Utils/StringFormat.h"
#include "Math/Gaussian.h"
#include "Logger/Logger.h"

namespace DAVA
{
class HelperRenderer
{
public:
    virtual ~HelperRenderer(){};

    virtual void InitResources(const Size2i& newSize, const PostEffectRenderer::Settings& settings){};
    virtual void DestroyResources(){};
    virtual void InvalidateMaterials(){};
};

class HistogramRenderer : public HelperRenderer
{
public:
    void InitResources(const Size2i& newSize, const PostEffectRenderer::Settings& settings) override
    {
        struct IntPos
        {
            int16 x, y, z;
        };

        int32 verticesCount = newSize.dx * newSize.dy;
        Vector<IntPos> vertices;
        vertices.resize(verticesCount);

        for (int32 i = 0; i < newSize.dx; ++i)
        {
            for (int32 j = 0; j < newSize.dy; ++j)
            {
                IntPos* p = &vertices[j * newSize.dx + i];
                p->x = i;
                p->y = j;
                p->z = 1;
            }
        }

        rhi::VertexBuffer::Descriptor vDesc;
        vDesc.size = sizeof(IntPos) * verticesCount;
        vDesc.initialData = vertices.data();
        vDesc.usage = rhi::USAGE_STATICDRAW;
        vBuffer = rhi::CreateVertexBuffer(vDesc);

        rhi::VertexLayout vxLayout;
        vxLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_INT16, 3);
        packet.vertexLayoutUID = rhi::VertexLayout::UniqueId(vxLayout);

        packet.vertexStreamCount = 1;
        packet.vertexStream[0] = vBuffer;
        packet.primitiveType = rhi::PRIMITIVE_POINTLIST;
        packet.primitiveCount = verticesCount;

        rhi::SamplerState::Descriptor samplerDescriptor;
        samplerDescriptor.vertexSamplerCount = 1;
        samplerDescriptor.vertexSampler[0].addrU = rhi::TEXADDR_CLAMP;
        samplerDescriptor.vertexSampler[0].addrV = rhi::TEXADDR_CLAMP;
        samplerDescriptor.vertexSampler[0].minFilter = rhi::TEXFILTER_NEAREST;
        samplerDescriptor.vertexSampler[0].magFilter = rhi::TEXFILTER_NEAREST;
        samplerDescriptor.vertexSampler[0].mipFilter = rhi::TEXMIPFILTER_NEAREST;
        rhi::HSamplerState samplerState = rhi::AcquireSamplerState(samplerDescriptor);
        packet.samplerState = samplerState;

        rhi::Texture::Descriptor descriptor;
        descriptor.width = bucketsCount;
        descriptor.height = 1;
        descriptor.autoGenMipmaps = false;
        descriptor.isRenderTarget = true;
        descriptor.needRestore = false;
        descriptor.type = rhi::TEXTURE_TYPE_2D;
        descriptor.format = rhi::TEXTURE_FORMAT_R32F;
        descriptor.cpuAccessRead = false;
        descriptor.cpuAccessWrite = false;
        bucketTexture = rhi::CreateTexture(descriptor);

        passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_NONE;
        passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
        passConfig.colorBuffer[0].clearColor[0] = 0.0f;
        passConfig.colorBuffer[0].clearColor[1] = 0.0f;
        passConfig.colorBuffer[0].clearColor[2] = 1.0f;
        passConfig.colorBuffer[0].clearColor[3] = 1.0f;
        passConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_NONE;
        passConfig.depthStencilBuffer.storeAction = rhi::STOREACTION_NONE;
        passConfig.viewport = rhi::Viewport(0, 0, bucketsCount, 1);
        passConfig.colorBuffer[0].texture = bucketTexture;
        passConfig.priority = PRIORITY_MAIN_3D - 100;

        material = new NMaterial();
        material->SetFXName(FastName("~res:/Materials2/Histogram.material"));
        Vector2 defaultTexSize{ 2048.f, 2048.f };
        Vector2 defaultTexOffset{ 0, 0 };
        material->AddProperty(FastName("srcTexSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        float32 bucketsCount = 16.f;
        material->AddProperty(FastName("bucketsCount"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT1);
    }

    void DestroyResources() override
    {
        SafeRelease(material);
        rhi::DeleteVertexBuffer(vBuffer);
        rhi::DeleteTexture(bucketTexture);
    }

    void Render(rhi::HTexture source)
    {
        DAVA_PROFILER_GPU_RENDER_PASS(passConfig, "HistogramRenderer");
        renderPass = rhi::AllocateRenderPass(passConfig, 1, &packetList);
        if (renderPass != rhi::InvalidHandle)
        {
            rhi::BeginRenderPass(renderPass);
            rhi::BeginPacketList(packetList);

            Vector2 srcTexSize(2048.f, 2048.f);
            material->SetPropertyValue(FastName("srcTexSize"), srcTexSize.data);
            float32 floatBucketsCount = static_cast<float32>(bucketsCount);
            material->SetPropertyValue(FastName("bucketsCount"), &floatBucketsCount);
            material->BindParams(packet);

            packet.textureSet = RhiUtils::VertexTextureSet{ source };

            rhi::AddPacket(packetList, packet);

            rhi::EndPacketList(packetList);
            rhi::EndRenderPass(renderPass);
        }
    }

    void InvalidateMaterials() override
    {
        material->InvalidateRenderVariants();
    }

    rhi::HVertexBuffer vBuffer;
    rhi::Packet packet;
    rhi::RenderPassConfig passConfig;
    rhi::HRenderPass renderPass;
    rhi::HPacketList packetList;

    static const uint32 bucketsCount = 64;
    rhi::HTexture bucketTexture;

    NMaterial* material = nullptr;
};

class BloomRenderer : public HelperRenderer
{
public:
    void InitResources(const Size2i& newSize, const PostEffectRenderer::Settings& settings) override
    {
        Size2i currentSize;
        currentSize.dx = newSize.dx;
        currentSize.dy = newSize.dy;
        for (int32 i = 0; i < BLUR_CHAIN_COUNT; ++i)
        {
            currentSize.dx = Max(1, currentSize.dx / 2);
            currentSize.dy = Max(1, currentSize.dy / 2);

            rhi::Texture::Descriptor descriptor;
            descriptor.width = currentSize.dx;
            descriptor.height = currentSize.dy;
            descriptor.autoGenMipmaps = false;
            descriptor.isRenderTarget = true;
            descriptor.needRestore = false;
            descriptor.type = rhi::TEXTURE_TYPE_2D;
            descriptor.format = rhi::TEXTURE_FORMAT_R16F;
            descriptor.cpuAccessRead = false;
            descriptor.cpuAccessWrite = false;
            blurChain[i].orig.handle = rhi::CreateTexture(descriptor);
            blurChain[i].blur.handle = rhi::CreateTexture(descriptor);
            blurChain[i].orig.size = currentSize;
            blurChain[i].blur.size = currentSize;
        }
    }

    void DestroyResources() override
    {
        for (BlurPair& pair : blurChain)
        {
            rhi::DeleteTexture(pair.orig.handle);
            rhi::DeleteTexture(pair.blur.handle);
        }
    }

    void InvalidateMaterials() override
    {
    }

    static const int32 BLUR_CHAIN_COUNT = 5;
    struct BlurPair
    {
        RhiTexture orig;
        RhiTexture blur;
    };
    Array<BlurPair, BLUR_CHAIN_COUNT> blurChain;
    NMaterial* material = nullptr;
};

PostEffectRenderer::PostEffectRenderer()
{
    for (HelperRenderer*& r : renderers)
        r = nullptr;

    // renderers[RendererType::HISTOGRAM] = new HistogramRenderer();
    // renderers[RendererType::BLOOM] = new BloomRenderer;

    Array<FastName, MaterialType::MATERIALS_COUNT> materialFlags
    {
      FastName("TECH_COMBINE"),
      FastName("TECH_DEBUG"),
      FastName("TECH_DEBUG_R16F"),
      FastName("TECH_INIT_LUMINANCE"),
      FastName("TECH_LUMINANCE"),
      FastName("TECH_FINISH_LUMINANCE"),
      FastName("TECH_LUMINANCE_HISTORY"),
      FastName("TECH_COPY"),
      FastName("TECH_RESET_LUMINANCE_HISTORY"),
      FastName("TECH_BLOOM_THRESHOLD"),
      FastName("TECH_GAUSS_BLUR"),
      FastName("TECH_MUL_ADD_BLUR"),
      FastName("TECH_COMBINED_LUMINANCE"),
    };

    for (int32 i = 0; i < MaterialType::MATERIALS_COUNT; ++i)
    {
        materials[i] = new NMaterial();
        materials[i]->SetFXName(FastName("~res:/Materials2/PostEffect.material"));
        materials[i]->AddFlag(materialFlags[i], 1);

        Vector2 defaultTexSize{ 2048.f, 2048.f };
        Vector2 defaultTexOffset{ 0, 0 };
        materials[i]->AddProperty(FastName("srcRectOffset"), defaultTexOffset.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        materials[i]->AddProperty(FastName("srcRectSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        materials[i]->AddProperty(FastName("srcTexSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);

        materials[i]->AddProperty(FastName("destRectOffset"), Vector2::Zero.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        materials[i]->AddProperty(FastName("destRectSize"), Vector2::Zero.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        materials[i]->AddProperty(FastName("destTexSize"), Vector2::Zero.data, rhi::ShaderProp::Type::TYPE_FLOAT2);

        float32 float1{ 1.f };
        Vector2 float2{ 0.f, 1.f };
        Vector3 float3{ 0.f, 1.f, 2.f };
        materials[i]->AddFlag(FastName("ENABLE_TXAA"), 0);
        if (i == MaterialType::COMBINE)
        {
            materials[i]->AddProperty(FastName("lightMeterMaskWeight"), &float1, rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddFlag(FastName("COMBINE_INPLACE"), 0);
            materials[i]->AddFlag(FastName("ENABLE_COLOR_GRADING"), 0);
            materials[i]->AddFlag(FastName("DISPLAY_HEAT_MAP"), 0);
            materials[i]->AddFlag(FastName("DISPLAY_LIGHT_METER_MASK"), 0);
            materials[i]->AddProperty(FastName("texelOffset"), float2.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        }
        else if ((i == MaterialType::LUMINANCE_HISTORY) || (i == MaterialType::LUMINANCE_COMBINED))
        {
            materials[i]->AddProperty(FastName("frameTime"), &float1, rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("adaptationRange"), float2.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
            materials[i]->AddProperty(FastName("adaptationSpeed"), float2.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        }
        else if (i == MaterialType::FINISH_LUMINANCE)
        {
            materials[i]->AddProperty(FastName("lightMeterMaskWeight"), &float1, rhi::ShaderProp::Type::TYPE_FLOAT1);
        }
        else if (i == MaterialType::BLOOM_THRESHOLD)
        {
            materials[i]->AddProperty(FastName("EV"), &float1, rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("bloomEVC"), &float1, rhi::ShaderProp::Type::TYPE_FLOAT1);
        }
        else if (i == MaterialType::GAUSS_BLUR)
        {
            static const int32 weightsCount = 7; //must be synchronized with shader source, 7 raw -> 4 optimized

            Gaussian::Values values = Gaussian::CalculateOptimizedWeights(weightsCount, .33f);
            materials[i]->AddProperty(FastName("gaussWeight0"), &values.weights[0], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussWeight1"), &values.weights[1], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussWeight2"), &values.weights[2], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussWeight3"), &values.weights[3], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussOffset0"), &values.offsets[0], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussOffset1"), &values.offsets[1], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussOffset2"), &values.offsets[2], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("gaussOffset3"), &values.offsets[3], rhi::ShaderProp::Type::TYPE_FLOAT1);
            materials[i]->AddProperty(FastName("texelOffset"), float2.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        }
        else if (i == MaterialType::MUL_ADD_BLUR)
        {
            materials[i]->AddProperty(FastName("mulAdd"), float2.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
        }
        materials[i]->PreBuildMaterial(PASS_FORWARD);
    }

    if (nullptr != Engine::Instance()->PrimaryWindow())
    {
        Size2f initialSize = Engine::Instance()->PrimaryWindow()->GetSurfaceSize();
        OnWindowSizeChanged(nullptr, initialSize, initialSize);
        Engine::Instance()->PrimaryWindow()->sizeChanged.Connect(this, &PostEffectRenderer::OnWindowSizeChanged);
    }

    settings.colorGradingTable = Texture::CreateFromFile("~res:/Textures/colorgrading.png");
    settings.heatmapTable = Texture::CreateFromFile("~res:/Textures/heatmap.png");
    settings.lightMeterTable = Texture::CreateFromFile("~res:/Textures/lightmeter.png");

    rhi::SamplerState::Descriptor linearDesc;
    for (uint32 i = 0; i < rhi::MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT; ++i)
    {
        linearDesc.fragmentSampler[i].addrU = rhi::TEXADDR_CLAMP;
        linearDesc.fragmentSampler[i].addrV = rhi::TEXADDR_CLAMP;
        linearDesc.fragmentSampler[i].magFilter = rhi::TEXFILTER_LINEAR;
        linearDesc.fragmentSampler[i].minFilter = rhi::TEXFILTER_LINEAR;
        linearDesc.fragmentSampler[i].mipFilter = rhi::TEXMIPFILTER_NONE;
        linearDesc.fragmentSampler[i].anisotropyLevel = 1;
    }
    linearDesc.fragmentSamplerCount = rhi::MAX_FRAGMENT_TEXTURE_SAMPLER_COUNT;
    for (uint32 i = 0; i < rhi::MAX_VERTEX_TEXTURE_SAMPLER_COUNT; ++i)
    {
        linearDesc.vertexSampler[i].addrU = rhi::TEXADDR_CLAMP;
        linearDesc.vertexSampler[i].addrV = rhi::TEXADDR_CLAMP;
        linearDesc.vertexSampler[i].magFilter = rhi::TEXFILTER_LINEAR;
        linearDesc.vertexSampler[i].minFilter = rhi::TEXFILTER_LINEAR;
        linearDesc.vertexSampler[i].mipFilter = rhi::TEXMIPFILTER_NONE;
        linearDesc.vertexSampler[i].anisotropyLevel = 1;
    }
    linearDesc.vertexSamplerCount = rhi::MAX_VERTEX_TEXTURE_SAMPLER_COUNT;
    linearSamplerState = rhi::AcquireSamplerState(linearDesc);
}

PostEffectRenderer::~PostEffectRenderer()
{
    DestroyResources(true);

    for (HelperRenderer* r : renderers)
        SafeDelete(r);

    if (nullptr != Engine::Instance()->PrimaryWindow())
    {
        Engine::Instance()->PrimaryWindow()->sizeChanged.Disconnect(this);
    }
}

void PostEffectRenderer::InvalidateMaterials()
{
    for (NMaterial* material : materials)
        material->InvalidateRenderVariants();

    for (HelperRenderer* r : renderers)
    {
        if (r)
        {
            r->InvalidateMaterials();
        }
    }
}

void PostEffectRenderer::OnWindowSizeChanged(Window* w, Size2f windowSize, Size2f surfaceSize)
{
    int32 surfaceWidth = static_cast<int32>(surfaceSize.dx);
    int32 surfaceHeight = static_cast<int32>(surfaceSize.dy);
    InitResources(Size2i(surfaceWidth, surfaceHeight));
}

void PostEffectRenderer::SetFrameContext(rhi::HTexture hdrImage, rhi::HTexture destination, const rhi::Viewport& vp)
{
    frameContext = { hdrImage, destination, vp };
}

void PostEffectRenderer::Render(rhi::Handle destination, const rhi::Viewport& viewport)
{
    SetFrameContext(hdrRenderer.hdrTarget, rhi::HTexture(destination), viewport);
    GetAverageLuminance();
    Combine(CombineMode::Separate);
    Debug();

    // TODO : enable bloom again some day
}

void PostEffectRenderer::Render(rhi::Handle hdrImage, rhi::Handle destination, const rhi::Viewport& viewport)
{
    SetFrameContext(rhi::HTexture(hdrImage), rhi::HTexture(destination), viewport);
    GetAverageLuminance();
    Combine(CombineMode::Separate);
    Debug();
}

void PostEffectRenderer::GetAverageLuminance()
{
    static const char* renderPassName = "avgLuminance";

    // init luminance
    QuadRenderer::Options options;
    options.loadAction = rhi::LoadAction::LOADACTION_NONE;
    options.material = materials[MaterialType::INIT_LUMINANCE];
    options.srcRect = Rect2f(0.f, 0.f, float32(frameContext.viewport.width), float32(frameContext.viewport.height));
    options.srcTexSize = Vector2(float32(hdrTargetSize.dx), float32(hdrTargetSize.dy));
    options.dstTexture = allRenderer.averageColorArray[0];
    options.dstRect = Rect2f(0.f, 0.f, allRenderer.averageColorSize[0], allRenderer.averageColorSize[0]);
    options.dstTexSize = Vector2(allRenderer.averageColorSize[0], allRenderer.averageColorSize[0]);
    options.renderPassName = "lum_init";
    options.textureSet = RhiUtils::FragmentTextureSet({ hdrRenderer.hdrTarget, settings.lightMeterTable->handle });
    quadRenderer.Render(options);

    Size2i baseSize(static_cast<int32>(allRenderer.averageColorSize.front()), static_cast<int32>(allRenderer.averageColorSize.front()));
    DownsampleLuminance(allRenderer.averageColorArray.front(), baseSize);
}

void PostEffectRenderer::DownsampleLuminanceInplace(rhi::HTexture srcTexture, const Size2i& srcSize, const Size2i& srcTextureSize, int32 deltaPriority)
{
    if (settings.resetHistory)
    {
        NMaterial* material = materials[MaterialType::RESET_LUMINANCE_HISTORY];

        QuadRenderer::Options options;
        options.renderPassName = "lum_reset";
        options.material = material;
        options.srcRect = Rect2f(0, 0, 1.f, 1.f);
        options.srcTexSize = Vector2(1.f, 1.f);
        options.dstTexture = allRenderer.luminanceTexture;
        options.dstRect = Rect2f(0.f, 0.f, 1.0, 1.f);
        options.dstTexSize = Vector2(1.0f, 1.0f);
        options.textureSet = RhiUtils::FragmentTextureSet{ allRenderer.averageColorArray.back() };
        options.renderPassPriority = 100;
        quadRenderer.Render(options);

        settings.resetHistory = false;
    }

    static char passNamesBuffer[4096] = {};
    memset(passNamesBuffer, 0, sizeof(passNamesBuffer));

    QuadRenderer::Options options;
    options.renderPassPriority += deltaPriority;
    int printPosition = 0;
    for (size_t i = 1; i < allRenderer.averageColorArray.size(); ++i)
    {
        int charactersPrinter = sprintf(passNamesBuffer + printPosition, "lum_downscale_%d", static_cast<int>(i));
        options.renderPassName = passNamesBuffer + printPosition;
        printPosition += charactersPrinter + 1;

        if (i == 1)
        {
            options.srcTexture = srcTexture;
            options.srcRect = Rect2f(0.f, 0.f, float32(srcSize.dx), float32(srcSize.dy));
            options.srcTexSize = Vector2(float32(srcTextureSize.dx), float32(srcTextureSize.dy));
        }
        else
        {
            options.srcTexture = allRenderer.averageColorArray[i - 1];
            options.srcRect = Rect2f(0.f, 0.f, allRenderer.averageColorSize[i - 1], allRenderer.averageColorSize[i - 1]);
            options.srcTexSize = Vector2(allRenderer.averageColorSize[i - 1], allRenderer.averageColorSize[i - 1]);
        }

        if (i + 1 == allRenderer.averageColorArray.size())
        {
            float32 frameDelta = SystemTimer::GetFrameDelta();

            options.material = materials[MaterialType::LUMINANCE_COMBINED];
            options.material->SetPropertyValue(FastName("adaptationRange"), settings.adaptationRange.data);
            options.material->SetPropertyValue(FastName("adaptationSpeed"), settings.adaptationSpeed.data);
            options.material->SetPropertyValue(FastName("frameTime"), &frameDelta);

            options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
            options.dstTexture = allRenderer.luminanceTexture;
            options.dstRect = Rect2f(0.f, 0.f, 1.0f, 1.0f);
            options.dstTexSize = Vector2(1.0f, 1.0f);

            rhi::HTexture previousTexture = allRenderer.averageColorArray[i - 1];
            options.textureSet = RhiUtils::FragmentTextureSet{ previousTexture };
        }
        else
        {
            options.material = materials[MaterialType::LUMINANCE];
            options.loadAction = rhi::LoadAction::LOADACTION_NONE;
            options.dstTexture = allRenderer.averageColorArray[i];
            options.dstRect = Rect2f(0.f, 0.f, allRenderer.averageColorSize[i], allRenderer.averageColorSize[i]);
            options.dstTexSize = Vector2(allRenderer.averageColorSize[i], allRenderer.averageColorSize[i]);
        }

        quadRenderer.Render(options);
    }
}

void PostEffectRenderer::DownsampleLuminance(rhi::HTexture srcTexture, const Size2i& srcTextureSize, int32 deltaPriority)
{
    if (settings.resetHistory)
    {
        NMaterial* material = materials[MaterialType::RESET_LUMINANCE_HISTORY];

        QuadRenderer::Options options;
        options.renderPassName = "lum_reset";
        options.material = material;
        options.srcRect = Rect2f(0, 0, 1.f, 1.f);
        options.srcTexSize = Vector2(1.f, 1.f);
        options.dstTexture = allRenderer.luminanceHistory;
        options.dstRect = Rect2f(0.f, 0.f, 1.0f, 1.f);
        options.dstTexSize = Vector2(1.0f, 1.f);
        options.textureSet = RhiUtils::FragmentTextureSet{ allRenderer.averageColorArray.back() };
        options.renderPassPriority = 100;
        quadRenderer.Render(options);

        settings.resetHistory = false;
    }

    // downscale luminance
    static char passNamesBuffer[4096] = {};
    memset(passNamesBuffer, 0, sizeof(passNamesBuffer));
    int printPosition = 0;

    for (size_t i = 1; i < allRenderer.averageColorArray.size(); ++i)
    {
        int charsPrinted = sprintf(passNamesBuffer + printPosition, "lum_downscale_%d", static_cast<int>(i));

        QuadRenderer::Options options;
        options.renderPassName = passNamesBuffer + printPosition;
        options.loadAction = rhi::LoadAction::LOADACTION_NONE;
        options.material = materials[MaterialType::LUMINANCE];

        if (i + 1 == allRenderer.averageColorArray.size())
        {
            options.material = materials[MaterialType::FINISH_LUMINANCE];
            options.material->SetPropertyValue(FastName("lightMeterMaskWeight"), &settings.lightMeterTableWeight);
        }

        if (i == 1)
        {
            options.srcTexture = srcTexture;
            options.srcRect = Rect2f(0.f, 0.f, float32(srcTextureSize.dx), float32(srcTextureSize.dy));
            options.srcTexSize = Vector2(float32(srcTextureSize.dx), float32(srcTextureSize.dy));
        }
        else
        {
            options.srcTexture = allRenderer.averageColorArray[i - 1];
            options.srcRect = Rect2f(0.f, 0.f, allRenderer.averageColorSize[i - 1], allRenderer.averageColorSize[i - 1]);
            options.srcTexSize = Vector2(allRenderer.averageColorSize[i - 1], allRenderer.averageColorSize[i - 1]);
        }

        options.dstTexture = allRenderer.averageColorArray[i];
        options.dstRect = Rect2f(0.f, 0.f, allRenderer.averageColorSize[i], allRenderer.averageColorSize[i]);
        options.dstTexSize = Vector2(allRenderer.averageColorSize[i], allRenderer.averageColorSize[i]);
        options.renderPassPriority += deltaPriority;
        quadRenderer.Render(options);

        printPosition += charsPrinted + 1;
    }

    if (settings.resetHistory)
    {
        NMaterial* material = materials[MaterialType::RESET_LUMINANCE_HISTORY];

        QuadRenderer::Options options;
        options.renderPassName = "lum_reset";
        options.material = material;
        options.srcRect = Rect2f(0, 0, 1.f, 1.f);
        options.srcTexSize = Vector2(1.f, 1.f);
        options.dstTexture = allRenderer.luminanceHistory;
        options.dstRect = Rect2f(0.f, 0.f, 1.0f, 1.f);
        options.dstTexSize = Vector2(1.0f, 1.f);
        options.textureSet = RhiUtils::FragmentTextureSet{ allRenderer.averageColorArray.back() };
        options.renderPassPriority = 100;
        quadRenderer.Render(options);

        settings.resetHistory = false;
    }

    // copy current luminance
    {
        QuadRenderer::Options options;
        options.loadAction = rhi::LoadAction::LOADACTION_NONE;
        options.material = materials[MaterialType::COPY];
        options.srcTexture = allRenderer.luminanceHistory;
        options.srcRect = Rect2f(0.0f, 0.0f, 1.0f, 1.0f);
        options.srcTexSize = Vector2(1.0f, 1.f);
        options.dstTexture = allRenderer.luminancePrevious;
        options.dstRect = Rect2f(0.f, 0.f, 1.f, 1.f);
        options.dstTexSize = Vector2(1.f, 1.f);
        options.renderPassName = "lum_copy";
        options.renderPassPriority += deltaPriority;
        quadRenderer.Render(options);
    }

    // adopt
    {
        float32 frameDelta = SystemTimer::GetFrameDelta();

        NMaterial* material = materials[MaterialType::LUMINANCE_HISTORY];
        material->SetPropertyValue(FastName("adaptationRange"), settings.adaptationRange.data);
        material->SetPropertyValue(FastName("adaptationSpeed"), settings.adaptationSpeed.data);
        material->SetPropertyValue(FastName("frameTime"), &frameDelta);

        QuadRenderer::Options options;
        options.renderPassName = "lum_adopt";
        options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
        options.material = material;
        options.srcRect = Rect2f(0, 0, 1.f, 1.f);
        options.srcTexSize = Vector2(1.f, 1.f);
        options.dstTexture = allRenderer.luminanceHistory;
        options.dstRect = Rect2f(0.0f, 0.0f, 1.f, 1.f);
        options.dstTexSize = Vector2(1.0f, 1.f);
        options.textureSet = RhiUtils::FragmentTextureSet{ allRenderer.averageColorArray.back(), allRenderer.luminancePrevious };
        options.renderPassPriority += deltaPriority;
        quadRenderer.Render(options);
    }
}

void PostEffectRenderer::GetHistogram()
{
    HistogramRenderer* r = static_cast<HistogramRenderer*>(renderers[RendererType::HISTOGRAM]);
    if (r)
    {
        r->Render(hdrRenderer.hdrTarget);
    }
}

void PostEffectRenderer::Combine(CombineMode mode, rhi::HPacketList pl)
{
    bool txaaEnabled =
    ((Renderer::GetCurrentRenderFlow() == RenderFlow::HDRDeferred) || (Renderer::GetCurrentRenderFlow() == RenderFlow::TileBasedHDRDeferred)) &&
    (QualitySettingsSystem::Instance()->GetCurrentQualityValue<QualityGroup::Antialiasing>() == rhi::AntialiasingType::TEMPORAL_REPROJECTION);

    BloomRenderer* bloomRenderer = static_cast<BloomRenderer*>(renderers[RendererType::BLOOM]);
    rhi::HTexture bloomTexture = (bloomRenderer != nullptr) ? bloomRenderer->blurChain[0].blur.handle : rhi::HTexture();
    rhi::HTexture lumTexture = (mode == CombineMode::Separate) ? allRenderer.luminanceHistory : allRenderer.luminanceTexture;

    NMaterial* material = materials[MaterialType::COMBINE];
    material->SetPropertyValue(FastName("lightMeterMaskWeight"), &settings.lightMeterTableWeight);
    material->SetFlag(FastName("COMBINE_INPLACE"), static_cast<uint32>(mode));
    material->SetFlag(FastName("ENABLE_COLOR_GRADING"), settings.enableColorGrading ? 1 : 0);
    material->SetFlag(FastName("DISPLAY_HEAT_MAP"), settings.enableHeatMap ? 1 : 0);
    material->SetFlag(FastName("DISPLAY_LIGHT_METER_MASK"), debugRenderer.drawLightMeterMask ? 1 : 0);
    material->SetFlag(FastName("ENABLE_TXAA"), txaaEnabled);

    QuadRenderer::Options options;
    if (txaaEnabled)
    {
        Size2i sizei = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);
        Vector2 dstSize(static_cast<float32>(sizei.dx), static_cast<float32>(sizei.dy));
        Vector2 invSize(1.0f / dstSize.x, 1.0f / dstSize.y);
        material->SetPropertyValue(FastName("texelOffset"), invSize.data);

        options.dstTexSize = dstSize;
        options.srcRect = Rect2f(0.f, 0.f, dstSize.x, dstSize.y);

        options.srcRect = Rect2f(0.f, 0.f, float32(frameContext.viewport.width), float32(frameContext.viewport.height));
        options.dstRect = Rect2f(0.0f, 0.0f, dstSize.x, dstSize.y);
        options.dstRect = Rect2f(float32(frameContext.viewport.x), float32(frameContext.viewport.y), float32(frameContext.viewport.width), float32(frameContext.viewport.height));

        options.dstTexture = Renderer::GetDynamicBindings().GetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_CURRENT));
    }
    else
    {
        options.srcRect = Rect2f(0.f, 0.f, float32(frameContext.viewport.width), float32(frameContext.viewport.height));
        options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
        options.dstRect = Rect2f(float32(frameContext.viewport.x), float32(frameContext.viewport.y), float32(frameContext.viewport.width), float32(frameContext.viewport.height));
        options.dstTexture = frameContext.destination;
    }
    options.srcTexSize = Vector2(float32(hdrTargetSize.dx), float32(hdrTargetSize.dy));
    options.samplerState = linearSamplerState;

    if (mode != CombineMode::Separate)
    {
        if (material->HasLocalProperty(FastName("srcRectOffset")))
            material->SetPropertyValue(FastName("srcRectOffset"), options.srcRect.GetPosition().data);

        if (material->HasLocalProperty(FastName("srcRectSize")))
            material->SetPropertyValue(FastName("srcRectSize"), options.srcRect.GetSize().data);

        if (material->HasLocalProperty(FastName("srcTexSize")))
            material->SetPropertyValue(FastName("srcTexSize"), options.srcTexSize.data);

        if (material->HasLocalProperty(FastName("destTexSize")))
            material->SetPropertyValue(FastName("destTexSize"), options.dstTexSize.data);

        if (material->PreBuildMaterial(PASS_FORWARD))
        {
            uint32 textureIndex = 0;
            RhiUtils::FragmentTextureSet fragmentTextures;
            if (settings.enableColorGrading)
                fragmentTextures[textureIndex++] = settings.colorGradingTable->handle;
            if (settings.enableHeatMap)
                fragmentTextures[textureIndex++] = settings.heatmapTable->handle;
            fragmentTextures[textureIndex++] = settings.lightMeterTable->handle;

            rhi::Packet rectPacket = quadRenderer.rectPacket;
            material->BindParams(rectPacket);
            rectPacket.textureSet = RhiUtils::TextureSet({ lumTexture }, fragmentTextures);
            rhi::AddPacket(pl, rectPacket);
        }
    }
    else
    {
        RhiUtils::FragmentTextureSet fragmentTextures;
        {
            uint32 textureIndex = 0;
            fragmentTextures[textureIndex++] = hdrRenderer.hdrTarget;

            if (txaaEnabled)
            {
                fragmentTextures[textureIndex++] = Renderer::GetDynamicBindings().GetDynamicTexture(static_cast<DynamicBindings::eTextureSemantic>(DynamicBindings::DYNAMIC_TEXTURE_LDR_HISTORY));
                fragmentTextures[textureIndex++] = Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_VELOCITY);
            }

            if (settings.enableColorGrading)
                fragmentTextures[textureIndex++] = settings.colorGradingTable->handle;

            if (settings.enableHeatMap)
                fragmentTextures[textureIndex++] = settings.heatmapTable->handle;

            fragmentTextures[textureIndex++] = settings.lightMeterTable->handle;
        }
        options.renderPassName = "Combine";
        options.loadAction = rhi::LoadAction::LOADACTION_CLEAR;
        options.material = material;
        options.textureSet = RhiUtils::TextureSet({ lumTexture }, fragmentTextures);
        options.renderPassPriority = 4;

        bool invertProjection = (options.dstTexture != rhi::InvalidHandle) && (!rhi::DeviceCaps().isUpperLeftRTOrigin);
        float32 cv = invertProjection ? 1.0f : -1.0f;

        //GFX_COMPLETE fix projection flip for all back-ends
        if (rhi::HostApi() == rhi::RHI_METAL)
        {
            RenderFlow flow = Renderer::GetCurrentRenderFlow();
            bool halfRes = QualitySettingsSystem::Instance()->IsOptionEnabled(QualitySettingsSystem::QUALITY_OPTION_HALF_RESOLUTION_3D);

            if ((flow != RenderFlow::TileBasedHDRDeferred) || (flow == RenderFlow::TileBasedHDRDeferred && (!halfRes || txaaEnabled)))
                cv = 1.f;
        }

        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PROJECTION_FLIPPED, &cv, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);

        quadRenderer.Render(options);
    }

    if (debugRenderer.enableLightMeter)
    {
        float32 pickerSize = float32(DebugPickerSize);
        Vector2 poi = GetDebugRenderer()->pointOfInterest;

        QuadRenderer::Options pickerOptions;
        pickerOptions.renderPassName = "HDRPicker";
        pickerOptions.loadAction = rhi::LoadAction::LOADACTION_LOAD;
        pickerOptions.material = materials[MaterialType::COPY];
        pickerOptions.srcTexture = hdrRenderer.hdrTarget;
        pickerOptions.srcRect = Rect2f(poi.x - 0.5f * pickerSize, poi.y - 0.5f * pickerSize, pickerSize, pickerSize);
        pickerOptions.srcTexSize = Vector2(float32(hdrTargetSize.dx), float32(hdrTargetSize.dy));

        pickerOptions.dstTexture = hdrRenderer.hdrPicker[hdrRenderer.debugPickerIndex % DebugPickerPoolSize];
        pickerOptions.dstRect = Rect2f(0.0f, 0.0f, pickerSize, pickerSize);
        pickerOptions.dstTexSize = Vector2(pickerSize, pickerSize);

        pickerOptions.renderPassPriority = 4;
        quadRenderer.Render(pickerOptions);

        if (hdrRenderer.debugPickerIndex >= DebugPickerPoolSize)
        {
            uint32 readbackIndex = (hdrRenderer.debugPickerIndex + DebugPickerPoolSize - 2) % DebugPickerPoolSize;
            void* ptr = rhi::MapTexture(hdrRenderer.hdrPicker[readbackIndex]);
            memcpy(debugRenderer.debugPickerReadback, ptr, sizeof(debugRenderer.debugPickerReadback));
            rhi::UnmapTexture(hdrRenderer.hdrPicker[readbackIndex]);
            BuildDebugPickerLuminance();
        }

        ++hdrRenderer.debugPickerIndex;
    }
    else
    {
        hdrRenderer.debugPickerIndex = 0;
    }
}

void PostEffectRenderer::BuildDebugPickerLuminance()
{
    debugRenderer.debugPickedLuminance = Vector4(0.0f, 0.0f, 0.0f, 0.0f);
    for (uint32 y = 0; y < DebugPickerSize; ++y)
    {
        for (uint32 x = 0; x < DebugPickerSize; ++x)
        {
            debugRenderer.debugPickedLuminance += debugRenderer.debugPickerReadback[x + y * DebugPickerSize];
        }
    }
    debugRenderer.debugPickedLuminance /= float32(DebugPickerSize * DebugPickerSize);
    debugRenderer.debugPickedLuminance.w = debugRenderer.debugPickedLuminance.GetVector3().DotProduct(Vector3(0.2126f, 0.7152f, 0.0722f));
}

void PostEffectRenderer::Debug()
{
    const char* passName = "PostEffectDebug";

    Size2i drawOffset = debugRenderer.debugRectOffset;

    if (debugRenderer.drawHDRTarget)
    {
        QuadRenderer::Options options;
        options.renderPassName = passName;
        options.dstTexture = frameContext.destination;
        options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
        options.material = materials[MaterialType::DEBUG];
        options.srcTexture = hdrRenderer.hdrTarget;
        options.srcRect = Rect2f(0.f, 0.f, float32(frameContext.viewport.width), float32(frameContext.viewport.height));
        options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy), float32(debugRenderer.debugRectSize), float32(debugRenderer.debugRectSize));
        options.srcTexSize = Vector2(float32(hdrTargetSize.dx), float32(hdrTargetSize.dy));
        options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
        options.renderPassPriority = -1;
        quadRenderer.Render(options);

        if (debugRenderer.enableLightMeter)
        {
            drawOffset.dx += debugRenderer.debugRectOffset.dx + debugRenderer.debugRectSize;
            uint32 readbackDisplayIndex = (hdrRenderer.debugPickerIndex + DebugPickerPoolSize - 1) % DebugPickerPoolSize;
            options.srcTexture = hdrRenderer.hdrPicker[readbackDisplayIndex];
            options.srcRect = Rect2f(0.f, 0.f, float32(DebugPickerSize), float32(DebugPickerSize));
            options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy), float32(debugRenderer.debugRectSize), float32(debugRenderer.debugRectSize));
            options.srcTexSize = Vector2(float32(DebugPickerSize), float32(DebugPickerSize));
            options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
            options.renderPassPriority = -1;
            quadRenderer.Render(options);
            drawOffset.dx = debugRenderer.debugRectOffset.dx;
        }

        drawOffset.dy += debugRenderer.debugRectOffset.dy + debugRenderer.debugRectSize;
    }

    if (debugRenderer.drawLuminance)
    {
        for (size_t i = 0; i < allRenderer.averageColorArray.size(); ++i)
        {
            QuadRenderer::Options options;
            options.dstTexture = frameContext.destination;
            options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy), float32(debugRenderer.debugRectSize), float32(debugRenderer.debugRectSize));
            options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
            options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
            options.material = materials[MaterialType::DEBUG_R16F];
            options.srcRect = Rect2f(0.f, 0.f, allRenderer.averageColorSize[i], allRenderer.averageColorSize[i]);
            options.srcTexSize = Vector2(allRenderer.averageColorSize[i], allRenderer.averageColorSize[i]);
            options.srcTexture = allRenderer.averageColorArray[i];
            options.renderPassPriority = -1;
            options.renderPassName = passName;
            quadRenderer.Render(options);

            drawOffset.dx += debugRenderer.debugRectOffset.dx + debugRenderer.debugRectSize;
            if (drawOffset.dx + debugRenderer.debugRectSize > Renderer::GetFramebufferWidth())
            {
                drawOffset.dy += debugRenderer.debugRectOffset.dy + debugRenderer.debugRectSize;
                drawOffset.dx = debugRenderer.debugRectOffset.dx;
            }
        }

        drawOffset.dy += debugRenderer.debugRectOffset.dy + debugRenderer.debugRectSize;

        //{
        //    static int i = 1;
        //    i++;
        //    if (i % 100 == 0)
        //    {
        //        DumpAverageTexture(0);
        //        DumpAverageTexture(1);
        //        DumpAverageTexture(2);
        //        DumpAverageTexture(3);
        //        DumpAverageTexture(4);
        //        DumpAverageTexture(5);
        //        DumpAverageTexture(6);
        //        DumpAverageTexture(7);
        //        DumpAverageTexture(8);
        //        DumpAverageTexture(9);
        //        DumpAverageTexture(10);
        //    }
        //}
    }

    if (debugRenderer.drawAdaptation)
    {
        drawOffset.dx = debugRenderer.debugRectOffset.dx;

        QuadRenderer::Options options;
        options.renderPassName = passName;
        options.dstTexture = frameContext.destination;
        options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
        options.material = materials[MaterialType::DEBUG_R16F];
        options.srcTexture = allRenderer.luminanceHistory;
        options.srcRect = Rect2f(0.f, 0.f, 1.0f, 1.f);
        options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy), 8.f, 8.f);
        options.srcTexSize = Vector2(1.0f, 1.f);
        options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
        options.renderPassPriority = -1;
        quadRenderer.Render(options);

        drawOffset.dy += debugRenderer.debugRectOffset.dy + 8;
    }

    //if (histogramRenderer && debugRenderer.drawHistogram)
    //{
    //    QuadRenderer::Options options;
    //    options.dstTexture = frameContext.destination;
    //    options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
    //    options.material = posteffectMaterials[MaterialType::DEBUG];
    //    options.srcTexture = histogramRenderer->bucketTexture;
    //    options.srcRect = Rect2f(0.f, 0.f, float32(histogramRenderer->bucketsCount), 1);
    //    options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy), 8.f * histogramRenderer->bucketsCount, 8.f);
    //    options.srcTexSize = Vector2(float32(histogramRenderer->bucketsCount), 1.f);
    //    options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
    //    options.renderPassPriority = -1;
    //    quadRenderer.Render(options);
    //}

    BloomRenderer* b = static_cast<BloomRenderer*>(renderers[RendererType::BLOOM]);
    if (b && debugRenderer.drawBloom)
    {
        drawOffset.dx = debugRenderer.debugRectOffset.dx;

        for (size_t i = 0; i < b->BLUR_CHAIN_COUNT; ++i)
        {
            QuadRenderer::Options options;
            options.renderPassName = passName;
            options.dstTexture = frameContext.destination;
            options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy), float32(debugRenderer.debugRectSize), float32(debugRenderer.debugRectSize));
            options.dstTexSize = Vector2(float32(Renderer::GetFramebufferWidth()), float32(Renderer::GetFramebufferHeight()));
            options.loadAction = rhi::LoadAction::LOADACTION_LOAD;
            options.material = materials[MaterialType::DEBUG_R16F];
            options.srcRect = Rect2f(0.f, 0.f, float32(b->blurChain[i].blur.size.dx), float32(b->blurChain[i].blur.size.dy));
            options.srcTexSize = Vector2(float32(b->blurChain[i].blur.size.dx), float32(b->blurChain[i].blur.size.dy));
            options.srcTexture = b->blurChain[i].orig.handle;
            options.renderPassPriority = -1;
            quadRenderer.Render(options);

            options.srcTexture = b->blurChain[i].blur.handle;
            options.dstRect = Rect2f(float32(drawOffset.dx), float32(drawOffset.dy + debugRenderer.debugRectOffset.dy + debugRenderer.debugRectSize), float32(debugRenderer.debugRectSize), float32(debugRenderer.debugRectSize));
            quadRenderer.Render(options);

            drawOffset.dx += debugRenderer.debugRectOffset.dx + debugRenderer.debugRectSize;
        }

        drawOffset.dy += debugRenderer.debugRectOffset.dy + debugRenderer.debugRectSize;
    }
}

void PostEffectRenderer::Bloom()
{
    static const char* renderPassName = "Bloom";

    BloomRenderer* br = static_cast<BloomRenderer*>(renderers[RendererType::BLOOM]);
    if (br != nullptr)
    {
        auto& chain = br->blurChain;

        NMaterial* material = materials[MaterialType::BLOOM_THRESHOLD];
        material->SetPropertyValue(FastName("bloomEVC"), &settings.bloomEVC);
        // material->SetPropertyValue(FastName("EV"), &ev100);

        QuadRenderer::Options o;
        o.renderPassName = "bloom_threshold";
        o.dstRect = { 0.f, 0.f, float32(chain[0].orig.size.dx), float32(chain[0].orig.size.dy) };
        o.dstTexSize = { float32(chain[0].orig.size.dx), float32(chain[0].orig.size.dy) };
        o.dstTexture = chain[0].orig.handle;
        o.material = material;
        o.srcRect = { 0.f, 0.f, float32(hdrTargetSize.dx), float32(hdrTargetSize.dy) };
        o.srcTexSize = { float32(hdrTargetSize.dx), float32(hdrTargetSize.dy) };
        o.srcTexture = hdrRenderer.hdrTarget;
        quadRenderer.Render(o);

        //downsample
        for (int32 i = 0; i < br->BLUR_CHAIN_COUNT - 1; ++i)
        {
            QuadRenderer::Options o;
            o.renderPassName = "bloom_downsample";
            o.material = materials[MaterialType::LUMINANCE];
            o.srcTexture = chain[i].orig.handle;
            o.srcRect = Rect2f(0.f, 0.f, float32(chain[i].orig.size.dx), float32(chain[i].orig.size.dy));
            o.srcTexSize = Vector2(float32(chain[i].orig.size.dx), float32(chain[i].orig.size.dy));
            o.dstTexture = chain[i + 1].orig.handle;
            o.dstRect = Rect2f(0.f, 0.f, float32(chain[i + 1].orig.size.dx), float32(chain[i + 1].orig.size.dy));
            o.dstTexSize = Vector2(float32(chain[i + 1].orig.size.dx), float32(chain[i + 1].orig.size.dy));
            quadRenderer.Render(o);
        }

        Blur(&br->blurChain[4].orig, &br->blurChain[4].orig, &br->blurChain[4].blur);
        //Blur(&br->blurChain[3].orig, &br->blurChain[3].orig, &br->blurChain[3].blur);
        //Blur(&br->blurChain[2].orig, &br->blurChain[2].orig, &br->blurChain[2].blur);
        //Blur(&br->blurChain[1].orig, &br->blurChain[1].orig, &br->blurChain[1].blur);
        //Blur(&br->blurChain[0].orig, &br->blurChain[0].orig, &br->blurChain[0].blur);
        MulAddBlur(&chain[3].orig, &chain[4].orig, 4.f / 21.f, 5.f / 21.f, &chain[3].blur);
        MulAddBlur(&chain[2].orig, &chain[3].blur, 3.f / 21.f, 1.f, &chain[2].blur);
        MulAddBlur(&chain[1].orig, &chain[2].blur, 2.f / 21.f, 1.f, &chain[1].blur);
        MulAddBlur(&chain[0].orig, &chain[1].blur, 1.f / 21.f, 1.f, &chain[0].blur);
    }
}

void PostEffectRenderer::Blur(RhiTexture* src, RhiTexture* dst, RhiTexture* tmp)
{
    rhi::SamplerState::Descriptor samplerDescriptor;
    samplerDescriptor.fragmentSamplerCount = 2;
    samplerDescriptor.fragmentSampler[0].addrU = rhi::TEXADDR_CLAMP;
    samplerDescriptor.fragmentSampler[0].addrV = rhi::TEXADDR_CLAMP;
    samplerDescriptor.fragmentSampler[0].minFilter = rhi::TEXFILTER_LINEAR;
    samplerDescriptor.fragmentSampler[0].magFilter = rhi::TEXFILTER_LINEAR;
    samplerDescriptor.fragmentSampler[0].mipFilter = rhi::TEXMIPFILTER_NEAREST;
    samplerDescriptor.fragmentSampler[1].addrU = rhi::TEXADDR_CLAMP;
    samplerDescriptor.fragmentSampler[1].addrV = rhi::TEXADDR_CLAMP;
    samplerDescriptor.fragmentSampler[1].minFilter = rhi::TEXFILTER_LINEAR;
    samplerDescriptor.fragmentSampler[1].magFilter = rhi::TEXFILTER_LINEAR;
    samplerDescriptor.fragmentSampler[1].mipFilter = rhi::TEXMIPFILTER_NEAREST;
    rhi::HSamplerState samplerState = rhi::AcquireSamplerState(samplerDescriptor);

    //horiz
    NMaterial* material = materials[MaterialType::GAUSS_BLUR];
    Vector2 texelOffset{ 0, 1.f / src->size.dy };
    material->SetPropertyValue(FastName("texelOffset"), texelOffset.data);
    QuadRenderer::Options o;
    o.renderPassName = "bloom_blur_horiz";
    o.material = material;
    o.dstRect = Rect2f(0, 0, float32(tmp->size.dx), float32(tmp->size.dy));
    o.dstTexSize = Vector2(float32(tmp->size.dx), float32(tmp->size.dy));
    o.dstTexture = tmp->handle;
    o.srcTexture = src->handle;
    o.srcTexSize = Vector2(float32(src->size.dx), float32(src->size.dy));
    o.srcRect = Rect2f(0, 0, float32(src->size.dx), float32(src->size.dy));
    o.samplerState = samplerState;
    quadRenderer.Render(o);

    //vert
    texelOffset = { 1.f / tmp->size.dy, 0 };
    material->SetPropertyValue(FastName("texelOffset"), texelOffset.data);
    o.renderPassName = "bloom_blur_vert";
    o.dstTexture = dst->handle;
    o.dstTexSize = Vector2(float32(dst->size.dx), float32(dst->size.dy));
    o.dstRect = Rect2f(0, 0, float32(dst->size.dx), float32(dst->size.dy));
    o.srcRect = Rect2f(0, 0, float32(tmp->size.dx), float32(tmp->size.dy));
    o.srcTexSize = Vector2(float32(tmp->size.dx), float32(tmp->size.dy));
    o.srcTexture = tmp->handle;
    o.samplerState = samplerState;
    quadRenderer.Render(o);
}

void PostEffectRenderer::MulAddBlur(RhiTexture* src1, RhiTexture* src2, float32 mul1, float32 mul2, RhiTexture* dst)
{
    // static const char* renderPassName = "Bloom";
    NMaterial* material = materials[MaterialType::MUL_ADD_BLUR];
    Vector2 mul{ mul1, mul2 };
    material->SetPropertyValue(FastName("mulAdd"), mul.data);

    QuadRenderer::Options o;
    o.renderPassName = "bloom_mul_add";
    o.material = material;
    o.dstTexture = dst->handle;
    o.dstTexSize = { float32(dst->size.dx), float32(dst->size.dy) };
    o.dstRect = { 0, 0, float32(dst->size.dx), float32(dst->size.dy) };
    o.srcTexSize = { 512.f, 512.f };
    o.srcRect = { 0, 0, 512.f, 512.f };

    o.textureSet = RhiUtils::FragmentTextureSet{ src1->handle, src2->handle };

    quadRenderer.Render(o);

    Blur(dst, dst, src1);
}

void PostEffectRenderer::DumpAverageTexture(int32 index)
{
    rhi::HTexture handle = allRenderer.averageColorArray[index];
    void* mappedData = rhi::MapTexture(handle);
    {
        uint32 imageSize = static_cast<uint32>(allRenderer.averageColorSize[index]);
        ScopedPtr<Image> image(Image::CreateFromData(imageSize, imageSize, PixelFormat::FORMAT_RGBA16F, static_cast<uint8*>(mappedData)));
        ImageSystem::Save(Format("E:\\temp\\av%d.dds", index), image, PixelFormat::FORMAT_RGBA16F);
    }
    rhi::UnmapTexture(handle);
}

void PostEffectRenderer::InitResources(const Size2i& newSize)
{
    if (sharedResourcesCreated == false)
    {
        hdrTargetSize = Renderer::GetRuntimeTextures().GetRuntimeTextureSize(RuntimeTextures::TEXTURE_SHARED_DEPTHBUFFER);

        // create main render target
        {
            rhi::Texture::Descriptor descriptor;
            descriptor.width = hdrTargetSize.dx;
            descriptor.height = hdrTargetSize.dy;
            descriptor.autoGenMipmaps = false;
            descriptor.isRenderTarget = true;
            descriptor.needRestore = false;
            descriptor.type = rhi::TEXTURE_TYPE_2D;
            descriptor.format = rhi::TEXTURE_FORMAT_RGBA16F;
            descriptor.cpuAccessRead = false;
            descriptor.cpuAccessWrite = false;
            hdrRenderer.hdrTarget = rhi::CreateTexture(descriptor);

            descriptor.format = rhi::TEXTURE_FORMAT_RGBA32F;
            descriptor.width = DebugPickerSize;
            descriptor.height = DebugPickerSize;
            descriptor.cpuAccessRead = true;
            for (uint32 i = 0; i < DebugPickerPoolSize; ++i)
                hdrRenderer.hdrPicker[i] = rhi::CreateTexture(descriptor);
        }

        // create luminance textures
        {
            rhi::Texture::Descriptor descriptor;
            descriptor.width = 1;
            descriptor.height = 1;
            descriptor.autoGenMipmaps = false;
            descriptor.isRenderTarget = true;
            descriptor.needRestore = false;
            descriptor.type = rhi::TEXTURE_TYPE_2D;
            descriptor.format = rhi::TEXTURE_FORMAT_R16F;
            descriptor.cpuAccessRead = false;
            descriptor.cpuAccessWrite = false;
            allRenderer.luminancePrevious = rhi::CreateTexture(descriptor);
            allRenderer.luminanceTexture = rhi::CreateTexture(descriptor);
            allRenderer.luminanceHistory = rhi::CreateTexture(descriptor);
            settings.resetHistory = true;
        }

        sharedResourcesCreated = true;
    }

    if (newSize != lastWindowSize)
    {
        int32 baseDownsampleChainSize = NextPowerOf2(std::min(newSize.dx, newSize.dy));
        int32 lastDownsampleChainSize = allRenderer.averageColorSize.empty() ? 0 : (static_cast<int32>(allRenderer.averageColorSize.front()) * 2);

        if (baseDownsampleChainSize != lastDownsampleChainSize)
        {
            Logger::Info("Recreating color chain: %d -> %d", lastDownsampleChainSize, baseDownsampleChainSize);
            DestroyResources(false);
            float32 averageColorSize = static_cast<float32>(baseDownsampleChainSize);
            while (true)
            {
                averageColorSize = std::max(1.0f, 0.5f * averageColorSize);

                rhi::Texture::Descriptor descriptor;
                descriptor.width = static_cast<uint32>(averageColorSize);
                descriptor.height = descriptor.width;
                descriptor.autoGenMipmaps = false;
                descriptor.isRenderTarget = true;
                descriptor.needRestore = false;
                descriptor.type = rhi::TEXTURE_TYPE_2D;
                descriptor.format = rhi::TEXTURE_FORMAT_R16F;
                descriptor.cpuAccessRead = false;
                descriptor.cpuAccessWrite = false;
                allRenderer.averageColorArray.push_back(rhi::CreateTexture(descriptor));
                allRenderer.averageColorSize.push_back(averageColorSize);
                settings.resetHistory = true;

                if (std::abs(averageColorSize - 1.0f) <= std::numeric_limits<float>::epsilon())
                    break;
            }
        }

        for (HelperRenderer* r : renderers)
        {
            if (r != nullptr)
                r->InitResources(newSize, settings);
        }

        lastWindowSize = newSize;
    }
}

void PostEffectRenderer::DestroyResources(bool includeShared)
{
    for (HelperRenderer* r : renderers)
    {
        if (r != nullptr)
            r->DestroyResources();
    }

    for (rhi::HTexture tex : allRenderer.averageColorArray)
        rhi::DeleteTexture(tex);

    allRenderer.averageColorArray.clear();
    allRenderer.averageColorSize.clear();

    if (includeShared)
    {
        rhi::DeleteTexture(allRenderer.luminancePrevious);
        rhi::DeleteTexture(allRenderer.luminanceHistory);
        rhi::DeleteTexture(allRenderer.luminanceTexture);
        rhi::DeleteTexture(hdrRenderer.hdrTarget);
    }
}
}
