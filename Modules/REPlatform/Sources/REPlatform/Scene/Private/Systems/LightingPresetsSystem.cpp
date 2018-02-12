#include "REPlatform/Scene/Systems/LightingPresetsSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"
#include "REPlatform/Global/StringConstants.h"

#include <FileSystem/FilePath.h>
#include <FileSystem/YamlEmitter.h>
#include <FileSystem/YamlNode.h>
#include <FileSystem/YamlParser.h>
#include <Entity/ComponentUtils.h>
#include <Render/DynamicBufferAllocator.h>
#include <Render/Highlevel/Light.h>
#include <Render/Highlevel/RenderPassNames.h>
#include <Render/Image/Image.h>
#include <Render/Image/ImageSystem.h>
#include <Render/RenderBase.h>
#include <Render/Texture.h>
#include <Render/TextureDescriptor.h>
#include <Scene3D/Components/LightComponent.h>
#include <Scene3D/Components/LightRenderComponent.h>
#include <Scene3D/Components/PostEffectComponent.h>
#include <Scene3D/Components/ReflectionComponent.h>
#include <Scene3D/Entity.h>

namespace DAVA
{
static const Array<uint32, 6> cubemapFaceRotations =
{
  90,
  90,
  270,
  90,
  180,
  0
};

static const Array<Vector3, 6> t0 =
{ {
{ 1.0, 0.0, 0.0 },
{ -1.0, 0.0, 0.0 },
{ 0.0, 1.0, 0.0 },
{ 0.0, -1.0, 0.0 },
{ 0.0, 0.0, 1.0 },
{ 0.0, 0.0, -1.0 }
} };

static const Array<Vector3, 3> t1 =
{ {
{ 0.0, 1.0, 0.0 },
{ 0.0, 0.0, 1.0 },
{ 1.0, 0.0, 0.0 },
} };

LightingPresetsSystem::LightingPresetsSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<LightComponent>() | ComponentUtils::MakeMask<PostEffectComponent>() | ComponentUtils::MakeMask<ReflectionComponent>())
{
}

LightingPresetsSystem::~LightingPresetsSystem()
{
    Scene* se = GetScene();
    if (se != nullptr)
    {
        RemoveEntityFromScene(editorSunLight);
        RemoveEntityFromScene(editorSky);
        RemoveEntityFromScene(editorSkyUniformColor);
        RemoveEntityFromScene(editorLightProbe);
        RemoveEntityFromScene(editorPosteffect);
    }
    SafeRelease(editorSunLight);
    SafeRelease(editorSky);
    SafeRelease(editorSkyUniformColor);
    SafeRelease(editorLightProbe);
    SafeRelease(editorPosteffect);
}

void LightingPresetsSystem::RegisterEntity(Entity* entity)
{
    const ComponentMask& reqiredComponents = GetRequiredComponents();
    if ((entity->GetAvailableComponentMask() & reqiredComponents) != 0)
    {
        AddEntity(entity);
    }
}

void LightingPresetsSystem::UnregisterEntity(Entity* entity)
{
    const ComponentMask& reqiredComponents = GetRequiredComponents();
    if ((entity->GetAvailableComponentMask() & reqiredComponents) != 0)
    {
        RemoveEntity(entity);
    }
}

void LightingPresetsSystem::RegisterComponent(Entity* entity, Component* component)
{
    const ComponentMask& reqiredComponents = GetRequiredComponents();
    if ((entity->GetAvailableComponentMask() & reqiredComponents) != 0)
    {
        AddComponent(entity, component);
    }
}

void LightingPresetsSystem::UnregisterComponent(Entity* entity, Component* component)
{
    const ComponentMask& reqiredComponents = GetRequiredComponents();
    if ((entity->GetAvailableComponentMask() & reqiredComponents) != 0)
    {
        RemoveComponent(entity, component);
    }
}

void LightingPresetsSystem::AddEntity(Entity* entity)
{
    LightComponent* light = entity->GetComponent<LightComponent>();
    if (light != nullptr)
        LightComponentAdded(entity, light);
    PostEffectComponent* post = entity->GetComponent<PostEffectComponent>();
    if (post != nullptr)
        PosteffectComponentAdded(entity, post);
    ReflectionComponent* refl = entity->GetComponent<ReflectionComponent>();
    if (refl != nullptr && refl->GetReflectionType() == ReflectionProbe::eType::TYPE_GLOBAL)
        ReflectionComponentAdded(entity, refl);
}

void LightingPresetsSystem::RemoveEntity(Entity* entity)
{
    LightComponent* light = entity->GetComponent<LightComponent>();
    if (light != nullptr)
        LightComponentRemoved(entity, light);
    PostEffectComponent* post = entity->GetComponent<PostEffectComponent>();
    if (post != nullptr)
        PosteffectComponentRemoved(entity, post);
    ReflectionComponent* refl = entity->GetComponent<ReflectionComponent>();
    if (refl != nullptr && refl->GetReflectionType() == ReflectionProbe::eType::TYPE_GLOBAL)
        ReflectionComponentRemoved(entity, refl);
}

void LightingPresetsSystem::AddComponent(Entity* entity, Component* component)
{
    LightComponent* light = dynamic_cast<LightComponent*>(component);
    if (light != nullptr)
        LightComponentAdded(entity, light);
    PostEffectComponent* post = dynamic_cast<PostEffectComponent*>(component);
    if (post != nullptr)
        PosteffectComponentAdded(entity, post);
    ReflectionComponent* refl = dynamic_cast<ReflectionComponent*>(component);
    if (refl != nullptr && refl->GetReflectionType() == ReflectionProbe::eType::TYPE_GLOBAL)
        ReflectionComponentAdded(entity, refl);
}

void LightingPresetsSystem::RemoveComponent(Entity* entity, Component* component)
{
    LightComponent* light = dynamic_cast<LightComponent*>(component);
    if (light != nullptr)
        LightComponentRemoved(entity, light);
    PostEffectComponent* post = dynamic_cast<PostEffectComponent*>(component);
    if (post != nullptr)
        PosteffectComponentRemoved(entity, post);
    ReflectionComponent* refl = dynamic_cast<ReflectionComponent*>(component);
    if (refl != nullptr && refl->GetReflectionType() == ReflectionProbe::eType::TYPE_GLOBAL)
        ReflectionComponentRemoved(entity, refl);
}

void LightingPresetsSystem::PrepareForRemove()
{
}

void LightingPresetsSystem::SavePreset(const FilePath& path)
{
    auto saveTexture = [&path, this](const FilePath& p) {
        if (p.IsEmpty() == false)
        {
            ScopedPtr<Texture> tex(Texture::CreateFromFile(p));
            SaveTexture(tex, path, p);
        }
    };

    YamlNode* rootYamlNode = new YamlNode(YamlNode::TYPE_MAP);
    YamlNode* yamlNode = new YamlNode(YamlNode::TYPE_MAP);
    if (scenePosteffect != nullptr)
    {
        scenePosteffect->SaveToYaml(path, rootYamlNode);
        saveTexture(scenePosteffect->GetColorGradingTable());
        saveTexture(scenePosteffect->GetHeatmapTable());
        saveTexture(scenePosteffect->GetLightMeterTable());
    }

    LightComponent* sceneSunLight = nullptr;
    LightComponent* sceneSky = nullptr;
    LightComponent* sceneSkyUniformColor = nullptr;

    FindLights(sceneSunLight, sceneSky, sceneSkyUniformColor);
    Texture* srcTexture = nullptr;
    FilePath envMapPath;
    if (sceneSky != nullptr && !sceneSky->GetEnvironmentMap().IsEmpty())
    {
        envMapPath = sceneSky->GetEnvironmentMap();
        srcTexture = Texture::CreateFromFile(sceneSky->GetEnvironmentMap());
        if (!srcTexture->GetDescriptor()->IsCubeMap())
            SaveTexture(srcTexture, path, envMapPath);
        else
            SaveCubemap(srcTexture, path, envMapPath);
    }
    else if (sceneLightProbe != nullptr && sceneSkyUniformColor == nullptr)
    {
        srcTexture = sceneLightProbe->GetReflectionProbe()->GetCurrentTexture();
        srcTexture->Retain();
        envMapPath = Format("cubeFromProbe%u.dds", rhi::Handle(srcTexture->handle));
        SaveCubemap(srcTexture, path, envMapPath);
    }

    if (sceneSky != nullptr)
        sceneSky->SaveToYaml(path, rootYamlNode, envMapPath);
    if (sceneSunLight != nullptr)
        sceneSunLight->SaveToYaml(path, rootYamlNode, FilePath());
    if (sceneSkyUniformColor != nullptr && sceneSky == nullptr)
        sceneSkyUniformColor->SaveToYaml(path, rootYamlNode, FilePath());
    YamlEmitter::SaveToYamlFile(path, rootYamlNode);

    SafeRelease(srcTexture);
    SafeRelease(rootYamlNode);
    SafeRelease(yamlNode);
}

void LightingPresetsSystem::LoadPreset(const FilePath& path)
{
    SceneEditor2* se = static_cast<SceneEditor2*>(GetScene());
    if (se == nullptr)
        return;

    ScopedPtr<YamlParser> parser(YamlParser::Create(path));
    YamlNode* rootNode = parser->GetRootNode();
    const YamlNode* postEffectNode = rootNode->Get("posteffect");
    if (postEffectNode != nullptr && scenePosteffect == nullptr)
    {
        RemoveEntityFromScene(editorPosteffect);
        if (editorPosteffect == nullptr)
        {
            editorPosteffect = new Entity();
            editorPosteffect->SetName(FastName(ResourceEditor::EDITOR_POSTEFFECT));
        }
        editorPosteffect->RemoveComponent<PostEffectComponent>();

        PostEffectComponent* posteffect = new PostEffectComponent();
        posteffect->LoadFromYaml(path, postEffectNode);
        editorPosteffect->AddComponent(posteffect);
        se->AddEditorEntity(editorPosteffect);
    }

    LightComponent* sceneSunLight = nullptr;
    LightComponent* sceneSky = nullptr;
    LightComponent* sceneSkyUniformColor = nullptr;

    FindLights(sceneSunLight, sceneSky, sceneSkyUniformColor);

    String sunNodeName = Format("light%d", static_cast<uint32>(Light::TYPE_SUN));
    const YamlNode* sunNode = rootNode->Get(sunNodeName);
    String skyNodeName = Format("light%d", static_cast<uint32>(Light::TYPE_ENVIRONMENT_IMAGE));
    const YamlNode* skyNode = rootNode->Get(skyNodeName);
    String sceneSkyUniformColorNodeName = Format("light%d", static_cast<uint32>(Light::TYPE_UNIFORM_COLOR));
    const YamlNode* uniformSkyNode = rootNode->Get(sceneSkyUniformColorNodeName);

    if (sunNode != nullptr && sceneSunLight == nullptr)
    {
        if (editorSunLight == nullptr)
        {
            editorSunLight = new Entity();
            editorSunLight->SetName(FastName(ResourceEditor::EDITOR_SUN_LIGHT));
        }
        RemoveEntityFromScene(editorSunLight);
        editorSunLight->RemoveComponent<LightComponent>();

        ScopedPtr<Light> light(new Light());
        light->LoadFromYaml(path, sunNode);
        editorSunLight->AddComponent(new LightComponent(light));
        se->AddEditorEntity(editorSunLight);
    }
    if (skyNode != nullptr && sceneSky == nullptr)
    {
        if (editorSky == nullptr)
        {
            editorSky = new Entity();
            editorSky->SetName(FastName(ResourceEditor::EDITOR_SKY));
        }
        RemoveEntityFromScene(editorSky);
        editorSky->RemoveComponent<LightComponent>();
        editorSky->RemoveComponent<LightRenderComponent>();

        ScopedPtr<Light> light(new Light());
        light->LoadFromYaml(path, skyNode);
        editorSky->AddComponent(new LightComponent(light));
        editorSky->AddComponent(new LightRenderComponent());
        se->AddEditorEntity(editorSky);
    }
    if (skyNode == nullptr && uniformSkyNode != nullptr && sceneSkyUniformColor == nullptr)
    {
        if (editorSkyUniformColor == nullptr)
        {
            editorSkyUniformColor = new Entity();
            editorSkyUniformColor->SetName(FastName(ResourceEditor::EDITOR_SKY));
        }
        RemoveEntityFromScene(editorSkyUniformColor);
        editorSkyUniformColor->RemoveComponent<LightComponent>();
        editorSkyUniformColor->RemoveComponent<LightRenderComponent>();

        ScopedPtr<Light> light(new Light());
        light->LoadFromYaml(path, uniformSkyNode);
        editorSkyUniformColor->AddComponent(new LightComponent(light));
        editorSkyUniformColor->AddComponent(new LightRenderComponent());
        se->AddEditorEntity(editorSkyUniformColor);
    }
    if (sceneLightProbe == nullptr)
    {
        if (editorLightProbe == nullptr)
        {
            editorLightProbe = new Entity();
            editorLightProbe->SetName(FastName(ResourceEditor::EDITOR_LIGHT_PROBE));
            editorLightProbe->SetLocalTransform(Matrix4::MakeTranslation({ 0.0f, 0.0f, 9999.0f }));
            se->AddEditorEntity(editorLightProbe);
        }
        RemoveEntityFromScene(editorLightProbe);
        editorLightProbe->RemoveComponent<ReflectionComponent>();
        editorLightProbe->AddComponent(new ReflectionComponent());
        se->AddEditorEntity(editorLightProbe);
    }
}

void LightingPresetsSystem::LightComponentAdded(Entity* entity, LightComponent* light)
{
    String name = entity->GetName().c_str();
    if (name.find("editor.") == 0)
        return;
    auto it = std::find(sceneLights.begin(), sceneLights.end(), light);
    if (it == sceneLights.end())
        sceneLights.push_back(light);

    if (light->GetLightType() == Light::TYPE_SUN)
        RemoveEntityFromScene(editorSunLight);
    else if (light->GetLightType() == Light::TYPE_ENVIRONMENT_IMAGE)
        RemoveEntityFromScene(editorSky);
    else if (light->GetLightType() == Light::TYPE_UNIFORM_COLOR)
        RemoveEntityFromScene(editorSkyUniformColor);
}

void LightingPresetsSystem::LightComponentRemoved(Entity* entity, LightComponent* light)
{
    String name = entity->GetName().c_str();
    if (name.find("editor.") == 0)
        return;
    auto it = std::find(sceneLights.begin(), sceneLights.end(), light);
    if (it != sceneLights.end())
        sceneLights.erase(it);
}

void LightingPresetsSystem::ReflectionComponentAdded(Entity* entity, ReflectionComponent* refl)
{
    if (entity == editorLightProbe)
        return;

    sceneLightProbe = refl;
    RemoveEntityFromScene(editorLightProbe);
}

void LightingPresetsSystem::ReflectionComponentRemoved(Entity* entity, ReflectionComponent* refl)
{
    if (entity == editorLightProbe)
        return;

    sceneLightProbe = nullptr;
}

void LightingPresetsSystem::PosteffectComponentAdded(Entity* entity, PostEffectComponent* posteffect)
{
    if (entity == editorPosteffect)
        return;

    scenePosteffect = posteffect;
    RemoveEntityFromScene(editorPosteffect);
}

void LightingPresetsSystem::PosteffectComponentRemoved(Entity* entity, PostEffectComponent* posteffect)
{
    if (entity == editorPosteffect)
        scenePosteffect = nullptr;
}

void LightingPresetsSystem::SaveCubemap(Texture* src, const FilePath& path, const FilePath& originalTexturePath)
{
    PixelFormat format = GetTextureFormat(src);
    Texture::FBODescriptor fboCfg = CreateFBODescriptor(format, src->GetWidth(), src->GetHeight());

    Vector<Texture*> cubemapFacesTextures;

    NMaterial* blitMaterial = new NMaterial();
    blitMaterial->SetFXName(NMaterialName::CUBE_FACE_BLIT);
    blitMaterial->AddTexture(FastName("texture0"), src);

    float32 v0 = -1.0f;
    float32 v1 = float32(src->height) / float32(src->height - 1.0f) * 2.0f - 1.0f;
    float32 u0 = -1.0f;
    float32 u1 = float32(src->width) / float32(src->width - 1.0f) * 2.0f - 1.0f;

    rhi::VertexLayout blitQuadLayout;
    blitQuadLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    blitQuadLayout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 3);
    uint32 layout = rhi::VertexLayout::UniqueId(blitQuadLayout);
    for (uint32 face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
    {
        cubemapFacesTextures.push_back(Texture::CreateFBO(fboCfg));

        Vector3 offset(0.0f, 0.0f, 0.0f); // GFX_COMPLETE
        // Deal with half-pixel offset on DX9
        // = rhi::DeviceCaps().isCenterPixelMapping ? Vector3(1.0f / fPageSize, 1.0f / fPageSize, 0.0f) : Vector3(0.0f, 0.0f, 0.0f);

        Vector3 a0 = t0[face];
        Vector3 a1 = t1[face / 2];
        Vector3 a2 = a0.CrossProduct(a1);

        Array<LightingPresetsSystem::CubemapPlaneVertex, 4> blitVBData;
        blitVBData[0] = { Vector3(-1.f, -1.f, .0f) + offset, Normalize(a0 + a1 * u1 + a2 * v0) };
        blitVBData[1] = { Vector3(-1.f, 1.f, .0f) + offset, Normalize(a0 + a1 * u1 + a2 * v1) };
        blitVBData[2] = { Vector3(1.f, -1.f, .0f) + offset, Normalize(a0 + a1 * u0 + a2 * v0) };
        blitVBData[3] = { Vector3(1.f, 1.f, .0f) + offset, Normalize(a0 + a1 * u0 + a2 * v1) };

        CopyTexture(src, cubemapFacesTextures[face], blitVBData, blitMaterial, layout);
    }
    SafeRelease(blitMaterial);
    Renderer::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(), [ cubemapFacesTextures, originalTexturePath, path, format, origDescr = TextureDescriptor(*src->GetDescriptor()) ](rhi::HSyncObject)
                                   {
                                       Vector<Image*> mipmapImages;
                                       for (uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
                                       {
                                           void* data = rhi::MapTexture(cubemapFacesTextures[i]->handle, 0, static_cast<rhi::TextureFace>(i));
                                           Image* image = Image::CreateFromData(cubemapFacesTextures[i]->width, cubemapFacesTextures[i]->height, cubemapFacesTextures[i]->GetFormat(), reinterpret_cast<uint8*>(data));
                                           rhi::UnmapTexture(cubemapFacesTextures[i]->handle);
                                           image->mipmapLevel = 0;
                                           image->cubeFaceID = i;
                                           mipmapImages.push_back(image);
                                       }

                                       FilePath dirPath = path.GetDirectory();
                                       FilePath tmp = originalTexturePath;
                                       tmp.ReplaceExtension(".dds");
                                       dirPath += tmp.GetFilename();
                                       FilePath origPath = dirPath;
                                       dirPath.TruncateExtension();

                                       String oldFilename = dirPath.GetFilename();

                                       for (uint32 i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
                                       {
                                           dirPath.ReplaceFilename(Format("%s%s.dds", oldFilename.c_str(), Texture::FACE_NAME_SUFFIX[i].c_str()));
                                           mipmapImages[i]->RotateDeg(cubemapFaceRotations[i]);
                                           ImageSystem::Save(dirPath, mipmapImages[i], format);
                                       }

                                       for (auto im : mipmapImages)
                                           SafeRelease(im);
                                       mipmapImages.clear();
                                       for (auto texture : cubemapFacesTextures)
                                           texture->Release();

                                       SaveDescriptor(origPath, format, origDescr.dataSettings.cubefaceFlags, true);
                                   });
}

void LightingPresetsSystem::SaveTexture(Texture* src, const FilePath& path, const FilePath& originalTexturePath)
{
    PixelFormat pformat = GetTextureFormat(src);

    Texture::FBODescriptor fboCfg = CreateFBODescriptor(pformat, src->GetWidth(), src->GetHeight());
    Texture* fboTexture = Texture::CreateFBO(fboCfg);

    NMaterial* blitMaterial = new NMaterial();
    blitMaterial->SetFXName(NMaterialName::TEXTURE_BLIT);

    blitMaterial->AddFlag(NMaterialFlagName::FLAG_TEXTURE_COUNT, 1);
    blitMaterial->AddTexture(FastName("texture0"), src);

    Vector3 offset(0.0f, 0.0f, 0.0f); // GFX_COMPLETE
    // Deal with half-pixel offset on DX9
    // = rhi::DeviceCaps().isCenterPixelMapping ? Vector3(1.0f / fPageSize, 1.0f / fPageSize, 0.0f) : Vector3(0.0f, 0.0f, 0.0f);

    Array<Vector3, 4> vertData;
    vertData[0] = Vector3(-1.f, -1.f, .0f) + offset;
    vertData[1] = Vector3(-1.f, 1.f, .0f) + offset;
    vertData[2] = Vector3(1.f, -1.f, .0f) + offset;
    vertData[3] = Vector3(1.f, 1.f, .0f) + offset;

    rhi::VertexLayout blitQuadLayout;
    blitQuadLayout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    CopyTexture(src, fboTexture, vertData, blitMaterial, rhi::VertexLayout::UniqueId(blitQuadLayout));

    SafeRelease(blitMaterial);

    Renderer::RegisterSyncCallback(rhi::GetCurrentFrameSyncObject(), [fboTexture, originalTexturePath, path, pformat](rhi::HSyncObject)
                                   {
                                       Image* image = fboTexture->CreateImageFromMemory();
                                       FilePath dirPath = path.GetDirectory();
                                       FilePath tmp = originalTexturePath;
                                       tmp.ReplaceExtension(".dds");
                                       dirPath += tmp.GetFilename();
                                       ImageSystem::Save(dirPath, image, fboTexture->GetFormat());
                                       fboTexture->Release();
                                       SafeRelease(image);

                                       SaveDescriptor(dirPath, pformat);
                                   });
}

void LightingPresetsSystem::FindLights(LightComponent*& sceneSunLight, LightComponent*& sceneSky, LightComponent*& sceneSkyUniformColor)
{
    for (LightComponent* light : sceneLights)
    {
        if (light->GetLightType() == Light::TYPE_SUN)
            sceneSunLight = light;
        else if (light->GetLightType() == Light::TYPE_ENVIRONMENT_IMAGE)
            sceneSky = light;
        else if (light->GetLightType() == Light::TYPE_UNIFORM_COLOR)
            sceneSkyUniformColor = light;
    }
}

void LightingPresetsSystem::SaveDescriptor(FilePath path, PixelFormat format, uint8 cubefaceFlags /*= -1u*/, bool isCubemap /*= false*/)
{
    TextureDescriptor descriptor;
    descriptor.pathname = path;
    descriptor.drawSettings.SetDefaultValues();
    descriptor.dataSettings.SetDefaultValues();

    descriptor.dataSettings.textureFlags = TextureDescriptor::TextureDataSettings::FLAG_DEFAULT;
    descriptor.dataSettings.sourceFileFormat = ImageFormat::IMAGE_FORMAT_DDS;
    descriptor.dataSettings.sourceFileExtension = ImageSystem::GetExtensionsFor(ImageFormat::IMAGE_FORMAT_DDS)[0];

    for (int32 i = 0; i < GPU_FAMILY_COUNT; ++i)
        descriptor.compression[i].Clear();
    descriptor.compression[eGPUFamily::GPU_ORIGIN].imageFormat = ImageFormat::IMAGE_FORMAT_DDS;
    descriptor.imageFormat = ImageFormat::IMAGE_FORMAT_DDS;
    descriptor.gpu = eGPUFamily::GPU_ORIGIN;
    descriptor.format = format;
    descriptor.isCompressedFile = false;

    if (isCubemap)
    {
        descriptor.dataSettings.cubefaceFlags = cubefaceFlags == 0 ? 0x3F : cubefaceFlags;
        for (int i = 0; i < Texture::CUBE_FACE_COUNT; ++i)
            descriptor.dataSettings.cubefaceExtensions[i] = ".dds";
    }

    path.ReplaceExtension(".tex");
    descriptor.Save(path);
}

PixelFormat LightingPresetsSystem::GetTextureFormat(Texture* src)
{
    if (src->GetFormat() == PixelFormat::FORMAT_RGBA8888)
        return PixelFormat::FORMAT_RGBA32F;
    return PixelFormat::FORMAT_RGBA32F;
}

Texture::FBODescriptor LightingPresetsSystem::CreateFBODescriptor(PixelFormat format, uint32 width, uint32 height)
{
    Texture::FBODescriptor fboCfg;
    fboCfg.width = width;
    fboCfg.height = height;
    fboCfg.sampleCount = 1;
    fboCfg.textureType = rhi::TEXTURE_TYPE_2D;
    fboCfg.format = format;
    fboCfg.needDepth = false;
    fboCfg.needPixelReadback = true;
    fboCfg.mipLevelsCount = 1;
    return fboCfg;
}

void LightingPresetsSystem::RemoveEntityFromScene(Entity* entity)
{
    if (entity != nullptr && entity->GetParent() != nullptr)
        entity->GetParent()->RemoveNode(entity);
}

template <typename T>
void LightingPresetsSystem::CopyTexture(Texture* src, Texture* dst, Array<T, 4> vertData, NMaterial* material, uint32 layout)
{
    rhi::RenderPassConfig passConfig;
    passConfig.name = "Blit";
    passConfig.priority = PRIORITY_SERVICE_3D + 20;
    passConfig.colorBuffer[0].texture = dst->handle;
    passConfig.colorBuffer[0].loadAction = rhi::LOADACTION_LOAD;
    passConfig.colorBuffer[0].storeAction = rhi::STOREACTION_STORE;
    passConfig.colorBuffer[0].textureLevel = 0;

    passConfig.depthStencilBuffer.texture = rhi::InvalidHandle;
    passConfig.viewport.x = 0;
    passConfig.viewport.y = 0;
    passConfig.viewport.width = src->width;
    passConfig.viewport.height = src->height;

    rhi::Packet blitPacket;
    blitPacket.vertexStreamCount = 1;
    blitPacket.vertexCount = 4;
    blitPacket.primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    blitPacket.primitiveCount = 2;
    blitPacket.options |= rhi::Packet::OPT_OVERRIDE_SCISSOR;
    blitPacket.vertexLayoutUID = layout;

    if (material->PreBuildMaterial(PASS_FORWARD))
        material->BindParams(blitPacket);

    blitPacket.scissorRect.x = uint16(passConfig.viewport.x);
    blitPacket.scissorRect.y = uint16(passConfig.viewport.y);
    blitPacket.scissorRect.width = uint16(passConfig.viewport.width);
    blitPacket.scissorRect.height = uint16(passConfig.viewport.height);

    DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(sizeof(T), 4);
    blitPacket.vertexStream[0] = vb.buffer;
    blitPacket.baseVertex = vb.baseVertex;
    blitPacket.vertexCount = vb.allocatedVertices;
    blitPacket.indexBuffer = DynamicBufferAllocator::AllocateQuadListIndexBuffer(1);
    T* blitVBData = reinterpret_cast<T*>(vb.data);
    blitVBData[0] = vertData[0];
    blitVBData[1] = vertData[1];
    blitVBData[2] = vertData[2];
    blitVBData[3] = vertData[3];

    rhi::HPacketList packetList;
    rhi::HRenderPass pass = rhi::AllocateRenderPass(passConfig, 1, &packetList);

    rhi::BeginRenderPass(pass);
    rhi::BeginPacketList(packetList);

    rhi::AddPackets(packetList, &blitPacket, 1);

    rhi::EndPacketList(packetList);
    rhi::EndRenderPass(pass);
}
} // namespace DAVA
