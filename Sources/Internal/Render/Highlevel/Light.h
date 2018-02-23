#pragma once

#include "Base/BaseMath.h"
#include "Reflection/Reflection.h"
#include "Render/RenderBase.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Highlevel/RenderObject.h"

//default direction (with identity matrix) is -y
namespace DAVA
{
class SceneFileV2;
class Camera;

class Light : public RenderObject
{
public:
    enum eType : uint32
    {
        TYPE_SUN = 0,
        TYPE_SPOT,
        TYPE_POINT,
        TYPE_UNIFORM_COLOR,
        TYPE_DEPRECATED_AMBIENT,
        TYPE_ENVIRONMENT_IMAGE,
        TYPE_ATMOSPHERE,

        TYPE_COUNT,

        // support legacy naming
        TYPE_SKY = TYPE_UNIFORM_COLOR,
        TYPE_DIRECTIONAL = TYPE_SUN
    };

    enum eFlags : uint32
    {
        DYNAMIC_LIGHT = 1 << 0,
        CASTS_SHADOW = 1 << 1,
    };

public:
    Light();
    RenderObject* Clone(RenderObject* dstNode = nullptr) override;

    void SetLightType(eType type);
    eType GetLightType() const;

    void SetColor(const Color& color);
    const Color& GetColor() const;

    void SetColorTemperature(float);
    float32 GetColorTemperature() const;

    void SetPosition(const Vector3& position);
    void SetPositionDirectionFromMatrix(const Matrix4& worldTransform);
    const Vector3& GetPosition() const;

    void SetDirection(const Vector3& direction);
    const Vector3& GetDirection() const;

    void SetEnvironmentMap(const FilePath& path);
    const FilePath& GetEnvironmentMap() const;

    const Vector4& CalculatePositionDirectionBindVector(Camera* camera);
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SaveToYaml(const FilePath& presetPath, YamlNode* parentNode, const FilePath& realEnvMapPath);
    void LoadFromYaml(const FilePath& presetPath, const YamlNode* parentNode);

    void SetIsDynamic(const bool& isDynamic);
    bool GetIsDynamic();

    void SetCastsShadow(const bool& castsShadow);
    bool GetCastsShadow();

    void AddFlag(uint32 flag);
    void RemoveFlag(uint32 flag);
    uint32 GetFlags();

    const Vector4& GetShadowCascadesIntervals() const;
    void SetShadowCascadesIntervals(const Vector4& values);

    const Vector2& GetShadowFilterRadius() const;
    void SetShadowFilterRadius(const Vector2& value);

    const Vector2& GetShadowWriteBias() const;
    void SetShadowWriteBias(const Vector2& value);

    bool GetDebugDrawShadowMapEnabled() const;
    void SetDebugDrawShadowMapEnabled(bool value);

    bool GetDebugDrawFrustumsEnabled() const;
    void SetDebugDrawFrustumsEnabled(bool value);

    float GetRadius() const;
    void SetRadius(float value);

    float GetAORadius() const;
    void SetAORadius(float value);

    bool GetAutoColor() const;
    void SetAutoColor(bool value);

    bool IsLocal() const;

private:
    DAVA_DEPRECATED(void SetAmbientColor(const Color& color));
    DAVA_DEPRECATED(const Color& GetAmbientColor() const);
    DAVA_DEPRECATED(void SetIntensity(float32 intensity));
    DAVA_DEPRECATED(float32 GetIntensity() const);
    Color ambientColorDeprecated = Color::Black;
    float32 intensityDeprecated = 1.0f;

protected:
    Camera* lastUsedCamera = nullptr;
    FilePath environmentMap;
    uint32 flags = 0;
    uint32 lastUpdatedFrame = 0;
    eType type = eType::TYPE_SUN;
    Vector3 position = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 direction = Vector3(0.0f, 0.0f, -1.0f);
    Vector4 resultPositionDirection = Vector4(0.0f, 0.0f, -1.0f, 0.0f);

    Color baseColor = Color::White;
    float32 lastSetColorTemperature = 0.0f;

    Vector4 shadowCascadesIntervals = Vector4(16.0f, 32.0f, 64.0f, 128.0f);
    Vector2 shadowFilterRadius = Vector2(0.0f, 0.0f);
    Vector2 shadowWriteBias = Vector2(0.0, 0.0);
    float32 radius = 1.0f;
    float32 aoRadius = 1.0f;

    bool debugDrawShadowMaps = false;
    bool debugDrawFrustums = false;
    bool autoColor = false;

    DAVA_VIRTUAL_REFLECTION(Light, BaseObject);
};

inline Light::eType Light::GetLightType() const
{
    return type;
}

inline void Light::SetLightType(eType _type)
{
    type = _type;
}

inline const Vector3& Light::GetPosition() const
{
    return position;
}

inline const Vector3& Light::GetDirection() const
{
    return direction;
}

inline void Light::SetPosition(const Vector3& _position)
{
    position = _position;
}

inline void Light::SetDirection(const Vector3& _direction)
{
    direction = _direction;
}

inline const Color& Light::GetAmbientColor() const
{
    return ambientColorDeprecated;
}

inline const Color& Light::GetColor() const
{
    return baseColor;
}

inline float32 Light::GetIntensity() const
{
    return intensityDeprecated;
}

inline void Light::SetEnvironmentMap(const FilePath& path)
{
    environmentMap = path;
}

inline const FilePath& Light::GetEnvironmentMap() const
{
    return environmentMap;
}

inline const Vector2& Light::GetShadowFilterRadius() const
{
    return shadowFilterRadius;
}

inline void Light::SetShadowFilterRadius(const Vector2& value)
{
    shadowFilterRadius = value;
}

inline const Vector4& Light::GetShadowCascadesIntervals() const
{
    return shadowCascadesIntervals;
}

inline void Light::SetShadowCascadesIntervals(const Vector4& values)
{
    shadowCascadesIntervals = values;
}

inline bool Light::GetDebugDrawShadowMapEnabled() const
{
    return debugDrawShadowMaps;
}

inline void Light::SetDebugDrawShadowMapEnabled(bool value)
{
    debugDrawShadowMaps = value;
}

inline bool Light::GetDebugDrawFrustumsEnabled() const
{
    return debugDrawFrustums;
}

inline void Light::SetDebugDrawFrustumsEnabled(bool value)
{
    debugDrawFrustums = value;
}

inline const Vector2& Light::GetShadowWriteBias() const
{
    return shadowWriteBias;
}

inline void Light::SetShadowWriteBias(const Vector2& value)
{
    shadowWriteBias = value;
}

inline float Light::GetRadius() const
{
    return radius;
}

inline void Light::SetRadius(float value)
{
    radius = value;
}

inline float Light::GetAORadius() const
{
    return aoRadius;
}

inline void Light::SetAORadius(float value)
{
    aoRadius = value;
}

inline bool Light::GetAutoColor() const
{
    return autoColor;
}

inline void Light::SetAutoColor(bool value)
{
    autoColor = value;
}

inline bool Light::IsLocal() const
{
    return (type == eType::TYPE_POINT) || (type == eType::TYPE_SPOT);
}
}
