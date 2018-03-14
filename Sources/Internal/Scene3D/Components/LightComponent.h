#pragma once

#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Entity/Component.h"
#include "Render/Highlevel/Light.h"
#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
#define IMPL_LIGHT_PROXY(TYPE, NAME) TYPE Get##NAME() const { return (light == nullptr) ? TYPE() : light->Get##NAME(); } \
void Set##NAME(const TYPE& value) { if (light) { light->Set##NAME(value); NotifyRenderSystemLightChanged(); } }

class LightComponent : public Component
{
protected:
    ~LightComponent();

public:
    LightComponent(Light* _light = 0);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SaveToYaml(const FilePath& presetPath, YamlNode* parentNode, const FilePath& realEnvMapPath);
    void LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode);

    void SetLightObject(Light* _light);
    Light* GetLightObject() const;

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

    /*
     * Proxy functions to Get/Set light properties
     */
    IMPL_LIGHT_PROXY(bool, IsDynamic);
    IMPL_LIGHT_PROXY(bool, CastsShadow);
    IMPL_LIGHT_PROXY(Light::eType, LightType);
    IMPL_LIGHT_PROXY(Color, Color);
    IMPL_LIGHT_PROXY(Vector3, Position);
    IMPL_LIGHT_PROXY(Vector3, Direction);
    IMPL_LIGHT_PROXY(float32, Radius);
    IMPL_LIGHT_PROXY(float32, AORadius);
    IMPL_LIGHT_PROXY(FilePath, EnvironmentMap);
    IMPL_LIGHT_PROXY(float, ColorTemperature);
    IMPL_LIGHT_PROXY(float, ShadowCascadesIntervals1);
    IMPL_LIGHT_PROXY(Vector2, ShadowCascadesIntervals2);
    IMPL_LIGHT_PROXY(Vector3, ShadowCascadesIntervals3);
    IMPL_LIGHT_PROXY(Vector4, ShadowCascadesIntervals4);
    IMPL_LIGHT_PROXY(Vector2, ShadowFilterRadius);
    IMPL_LIGHT_PROXY(Vector2, ShadowWriteBias);
    IMPL_LIGHT_PROXY(bool, DebugDrawShadowMapEnabled);
    IMPL_LIGHT_PROXY(bool, DebugDrawFrustumsEnabled);
    IMPL_LIGHT_PROXY(bool, AutoColor);

private:
    DAVA_DEPRECATED(void SetAmbientColor(const Color& _color));
    DAVA_DEPRECATED(void SetIntensity(const float32& intensity));
    DAVA_DEPRECATED(const Color GetAmbientColor());
    DAVA_DEPRECATED(const float32 GetIntensity());

private:
    void NotifyRenderSystemLightChanged();

private:
    Light* light = nullptr;
    ScopedPtr<NMaterial> lightMaterial;
    DAVA_VIRTUAL_REFLECTION(LightComponent, Component);
};

#undef IMPL_LIGHT_PROXY
};
