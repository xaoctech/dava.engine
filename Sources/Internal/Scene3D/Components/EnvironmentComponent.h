#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class EnvironmentComponent : public Component
{
public:
    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    float GetFogDistanceScale() const;
    void SetFogDistanceScale(float value);

    float GetFogTurbidity() const;
    void SetFogTurbidity(float value);

    float GetFogAnisotropy() const;
    void SetFogAnisotropy(float value);

public:
    DAVA_VIRTUAL_REFLECTION(EnvironmentComponent, Component);

private:
    float fogDistanceScale = 1.0f;
    float fogTurbidity = 0.0f;
    float fogAnisotropy = 0.0f;
};

inline float EnvironmentComponent::GetFogDistanceScale() const
{
    return fogDistanceScale;
}
inline void EnvironmentComponent::SetFogDistanceScale(float value)
{
    fogDistanceScale = value;
}
inline float EnvironmentComponent::GetFogTurbidity() const
{
    return fogTurbidity;
}
inline void EnvironmentComponent::SetFogTurbidity(float value)
{
    fogTurbidity = value;
}
inline float EnvironmentComponent::GetFogAnisotropy() const
{
    return fogAnisotropy;
}
inline void EnvironmentComponent::SetFogAnisotropy(float value)
{
    fogAnisotropy = value;
}
}
