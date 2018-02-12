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
const uint32 CONVOLUTION_MIP_COUNT = 8;
const uint32 ReflectionRenderer::maxCacheCubemapOnEachLevel[CUBEMAP_QUALITY_LEVELS] = { 4, 8, 16, 32 };
const uint32 ReflectionRenderer::cacheCubemapFaceSize[CUBEMAP_QUALITY_LEVELS] = { 256, 128, 64, 32 };
const FastName DISPLAY_SPHERICAL_HARMONICS = FastName("DISPLAY_SPHERICAL_HARMONICS");
const FastName DISPLAY_INDIRECT_LOOKUP = FastName("DISPLAY_INDIRECT_LOOKUP");
const FastName GLOBAL_SPHERICAL_HARMONICS = FastName("sphericalHarmonics");

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

    reflectionMainFBO = Texture::CreateFBO(HIGH_RES_FBO_CUBEMAP_SIZE, HIGH_RES_FBO_CUBEMAP_SIZE, PixelFormat::FORMAT_RGBA16F, true, rhi::TEXTURE_TYPE_CUBE);
    reflectionMainFBO->SetMinMagFilter(rhi::TEXFILTER_LINEAR, rhi::TEXFILTER_LINEAR, rhi::TEXMIPFILTER_NONE);

    reflectionTemporaryFBO = CreateCubeTextureForReflection(HIGH_RES_FBO_CUBEMAP_SIZE, CONVOLUTION_MIP_COUNT);
    globalProbeSpecularConvolution = CreateCubeTextureForReflection(HIGH_RES_FBO_CUBEMAP_SIZE, CONVOLUTION_MIP_COUNT, PixelFormat::FORMAT_R11G11B10F);

    for (uint32 qualityLevel = 0; qualityLevel < CUBEMAP_QUALITY_LEVELS; ++qualityLevel)
    {
        for (uint32 t = 0; t < maxCacheCubemapOnEachLevel[qualityLevel]; ++t)
        {
            Texture* texture = CreateCubeTextureForReflection(cacheCubemapFaceSize[qualityLevel], FastLog2(cacheCubemapFaceSize[qualityLevel]));
            textureCache[qualityLevel].push_back(texture);
            allCacheTextures.push_back(texture);
        }
    }

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
    sphericalHarmonicsTexture = rhi::CreateTexture(shDesc);

    float level = 0.0f;
    Vector2 defaultTexSize{ 2048.f, 2048.f };
    Vector2 defaultTexOffset{ 0, 0 };
    debugMaterial->SetFXName(FastName("~res:/Materials2/CubemapDebug.material"));
    debugMaterial->AddProperty(FastName("srcRectOffset"), defaultTexOffset.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("srcRectSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("srcTexSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("destTexSize"), defaultTexSize.data, rhi::ShaderProp::Type::TYPE_FLOAT2);
    debugMaterial->AddProperty(FastName("sampledLevel"), &level, rhi::ShaderProp::Type::TYPE_FLOAT1);
    debugMaterial->AddProperty(GLOBAL_SPHERICAL_HARMONICS, globalDiffuseSphericalHarmonics->data, rhi::ShaderProp::TYPE_FLOAT4, 9);
    debugMaterial->AddFlag(DISPLAY_SPHERICAL_HARMONICS, 0);
    debugMaterial->AddFlag(DISPLAY_INDIRECT_LOOKUP, 0);

    invalidateCallback = NMaterialManager::Instance().RegisterInvalidateCallback([this]() {
        if (globalReflectionProbe != nullptr)
            UpdateProbe(globalReflectionProbe);
    });
}

ReflectionRenderer::~ReflectionRenderer()
{
    NMaterialManager::Instance().UnregisterInvalidateCallback(invalidateCallback);

    for (size_t k = 0; k < allCacheTextures.size(); ++k)
        SafeRelease(allCacheTextures[k]);

    SafeRelease(globalProbeSpecularConvolution);
    SafeRelease(reflectionTemporaryFBO);
    SafeRelease(reflectionMainFBO);
    SafeDelete(reflectionPass);
    rhi::DeleteTexture(sphericalHarmonicsTexture);
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

void ReflectionRenderer::RegisterReflectionProbe(ReflectionProbe* reflectionProbe)
{
    DVASSERT(reflectionProbe != nullptr);
    reflectionProbe->Retain();

    if (reflectionProbe->GetReflectionType() == ReflectionProbe::TYPE_GLOBAL && globalReflectionProbe == nullptr)
    {
        globalReflectionProbe = reflectionProbe;
    }
    else
    {
        // put only local probes to array
        reflectionProbeArray.push_back(reflectionProbe);
    }
}

void ReflectionRenderer::UnregisterReflectionProbe(ReflectionProbe* reflectionProbe)
{
    DVASSERT(reflectionProbe != nullptr);

    if (globalReflectionProbe == reflectionProbe)
    {
        globalReflectionProbe = nullptr;
    }
    FindAndRemoveExchangingWithLast(reflectionProbeArray, reflectionProbe);
    reflectionProbe->Release();
}

// GFX_COMPLETE:
// 1. Probe moves / Objects move / Dynamic objects ?
// 2. When we move probe we assign it to objects that are fall into it's influence

void ReflectionRenderer::UpdateProbe(ReflectionProbe* probe)
{
    auto i = std::find_if(probesForUpdate.begin(), probesForUpdate.end(), [probe](const ReflectionProbeToUpdate& p) {
        return (p.reflectionProbe == probe);
    });

    if (i == probesForUpdate.end())
        probesForUpdate.emplace(probe, 100);
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

void ReflectionRenderer::PrerenderReflections(ReflectionProbe* probe, uint32 layerFilter)
{
    CubemapRenderer* cmr = renderSystem->GetCubemapRenderer();
    cmr->RenderCubemap(renderSystem, reflectionPass, probe->GetPosition() + probe->GetCapturePosition(), reflectionMainFBO->handle,
                       reflectionMainFBO->handleDepthStencil, reflectionMainFBO->GetWidth(), reflectionMainFBO->GetHeight(), layerFilter);
    cmr->EdgeFilterCubemap(reflectionMainFBO, reflectionTemporaryFBO, CONVOLUTION_MIP_COUNT);
}

void ReflectionRenderer::GenerateDiffuseProbe(ReflectionProbe* probe)
{
    CubemapRenderer* cmr = renderSystem->GetCubemapRenderer();
    cmr->ConvoluteSphericalHarmonics(reflectionTemporaryFBO, sphericalHarmonicsTexture);
    sphericalHarmonicsGrabCountdown = 4;
}

void ReflectionRenderer::GenerateReflectionProbe(ReflectionProbe* probe, Texture* reflectionTextureTarget)
{
    // TODO: fix generation to make it correct for probes with different mip levels
    // Here we've generated everything for correct convoluted reflection
    CubemapRenderer* cmr = renderSystem->GetCubemapRenderer();
    cmr->ConvoluteSpecularCubemap(reflectionTemporaryFBO, reflectionTextureTarget, CONVOLUTION_MIP_COUNT);
}

void ReflectionRenderer::AllocateTextureFromCacheAndRender(ReflectionProbe* probe)
{
    if ((probe->GetCurrentTexture() == globalProbeSpecularConvolution) || (probe->GetCurrentTexture() == nullptr))
    {
        uint32 qualityLevel = probe->GetNextQualityLevel();
        DVASSERT(qualityLevel < CUBEMAP_QUALITY_LEVELS);

        Texture* texture = textureCache[qualityLevel].front();
        textureCache[qualityLevel].pop_front();

        probe->SetCurrentTexture(texture);
        probe->SetActiveQualityLevel(probe->GetNextQualityLevel());
    }

    Texture* texture = probe->GetCurrentTexture();
    DVASSERT(texture != nullptr);
    GenerateReflectionProbe(probe, texture);
}

void ReflectionRenderer::ReleaseTextureToCache(ReflectionProbe* probe)
{
    Texture* probeTexture = probe->GetCurrentTexture();
    if (!probeTexture)
        return;
    uint32 probeQualityLevel = probe->GetActiveQualityLevel();
    probe->SetCurrentTexture(globalProbeSpecularConvolution);
}

void ReflectionRenderer::UpdateProbeMaterialBindings(ReflectionProbe* probe)
{
    // AABBox3 probeGlobalBox
    if (probe == globalReflectionProbe)
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
    reflectionPass->SetName(Renderer::GetCurrentRenderFlow() == RenderFlow::LDRForward ? PASS_IB_REFLECTION_LDR : PASS_IB_REFLECTION);

    for (Landscape* landscape : renderSystem->GetLandscapes())
        landscape->SetPageUpdateLocked(true);

    SCOPE_EXIT
    {
        for (Landscape* landscape : renderSystem->GetLandscapes())
            landscape->SetPageUpdateLocked(false);
    };

#if 1
    // Find
    const Vector3& cameraPosition = camera->GetPosition();

    auto sortFunction = [&](const ReflectionProbe* probeA, const ReflectionProbe* probeB) -> bool
    {
        float32 squareDistanceA = (probeA->GetPosition() - cameraPosition).SquareLength();
        float32 squareDistanceB = (probeB->GetPosition() - cameraPosition).SquareLength();
        return squareDistanceA < squareDistanceB;
    };

    std::sort(reflectionProbeArray.begin(), reflectionProbeArray.end(), sortFunction);

    /*
        1. Найти ближайшие к камере
        2. Определить сколько из них уже в приемлимом разрешении.
        3. Поставить в очередь для обновлений (без дубликатов, и чтобы не происходило лишней работы)
     */

    uint32 size = static_cast<uint32>(reflectionProbeArray.size());
    uint32 cubemapQualityLevel = 0;
    uint32 cubemapsOnEachLevel[CUBEMAP_QUALITY_LEVELS];
    for (uint32 k = 0; k < CUBEMAP_QUALITY_LEVELS; ++k)
        cubemapsOnEachLevel[k] = maxCacheCubemapOnEachLevel[k];

    uint32 k;
    for (k = 0; k < size; ++k)
    {
        if (cubemapsOnEachLevel[cubemapQualityLevel] > 0)
        {
            cubemapsOnEachLevel[cubemapQualityLevel]--;
            reflectionProbeArray[k]->SetNextQualityLevel(cubemapQualityLevel);
        }
        else
        {
            cubemapQualityLevel++;
            if (cubemapQualityLevel >= CUBEMAP_QUALITY_LEVELS)
            {
                break;
            }
            k--;
        }
    }
    while (k++ < size)
    {
        reflectionProbeArray[k]->SetNextQualityLevel(ReflectionProbe::INVALID_QUALITY_LEVEL);
    }

    //
    for (k = 0; k < size; ++k)
    {
        if (reflectionProbeArray[k]->GetNextQualityLevel() < reflectionProbeArray[k]->GetActiveQualityLevel())
        {
            ReleaseTextureToCache(reflectionProbeArray[k]);
            UpdateProbe(reflectionProbeArray[k]);
            Logger::FrameworkDebug("Increase quality of probe: %d to level %d", k, reflectionProbeArray[k]->GetNextQualityLevel());
        }
        else if (reflectionProbeArray[k]->GetNextQualityLevel() > reflectionProbeArray[k]->GetActiveQualityLevel())
        {
            if (reflectionProbeArray[k]->GetNextQualityLevel() == 0xFFFFFFFF)
            {
                ReleaseTextureToCache(reflectionProbeArray[k]);
                UpdateProbe(reflectionProbeArray[k]);
                Logger::FrameworkDebug("Decrease quality of probe: %d to global probe", k);
            }
            else
            {
                ReleaseTextureToCache(reflectionProbeArray[k]);
                UpdateProbe(reflectionProbeArray[k]);
                Logger::FrameworkDebug("Decrease quality of probe: %d to level %d", reflectionProbeArray[k]->GetNextQualityLevel());
            }
        }
    }
#endif

    uint32 counter = MAX_NUMBER_OF_PROBES_TO_UPDATE_PER_FRAME;
    while (probesForUpdate.size() > 0 && counter-- > 0)
    {
        const ReflectionProbeToUpdate& probeWithPriority = (*probesForUpdate.begin());
        ReflectionProbe* reflectionProbe = probeWithPriority.reflectionProbe;

        if (globalReflectionProbe == reflectionProbe)
        {
            // Here we generated only sky ambient texture
            PrerenderReflections(reflectionProbe, 0xffffffff);
            GenerateDiffuseProbe(reflectionProbe);
            GenerateReflectionProbe(reflectionProbe, globalProbeSpecularConvolution);
            reflectionProbe->SetCurrentTexture(globalProbeSpecularConvolution);
            UpdateProbeMaterialBindings(reflectionProbe);
        }
        else
        {
            AllocateTextureFromCacheAndRender(reflectionProbe);
            UpdateProbeMaterialBindings(reflectionProbe);
        }

        probesForUpdate.erase(probesForUpdate.begin());
    }

    if ((sphericalHarmonicsGrabCountdown > 0) && (--sphericalHarmonicsGrabCountdown == 0))
    {
        sphericalHarmonicsGrabCountdown = -1;
        void* ptr = rhi::MapTexture(sphericalHarmonicsTexture);
        memcpy(globalDiffuseSphericalHarmonics, ptr, sizeof(globalDiffuseSphericalHarmonics));
        rhi::UnmapTexture(sphericalHarmonicsTexture);
        /*
        DAVA::Logger::Info("Spherical harmonics:");
        for (uint32_t i = 0; i < 9; ++i)
        {
            DAVA::Logger::Info("globalDiffuseSphericalHarmonics[%u] = Vector4(%ff, %ff, %ff, 1.0f);", i,
                               globalDiffuseSphericalHarmonics[i].x, globalDiffuseSphericalHarmonics[i].y, globalDiffuseSphericalHarmonics[i].z);
            
            if (!std::isfinite(globalDiffuseSphericalHarmonics[i].x) || std::isnan(globalDiffuseSphericalHarmonics[i].x))
                globalDiffuseSphericalHarmonics[i].x = 0.0f;
            
            if (!std::isfinite(globalDiffuseSphericalHarmonics[i].y) || std::isnan(globalDiffuseSphericalHarmonics[i].y))
                globalDiffuseSphericalHarmonics[i].y = 0.0f;
            
            if (!std::isfinite(globalDiffuseSphericalHarmonics[i].z) || std::isnan(globalDiffuseSphericalHarmonics[i].z))
                globalDiffuseSphericalHarmonics[i].z = 0.0f;
        }
        // */
    }

    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_DIFFUSE_SPHERICAL_HARMONICS, globalDiffuseSphericalHarmonics, reinterpret_cast<pointer_size>(globalDiffuseSphericalHarmonics));

    if (debugDrawEnabled)
    {
        float32 gap = 5.0f;
        float32 levelsPerColumn = 5.0f;
        float32 debugRenderHeight = (static_cast<float>(Renderer::GetFramebufferHeight()) - (1.0f + levelsPerColumn) * gap) / levelsPerColumn;
        float32 debugRenderWidth = 2.0f * debugRenderHeight;

        const FastName environnmentTextureName("environmentMap");
        debugMaterial->SetFlag(DISPLAY_INDIRECT_LOOKUP, 0);
        debugMaterial->SetFlag(DISPLAY_SPHERICAL_HARMONICS, 0);

        if (debugMaterial->HasLocalTexture(environnmentTextureName))
            debugMaterial->SetTexture(environnmentTextureName, reflectionMainFBO);
        else
            debugMaterial->AddTexture(environnmentTextureName, reflectionMainFBO);

        float32 xPos = gap;
        float32 yPos = gap;

        float32 sampledLevel = 0.0f;
        debugMaterial->SetPropertyValue(FastName("sampledLevel"), &sampledLevel);

        QuadRenderer::Options options;
        options.loadAction = rhi::LOADACTION_LOAD;
        options.material = debugMaterial;
        options.srcTexture = debugMaterial->GetEffectiveTexture(environnmentTextureName)->handle;
        options.srcTexSize = Vector2(1.0f, 1.0f);
        options.srcRect = Rect2f(0.0f, 0.0f, 1.0f, 1.0f);
        options.dstTexture = rhi::HTexture();
        options.dstTexSize = Vector2(static_cast<float>(Renderer::GetFramebufferWidth()), static_cast<float>(Renderer::GetFramebufferHeight()));
        options.renderPassPriority = -200;
        options.dstRect = Rect2f(xPos, yPos, debugRenderWidth, debugRenderHeight);
        quadRenderer.Render(options);

        auto advancePosition = [&xPos, &yPos, gap, debugRenderHeight, debugRenderWidth, &options]() {
            yPos += debugRenderHeight + gap;
            if (yPos >= options.dstTexSize.y)
            {
                yPos = gap;
                xPos += gap + debugRenderWidth;
            }
        };

        debugMaterial->SetTexture(environnmentTextureName, globalProbeSpecularConvolution);
        options.srcTexture = debugMaterial->GetEffectiveTexture(environnmentTextureName)->handle;
        for (; sampledLevel < static_cast<float32>(CONVOLUTION_MIP_COUNT); sampledLevel += 1.0f)
        {
            advancePosition();

            debugMaterial->SetPropertyValue(FastName("sampledLevel"), &sampledLevel);
            options.dstRect = Rect2f(xPos, yPos, debugRenderWidth, debugRenderHeight);
            quadRenderer.Render(options);
        }

        {
            advancePosition();

            debugMaterial->SetFlag(DISPLAY_INDIRECT_LOOKUP, 0);
            debugMaterial->SetFlag(DISPLAY_SPHERICAL_HARMONICS, 1);
            debugMaterial->SetPropertyValue(GLOBAL_SPHERICAL_HARMONICS, globalDiffuseSphericalHarmonics->data);
            options.dstRect = Rect2f(xPos, yPos, debugRenderWidth, debugRenderHeight);
            quadRenderer.Render(options);
        }

        {
            advancePosition();

            debugMaterial->SetFlag(DISPLAY_INDIRECT_LOOKUP, 1);
            debugMaterial->SetFlag(DISPLAY_SPHERICAL_HARMONICS, 0);
            options.dstRect = Rect2f(xPos, yPos, std::max(debugRenderHeight, debugRenderWidth), std::max(debugRenderHeight, debugRenderWidth));
            options.textureSet = RhiUtils::FragmentTextureSet({ Renderer::GetRuntimeTextures().GetRuntimeTexture(RuntimeTextures::TEXTURE_INDIRECT_SPECULAR_LOOKUP) });
            quadRenderer.Render(options);
        }
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

    for (ReflectionProbe* probe : reflectionProbeArray)
        UpdateProbe(probe);
}

void ReflectionRenderer::UpdateGlobalLightProbe()
{
    if (globalReflectionProbe != nullptr)
    {
        UpdateProbe(globalReflectionProbe);
    }
}
}
