#pragma once

#include "REPlatform/Scene/Systems/EditorSceneSystem.h"

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Math/Vector.h>
#include <Render/RenderBase.h>
#include <Render/Texture.h>

namespace DAVA
{
class Entity;
class LightComponent;
class ReflectionComponent;
class PostEffectComponent;
class NMaterial;
class FilePath;

class LightingPresetsSystem : public SceneSystem, public EditorSceneSystem
{
public:
    LightingPresetsSystem(Scene* scene);
    ~LightingPresetsSystem() override;

    void RegisterEntity(Entity* entity) override;
    void UnregisterEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void AddComponent(Entity* entity, Component* component) override;
    void RemoveComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

    void SavePreset(const FilePath& path);
    void LoadPreset(const FilePath& path);

private:
    struct CubemapPlaneVertex
    {
        Vector3 position;
        Vector3 uv;
    };

    void LightComponentAdded(Entity* entity, LightComponent* light);
    void LightComponentRemoved(Entity* entity, LightComponent* light);
    void ReflectionComponentAdded(Entity* entity, ReflectionComponent* refl);
    void ReflectionComponentRemoved(Entity* entity, ReflectionComponent* refl);
    void PosteffectComponentAdded(Entity* entity, PostEffectComponent* posteffect);
    void PosteffectComponentRemoved(Entity* entity, PostEffectComponent* posteffect);
    void SaveCubemap(Texture* src, const FilePath& path, const FilePath& originalTexturePath);
    void SaveTexture(Texture* src, const FilePath& path, const FilePath& originalTexturePath);
    void FindLights(LightComponent*& sceneSunLight, LightComponent*& sceneSky, LightComponent*& sceneSkyUniformColor);

    static void SaveDescriptor(FilePath path, PixelFormat format, uint8 cubefaceFlags = 0, bool isCubemap = false);

    template <typename T>
    void CopyTexture(Texture* src, Texture* dst, Array<T, 4> vertData, NMaterial* material, uint32 layout);

    PixelFormat GetTextureFormat(Texture* src);
    Texture::FBODescriptor CreateFBODescriptor(PixelFormat format, uint32 width, uint32 height);

    void RemoveEntityFromScene(Entity* entity);

    Vector<LightComponent*> sceneLights;
    ReflectionComponent* sceneLightProbe = nullptr;
    PostEffectComponent* scenePosteffect = nullptr;

    Entity* editorSunLight = nullptr;
    Entity* editorSky = nullptr;
    Entity* editorSkyUniformColor = nullptr;
    Entity* editorLightProbe = nullptr;
    Entity* editorPosteffect = nullptr;
};
} // namespace DAVA
