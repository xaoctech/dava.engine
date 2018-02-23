#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
#define IMPL_LIGHT_PROXY(method, ...) .Field(#method, &LightComponent::Get##method, &LightComponent::Set##method)[M::DisplayName(#method) __VA_ARGS__]
#define IMPL_LIGHT_PROXY_NAME(method, name, ...) .Field(#method, &LightComponent::Get##method, &LightComponent::Set##method)[M::DisplayName(name) __VA_ARGS__]

DAVA_VIRTUAL_REFLECTION_IMPL(LightComponent)
{
    ReflectionRegistrator<LightComponent>::Begin()
    .ConstructorByPointer()
    IMPL_LIGHT_PROXY(IsDynamic)
    IMPL_LIGHT_PROXY(CastsShadow)
    IMPL_LIGHT_PROXY(LightType, , M::EnumT<Light::eType>())
    IMPL_LIGHT_PROXY(Color)
    IMPL_LIGHT_PROXY(Position)
    IMPL_LIGHT_PROXY(Direction)
    IMPL_LIGHT_PROXY(Radius)
    IMPL_LIGHT_PROXY(AORadius)
    IMPL_LIGHT_PROXY(EnvironmentMap)
    IMPL_LIGHT_PROXY(ColorTemperature)
    IMPL_LIGHT_PROXY(ShadowCascadesIntervals)
    IMPL_LIGHT_PROXY(ShadowFilterRadius)
    IMPL_LIGHT_PROXY(ShadowWriteBias)
    IMPL_LIGHT_PROXY(DebugDrawShadowMapEnabled)
    IMPL_LIGHT_PROXY(DebugDrawFrustumsEnabled)
    IMPL_LIGHT_PROXY_NAME(AutoColor, "Auto Sun Color")
    .End();
}
#undef IMPL_LIGHT_PROXY

LightComponent::LightComponent(Light* _light)
    : lightMaterial(nullptr)
{
    light = SafeRetain(_light);
}

LightComponent::~LightComponent()
{
    SafeRelease(light);
}

void LightComponent::SetLightObject(Light* _light)
{
    SafeRelease(light);
    light = SafeRetain(_light);
}

Light* LightComponent::GetLightObject() const
{
    return light;
}

Component* LightComponent::Clone(Entity* toEntity)
{
    LightComponent* component = new LightComponent();
    component->SetEntity(toEntity);

    if (light)
        component->light = static_cast<Light*>(light->Clone());

    return component;
}

void LightComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (NULL != archive && NULL != light)
    {
        KeyedArchive* lightArch = new KeyedArchive();
        light->Save(lightArch, serializationContext);

        archive->SetArchive("lc.light", lightArch);

        lightArch->Release();
    }
}

void LightComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        KeyedArchive* lightArch = archive->GetArchive("lc.light");
        if (NULL != lightArch)
        {
            Light* l = new Light();
            l->Load(lightArch, serializationContext);
            SetLightObject(l);
            l->Release();
        }
    }

    Component::Deserialize(archive, serializationContext);
}

void LightComponent::SaveToYaml(const FilePath& presetPath, YamlNode* parentNode, const FilePath& realEnvMapPath)
{
    light->SaveToYaml(presetPath, parentNode, realEnvMapPath);
}

void LightComponent::LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode)
{
    light->LoadFromYaml(presetPath, parentNode);
}

void LightComponent::NotifyRenderSystemLightChanged()
{
    if (entity)
    {
        Scene* curScene = entity->GetScene();
        if (curScene)
        {
            curScene->renderSystem->SetForceUpdateLights();
        }
        GlobalEventSystem::Instance()->Event(this, EventSystem::LIGHT_PROPERTIES_CHANGED);
    }
}

/*
 * Deprecated stuff
 */
const Color LightComponent::GetAmbientColor()
{
#if (GFX_COMPLETE)
    if (light)
    {
        return light->GetAmbientColor();
    }
#endif
    return Color();
}

const float32 LightComponent::GetIntensity()
{
#if (GFX_COMPLETE)
    if (light)
    {
        return light->GetIntensity();
    }
#endif
    return 0.0f;
}

void LightComponent::SetAmbientColor(const Color& _color)
{
#if (GFX_COMPLETE)
    if (light)
    {
        light->SetAmbientColor(_color);
        NotifyRenderSystemLightChanged();
    }
#endif
}

void LightComponent::SetIntensity(const float32& intensity)
{
#if (GFX_COMPLETE)
    if (light)
    {
        light->SetIntensity(intensity);
        NotifyRenderSystemLightChanged();
    }
#endif
}

void LightComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    if (lightMaterial.get() == nullptr)
    {
        lightMaterial = new NMaterial();
        lightMaterial->SetMaterialName(FastName("LightComponent_Helper_Material"));
        lightMaterial->SetRuntime(true);
    }

    if (lightMaterial->HasLocalTexture(NMaterialTextureName::TEXTURE_ALBEDO))
        lightMaterial->RemoveTexture(NMaterialTextureName::TEXTURE_ALBEDO);

    if ((light->GetLightType() == Light::eType::TYPE_ENVIRONMENT_IMAGE) &&
        FileSystem::Instance()->Exists(light->GetEnvironmentMap()))
    {
        ScopedPtr<Texture> tex(Texture::CreateFromFile(light->GetEnvironmentMap()));
        lightMaterial->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, tex);
    }

    dataNodes.insert(lightMaterial);
}
};
