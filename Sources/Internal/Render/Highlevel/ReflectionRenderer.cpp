#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/ReflectionRenderer.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Render/Highlevel/CubemapRenderer.h"
#include "Render/Highlevel/RenderLayer.h"
#include "Render/Highlevel/RenderPass.h"
#include "Render/Material/NMaterialManager.h"
#include "Render/RhiUtils.h"
#include "Logger/Logger.h"

namespace DAVA
{
const uint32 HIGH_RES_FBO_CUBEMAP_SIZE = 256;
const uint32 CONVOLUTION_MIP_COUNT = 1 + StaticLog2<HIGH_RES_FBO_CUBEMAP_SIZE>::value;
const uint32 MAX_NUMBER_OF_PROBES_TO_UPDATE_PER_FRAME = 1;

const std::pair<uint32, uint32> cacheCubemapFaceSize[ReflectionRenderer::CUBEMAP_QUALITY_LEVELS] = { { 256, 9 }, { 128, 8 }, { 64, 7 }, { 32, 6 } };
const uint32 maxCacheCubemapOnEachLevel[ReflectionRenderer::CUBEMAP_QUALITY_LEVELS] = { 4, 8, 16, 32 };

const FastName DISPLAY_SPHERICAL_HARMONICS = FastName("DISPLAY_SPHERICAL_HARMONICS");
const FastName DISPLAY_INDIRECT_LOOKUP = FastName("DISPLAY_INDIRECT_LOOKUP");
const FastName GLOBAL_SPHERICAL_HARMONICS = FastName("sphericalHarmonics");
const PixelFormat ReflectionsIntermediateTextureFormat = PixelFormat::FORMAT_RGBA16F;
const PixelFormat ReflectionsTargetTextureFormat = PixelFormat::FORMAT_RGBM;

ReflectionRenderer::ReflectionRenderer(RenderSystem* renderSystem_)
    : debugMaterial(new NMaterial())
{
    renderSystem = renderSystem_;

    reflectionPass = new RenderPass(PASS_IB_REFLECTION);
    reflectionPass->GetPassConfig().priority = PRIORITY_SERVICE_3D;
    reflectionPass->AddRenderLayer(new RenderLayer(RENDER_LAYER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_OPAQUE));
    reflectionPass->AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_OPAQUE_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_OPAQUE));
    reflectionPass->AddRenderLayer(new RenderLayer(RENDER_LAYER_ALPHA_TEST_LAYER_ID, RenderLayer::LAYER_SORTING_FLAGS_ALPHA_TEST_LAYER));
    reflectionPass->AddRenderLayer(new RenderLayer(RENDER_LAYER_SKY_ID, 0)); //after all depth writing materials
    reflectionPass->AddRenderLayer(new RenderLayer(RENDER_LAYER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_TRANSLUCENT));
    reflectionPass->AddRenderLayer(new RenderLayer(RENDER_LAYER_AFTER_TRANSLUCENT_ID, RenderLayer::LAYER_SORTING_FLAGS_AFTER_TRANSLUCENT));

    temporaryFramebuffer = Texture::CreateFBO(HIGH_RES_FBO_CUBEMAP_SIZE, HIGH_RES_FBO_CUBEMAP_SIZE, ReflectionsIntermediateTextureFormat, true, rhi::TEXTURE_TYPE_CUBE);
    temporaryFramebuffer->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    downsampledFramebuffer = CreateCubeTextureForReflection(HIGH_RES_FBO_CUBEMAP_SIZE, CONVOLUTION_MIP_COUNT, ReflectionsIntermediateTextureFormat);
    globalProbeSpecularConvolution = CreateCubeTextureForReflection(HIGH_RES_FBO_CUBEMAP_SIZE, CONVOLUTION_MIP_COUNT, ReflectionsTargetTextureFormat);

    for (uint32 qualityLevel = 0; qualityLevel < CUBEMAP_QUALITY_LEVELS; ++qualityLevel)
    {
        for (uint32 t = 0; t < maxCacheCubemapOnEachLevel[qualityLevel]; ++t)
        {
            Texture* texture = CreateCubeTextureForReflection(cacheCubemapFaceSize[qualityLevel].first, cacheCubemapFaceSize[qualityLevel].second, ReflectionsTargetTextureFormat);
            textureCache[qualityLevel].push_back(texture);
            allCacheTextures.push_back(texture);
        }
    }

    float level = 0.0f;
    Vector2 defaultTexSize{ 2048.f, 2048.f };
    Vector2 defaultTexOffset{ 0, 0 };
    Vector4 dummySphericalHarmonics[9];
    debugMaterial->SetFXName(FastName("~res:/Materials2/CubemapDebug.material"));
    debugMaterial->AddProperty(FastName("srcRectOffset"), defaultTexOffset.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("srcRectSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("srcTexSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("destTexSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("sampledLevel"), &level, rhi::ShaderProp::Type::TYPE_FLOAT1);
    debugMaterial->AddProperty(GLOBAL_SPHERICAL_HARMONICS, dummySphericalHarmonics->data, rhi::ShaderProp::TYPE_FLOAT4, 9);
    debugMaterial->AddFlag(DISPLAY_SPHERICAL_HARMONICS, 0);
    debugMaterial->AddFlag(DISPLAY_INDIRECT_LOOKUP, 0);

    invalidateCallback = NMaterialManager::Instance().RegisterInvalidateCallback(MakeFunction(this, &ReflectionRenderer::InvalidateMaterials));
}

ReflectionRenderer::~ReflectionRenderer()
{
    NMaterialManager::Instance().UnregisterInvalidateCallback(invalidateCallback);

    for (size_t k = 0; k < allCacheTextures.size(); ++k)
        SafeRelease(allCacheTextures[k]);

    SafeRelease(globalProbeSpecularConvolution);
    SafeRelease(downsampledFramebuffer);
    SafeRelease(temporaryFramebuffer);
    SafeDelete(reflectionPass);
}

Texture* ReflectionRenderer::CreateCubeTextureForReflection(uint32 size, uint32 mipCount, PixelFormat format)
{
    Texture::FBODescriptor fboDesc;
    fboDesc.width = size;
    fboDesc.height = size;
    fboDesc.mipLevelsCount = mipCount;
    fboDesc.format = format;
    fboDesc.textureType = rhi::TEXTURE_TYPE_CUBE;
    fboDesc.needDepth = false;

    Texture* texture = Texture::CreateFBO(fboDesc);
    texture->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_LINEAR);
    texture->SetWrapMode(rhi::TEXADDR_CLAMP, rhi::TEXADDR_CLAMP);
    return texture;
}

void ReflectionRenderer::RegisterReflectionProbe(ReflectionProbe* probe)
{
    DVASSERT(probe != nullptr);

    switch (probe->GetReflectionType())
    {
    case ReflectionProbe::ProbeType::GLOBAL:
    case ReflectionProbe::ProbeType::GLOBAL_STATIC:
    {
        DVASSERT(globalReflectionProbe == nullptr);
        globalReflectionProbe = SafeRetain(probe);
        break;
    }
    case ReflectionProbe::ProbeType::LOCAL:
    case ReflectionProbe::ProbeType::LOCAL_STATIC:
    {
        localReflectionProbes.emplace_back(SafeRetain(probe));
        break;
    }
    default:
        DVASSERT(!"Invalid reflection probe type");
    }
}

void ReflectionRenderer::UnregisterReflectionProbe(ReflectionProbe* probe)
{
    DVASSERT(probe != nullptr);

    if (globalReflectionProbe == probe)
        globalReflectionProbe = nullptr;

    if (probe->GetActiveQualityLevel() != ReflectionProbe::INVALID_QUALITY_LEVEL)
        ReleaseProbeTexture(probe);

    FindAndRemoveExchangingWithLast(localReflectionProbes, probe);
    FindAndRemoveExchangingWithLast(probesForUpdate, ReflectionProbeToUpdate(probe, 0 /* since `operator ==` compares only pointers */));
    SafeRelease(probe);
}

// GFX_COMPLETE:
// 1. Probe moves / Objects move / Dynamic objects ?
// 2. When we move probe we assign it to objects that are fall into it's influence

void ReflectionRenderer::UpdateProbe(ReflectionProbe* probe)
{
    // priority is set accoring to probe type (global probes receive more priority)
    ReflectionProbeToUpdate update(probe, ~(1 << static_cast<uint32>(probe->GetReflectionType())));
    if (std::find(probesForUpdate.begin(), probesForUpdate.end(), update) == probesForUpdate.end())
        probesForUpdate.emplace_back(update);
}

/*
void ReflectionRenderer::SaveCubemap(const FilePath & filePath, Texture * cubemap, uint32 mipmapLevelCount)
{
    Vector<Vector<Image*>> cubemapImages;
    cubemap->CreateCubemapMipmapImages(cubemapImages, mipmapLevelCount);

    ImageSystem::Instance()->SaveAsCubeMap(filePath, cubemapImages);
    TextureDescriptor texDescriptor;
    texDescriptor.dataSettings.sourceFileExtension = ".dds";
    texDescriptor.dataSettings.sourceFileFormat = ImageFormat::IMAGE_FORMAT_DDS;
    texDescriptor.dataSettings.cubefaceFlags = 0xFF;
    texDescriptor.drawSettings.magFilter = Texture::FILTER_LINEAR;

    if (mipmapLevelCount > 1)
    {
        texDescriptor.drawSettings.minFilter = Texture::FILTER_LINEAR_MIPMAP_LINEAR;
    }
    else if (mipmapLevelCount == 1)
    {
        texDescriptor.drawSettings.minFilter = Texture::FILTER_LINEAR;
    }

    texDescriptor.drawSettings.wrapModeS = texDescriptor.drawSettings.wrapModeT = Texture::WRAP_CLAMP_TO_EDGE;

    FilePath texDescriptorFilePath = filePath;
    texDescriptorFilePath.ReplaceExtension(".tex");
    texDescriptor.Save(texDescriptorFilePath);
}*/

Texture* ReflectionRenderer::GetSpecularConvolution2()
{
    return globalProbeSpecularConvolution;
}

void ReflectionRenderer::EnumerateMaterials(Set<NMaterial*>& materials)
{
    Vector<RenderObject*>& renderObjectArray = renderSystem->GetRenderObjectArray();
    for (RenderObject* ro : renderObjectArray)
    {
        if (ro != nullptr)
        {
            for (uint32 i = 0, batchCount = ro->GetRenderBatchCount(); i < batchCount; ++i)
                materials.insert(ro->GetRenderBatch(i)->GetMaterial());
        }
    }
}

void ReflectionRenderer::AllocateProbeTexture(ReflectionProbe* probe)
{
    DVASSERT(probe->IsDynamicProbe());

    if (probe->GetReflectionType() == ReflectionProbe::ProbeType::GLOBAL)
    {
        probe->SetCurrentTexture(globalProbeSpecularConvolution);
    }
    else if ((probe->GetCurrentTexture() == globalProbeSpecularConvolution) || (probe->GetCurrentTexture() == nullptr))
    {
        uint32 qualityLevel = probe->GetNextQualityLevel();
        DVASSERT(qualityLevel < CUBEMAP_QUALITY_LEVELS);

        Texture* texture = textureCache[qualityLevel].front();
        textureCache[qualityLevel].pop_front();

        probe->SetCurrentTexture(texture);
        probe->SetActiveQualityLevel(probe->GetNextQualityLevel());
    }
}

void ReflectionRenderer::ReleaseProbeTexture(ReflectionProbe* probe)
{
    Texture* probeTexture = probe->GetCurrentTexture();
    if (probe->IsStaticProbe() || (probeTexture == nullptr) || (probeTexture == globalProbeSpecularConvolution))
        return;

    uint32 probeQualityLevel = probe->GetActiveQualityLevel();
    textureCache[probeQualityLevel].push_back(probeTexture);
    probe->SetCurrentTexture(globalProbeSpecularConvolution);
}

void ReflectionRenderer::RenderReflectionProbe(ReflectionProbe* probe)
{
    DVASSERT(probe != nullptr);

    if (probe->IsStaticProbe())
        return;

    Texture* target = probe->GetCurrentTexture();
    DVASSERT(target != nullptr);

    const uint32 layerFilter = 0xffffffff;
    CubemapRenderer* cmr = renderSystem->GetCubemapRenderer();
    SphericalHarmonicsUpdate shUpdate = EnqueueSphericalHarmonicsUpdate(probe);

    Vector3 capturePosition = probe->GetPosition() + probe->GetCapturePosition();
    cmr->RenderCubemap(renderSystem, reflectionPass, capturePosition, temporaryFramebuffer, layerFilter);
    cmr->EdgeFilterCubemap(temporaryFramebuffer, downsampledFramebuffer, CONVOLUTION_MIP_COUNT);
    cmr->ConvoluteSphericalHarmonics(downsampledFramebuffer, shUpdate.targetTexture);
    cmr->ConvoluteSpecularCubemap(downsampledFramebuffer, target, target->GetMipLevelsCount());
}

void ReflectionRenderer::UpdateProbeMaterialBindings(ReflectionProbe* probe)
{
    if (probe->IsGlobalProbe())
    {
        Vector<RenderObject*>& renderObjectArray = renderSystem->GetRenderObjectArray();
        for (RenderObject* ro : renderObjectArray)
        {
            ro->SetGlobalReflectionProbe(probe);
            ro->SetLocalReflectionProbe(probe);
        }
    }
    else
    {
        Vector<RenderObject*> reflectionProbeObjects;
        Vector3 boxCenter = probe->GetPosition();
        AABBox3 box(boxCenter - probe->GetCaptureSize() / 2.0f, boxCenter + probe->GetCaptureSize() / 2.0f);
        renderSystem->GetRenderHierarchy()->GetAllObjectsInBBox(box, reflectionProbeObjects);

        for (RenderObject* ro : reflectionProbeObjects)
            ro->SetLocalReflectionProbe(probe);
    }
}

void ReflectionRenderer::Draw(Camera* camera)
{
    RetrieveUpdatedSphericalHarmonics();

    reflectionPass->SetName(Renderer::GetCurrentRenderFlow() == RenderFlow::LDRForward ? PASS_IB_REFLECTION_LDR : PASS_IB_REFLECTION);

    for (Landscape* landscape : renderSystem->GetLandscapes())
        landscape->SetPageUpdateLocked(true);

    SCOPE_EXIT
    {
        for (Landscape* landscape : renderSystem->GetLandscapes())
            landscape->SetPageUpdateLocked(false);
    };

    /*
        1. Найти ближайшие к камере
        2. Определить сколько из них уже в приемлимом разрешении.
        3. Поставить в очередь для обновлений (без дубликатов, и чтобы не происходило лишней работы)
    */

    auto sortFunction = [camera](const ReflectionProbe* probeA, const ReflectionProbe* probeB) -> bool
    {
        float32 squareDistanceA = (probeA->GetPosition() - camera->GetPosition()).SquareLength();
        float32 squareDistanceB = (probeB->GetPosition() - camera->GetPosition()).SquareLength();
        return squareDistanceA < squareDistanceB;
    };
    std::sort(localReflectionProbes.begin(), localReflectionProbes.end(), sortFunction);

    uint32 size = static_cast<uint32>(localReflectionProbes.size());

    uint32 cubemapQualityLevel = 0;
    uint32 cubemapsOnEachLevel[CUBEMAP_QUALITY_LEVELS] = {};
    for (uint32 k = 0; k < CUBEMAP_QUALITY_LEVELS; ++k)
        cubemapsOnEachLevel[k] = maxCacheCubemapOnEachLevel[k];

    uint32 k = 0;
    for (; k < size; ++k)
    {
        if (localReflectionProbes[k]->IsStaticProbe())
        {
            // skip static probes
        }
        else if (cubemapsOnEachLevel[cubemapQualityLevel] > 0)
        {
            cubemapsOnEachLevel[cubemapQualityLevel]--;
            localReflectionProbes[k]->SetNextQualityLevel(cubemapQualityLevel);
        }
        else
        {
            cubemapQualityLevel++;
            if (cubemapQualityLevel >= CUBEMAP_QUALITY_LEVELS)
                break;
            k--;
        }
    }

    while (k++ < size)
    {
        localReflectionProbes[k]->SetNextQualityLevel(ReflectionProbe::INVALID_QUALITY_LEVEL);
    }

    k = 0;
    for (ReflectionProbe* probe : localReflectionProbes)
    {
        if (probe->GetNextQualityLevel() != probe->GetActiveQualityLevel())
        {
            ReleaseProbeTexture(probe);
            UpdateProbe(probe);
        }

        // debug info, can be disabled
        if (probe->GetNextQualityLevel() < probe->GetActiveQualityLevel())
        {
            Logger::FrameworkDebug("Increase quality of probe: %d to level %d", k, probe->GetNextQualityLevel());
        }
        else if (probe->GetNextQualityLevel() > probe->GetActiveQualityLevel())
        {
            if (probe->GetNextQualityLevel() == 0xFFFFFFFF)
            {
                Logger::FrameworkDebug("Decrease quality of probe: %d to global probe", k);
            }
            else
            {
                Logger::FrameworkDebug("Decrease quality of probe: %d to level %d", probe->GetNextQualityLevel());
            }
        }
        ++k;
        // */
    }

    uint32 probesForUpdateCount = uint32(probesForUpdate.size());
    if (probesForUpdateCount > 0)
    {
        std::sort(probesForUpdate.begin(), probesForUpdate.end()); // sort by priority
        uint32 count = std::min(probesForUpdateCount, MAX_NUMBER_OF_PROBES_TO_UPDATE_PER_FRAME);
        for (size_t i = 0; i < count; ++i)
        {
            const ReflectionProbeToUpdate& probeWithPriority = probesForUpdate[i];
            ReflectionProbe* reflectionProbe = probeWithPriority.reflectionProbe;
            if (reflectionProbe->IsDynamicProbe())
            {
                AllocateProbeTexture(reflectionProbe);
                RenderReflectionProbe(reflectionProbe);
            }
            UpdateProbeMaterialBindings(reflectionProbe);
            probesForUpdate.erase(probesForUpdate.begin(), probesForUpdate.begin() + count);
        }
    }

    Vector4* globalSh = (globalReflectionProbe != nullptr) ? globalReflectionProbe->GetSphericalHarmonicsArray() : emptySphericalHarmonicsArray;
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_GLOBAL_DIFFUSE_SPHERICAL_HARMONICS, globalSh, reinterpret_cast<pointer_size>(globalSh));

    DrawDebugInfo();
}

void ReflectionRenderer::DrawDebugInfo()
{
    if (debugDrawProbe == nullptr)
        return;

    float32 gap = 3.0f;
    float32 levelsPerColumn = 8.0f; // to fit all convolution levels
    float32 debugRenderHeight = (static_cast<float>(Renderer::GetFramebufferHeight()) - (1.0f + levelsPerColumn) * gap) / levelsPerColumn;
    float32 debugRenderWidth = 2.0f * debugRenderHeight;

    debugMaterial->SetFlag(DISPLAY_INDIRECT_LOOKUP, 0);
    debugMaterial->SetFlag(DISPLAY_SPHERICAL_HARMONICS, 0);

    float32 xPos = gap;
    float32 yPos = gap;

    auto advancePosition = [&xPos, &yPos, gap, debugRenderHeight, debugRenderWidth]() {
        yPos += debugRenderHeight + gap;
        if (yPos >= static_cast<float>(Renderer::GetFramebufferHeight()))
        {
            yPos = gap;
            xPos += gap + debugRenderWidth;
        }
    };

    const FastName envTextureName("environmentMap");

    if (debugMaterial->HasLocalTexture(envTextureName))
        debugMaterial->SetTexture(envTextureName, debugDrawProbe->GetCurrentTexture());
    else
        debugMaterial->AddTexture(envTextureName, debugDrawProbe->GetCurrentTexture());

    QuadRenderer::Options options;
    options.loadAction = rhi::LOADACTION_LOAD;
    options.material = debugMaterial;
    options.srcTexture = debugDrawProbe->GetCurrentTexture()->handle;
    options.srcTexSize = Vector2(1.0f, 1.0f);
    options.srcRect = Rect2f(0.0f, 0.0f, 1.0f, 1.0f);
    options.dstTexture = rhi::HTexture();
    options.dstTexSize = Vector2(static_cast<float>(Renderer::GetFramebufferWidth()), static_cast<float>(Renderer::GetFramebufferHeight()));
    options.renderPassPriority = -200;
    options.dstRect = Rect2f(xPos, yPos, debugRenderWidth, debugRenderHeight);

    for (float32 sampledLevel = 0.0f; sampledLevel < static_cast<float32>(CONVOLUTION_MIP_COUNT); sampledLevel += 1.0f)
    {
        debugMaterial->SetPropertyValue(FastName("sampledLevel"), &sampledLevel);
        options.dstRect = Rect2f(xPos, yPos, debugRenderWidth, debugRenderHeight);
        quadRenderer.Render(options);
        advancePosition();
    }

    {
        debugMaterial->SetFlag(DISPLAY_INDIRECT_LOOKUP, 0);
        debugMaterial->SetFlag(DISPLAY_SPHERICAL_HARMONICS, 1);
        debugMaterial->SetPropertyValue(GLOBAL_SPHERICAL_HARMONICS, debugDrawProbe->GetSphericalHarmonicsArray()->data);
        options.dstRect = Rect2f(xPos, yPos, debugRenderWidth, debugRenderHeight);
        quadRenderer.Render(options);
        advancePosition();
    }

    {
        debugMaterial->SetFlag(DISPLAY_INDIRECT_LOOKUP, 1);
        debugMaterial->SetFlag(DISPLAY_SPHERICAL_HARMONICS, 0);
        options.dstRect = Rect2f(xPos, yPos, std::max(debugRenderHeight, debugRenderWidth), std::max(debugRenderHeight, debugRenderWidth));
        options.textureSet = RhiUtils::FragmentTextureSet({ Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_INDIRECT_SPECULAR_LOOKUP) });
        quadRenderer.Render(options);
        advancePosition();
    }

#if 0
    static int i = 1;
    i++;
    if (!(i % 200))
    {
        Logger::Info(" * SAVING REFLECTIONS ");

        Vector<Vector<Image*>> cubemapImages;
        reflectionHighresFBO->CreateCubemapMipmapImages(cubemapImages, 1);
        ImageSystem::SaveAsCubeMap("~doc:/refl/cubemap.dds", cubemapImages);
    }
#endif

#if 0
    rhi::RenderPassConfig renderTargetConfig;

    renderTargetConfig.colorBuffer[0].loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.depthStencilBuffer.loadAction = rhi::LOADACTION_CLEAR;
    renderTargetConfig.priority = DAVA::PRIORITY_SERVICE_3D;

    renderTargetConfig.colorBuffer[0].clearColor[0] = 1.0f;
    renderTargetConfig.colorBuffer[0].clearColor[1] = 0.0f;
    renderTargetConfig.colorBuffer[0].clearColor[2] = 0.0f;
    renderTargetConfig.colorBuffer[0].clearColor[3] = 1.0f;

    renderTargetConfig.colorBuffer[0].texture = reflectionHighresFBO_2D->handle;
    renderTargetConfig.depthStencilBuffer.texture = reflectionHighresFBO_2D->handleDepthStencil;
    renderTargetConfig.viewport.width = reflectionHighresFBO_2D->GetWidth();
    renderTargetConfig.viewport.height = reflectionHighresFBO_2D->GetHeight();

    rhi::RenderPassConfig oldPassConfig = reflectionPass->GetPassConfig();
    reflectionPass->GetPassConfig() = renderTargetConfig;
    reflectionPass->Draw(renderSystem);

    reflectionPass->GetPassConfig() = oldPassConfig;

    static int i = 1;
    i++;
    if (!(i % 200))
    {
        Logger::Info(" * SAVING REFLECTIONS ");

        String folder = String("~doc:/refl/");
        String fileName;
        FileSystem::Instance()->CreateDirectory(FilePath(folder), true);
        Size2i size(reflectionHighresFBO_2D->width, reflectionHighresFBO_2D->height);
        Image* img;
        void* data;
        rhi::HTexture tex;

        fileName = folder + DAVA::Format("globalReflection.png");
        tex = reflectionHighresFBO_2D->handle;
        data = rhi::MapTexture(tex, 0);
        img = Image::CreateFromData(size.dx, size.dy, PixelFormat::FORMAT_RGBA8888, reinterpret_cast<uint8*>(data));
        img->Save(FilePath(fileName));
        SafeRelease(img);
        rhi::UnmapTexture(tex);
    }
#endif
}

void ReflectionRenderer::InvalidateMaterials()
{
    debugMaterial->InvalidateRenderVariants();

    for (ReflectionProbe* probe : localReflectionProbes)
        UpdateProbe(probe);

    if (globalReflectionProbe != nullptr)
        UpdateProbe(globalReflectionProbe);
}

ReflectionRenderer::SphericalHarmonicsUpdate ReflectionRenderer::EnqueueSphericalHarmonicsUpdate(ReflectionProbe* probe)
{
    rhi::Texture::Descriptor shDesc;
    shDesc.autoGenMipmaps = false;
    shDesc.cpuAccessRead = true;
    shDesc.cpuAccessWrite = false;
    shDesc.isRenderTarget = true;
    shDesc.format = rhi::TEXTURE_FORMAT_RGBA32F;
    shDesc.width = 9;
    shDesc.height = 1;
    shDesc.levelCount = 1;
    shDesc.type = rhi::TextureType::TEXTURE_TYPE_2D;

    shUpdateQueue.emplace_back();
    SphericalHarmonicsUpdate& update = shUpdateQueue.back();
    update.targetTexture = rhi::CreateTexture(shDesc);
    update.probe = SafeRetain(probe);
    update.countdown = 4;
    return update;
}

void ReflectionRenderer::RetrieveUpdatedSphericalHarmonics()
{
    for (auto i = shUpdateQueue.begin(); i < shUpdateQueue.end();)
    {
        DVASSERT(i->countdown > 0);
        if (--i->countdown == 0)
        {
            void* ptr = rhi::MapTexture(i->targetTexture);
            i->probe->SetSphericalHarmonics(reinterpret_cast<Vector4*>(ptr));
            rhi::UnmapTexture(i->targetTexture);
            rhi::DeleteTexture(i->targetTexture);
            i = shUpdateQueue.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void ReflectionRenderer::SetDebugDrawProbe(ReflectionProbe* probe)
{
    debugDrawProbe = probe;
}
}
