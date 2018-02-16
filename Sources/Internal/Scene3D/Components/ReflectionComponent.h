#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Render/Highlevel/ReflectionProbe.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Base/GlobalEnum.h"

namespace DAVA
{
class ReflectionComponent : public Component
{
public:
    ReflectionComponent() = default;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    ReflectionProbe::ProbeType GetReflectionType() const;
    void SetReflectionType(ReflectionProbe::ProbeType type_);
    void SetReflectionTypeSilently(ReflectionProbe::ProbeType type_);

    void SetCapturePosition(const Vector3& capturePosition_);
    const Vector3& GetCapturePosition() const;

    void SetCaptureSize(const Vector3& captureSize_);
    const Vector3& GetCaptureSize() const;

    ReflectionProbe* GetReflectionProbe() const;
    void SetReflectionProbe(ReflectionProbe* probe);

    bool GetDebugDrawEnabled() const;
    void SetDebugDrawEnabled(bool enabled);
    void DisableDebugDraw();

    const FilePath& GetReflectionsMap() const;
    void SetReflectionsMap(const FilePath& m);

    const Vector4* GetSphericalHarmonics() const;
    void SetSphericalHarmonics(Vector4 sh[9]);

public:
    DAVA_VIRTUAL_REFLECTION(ReflectionComponent, Component);

private:
    ReflectionProbe::ProbeType probeType = ReflectionProbe::ProbeType::LOCAL;
    ReflectionProbe* reflectionProbe = nullptr;
    Vector4 sphericalHarmonics[9];
    Vector3 capturePosition = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 captureSize = Vector3(50.0f, 50.0f, 50.0f);
    FilePath reflectionsMap;
    bool debugDraw = false;
};

inline ReflectionProbe::ProbeType ReflectionComponent::GetReflectionType() const
{
    return probeType;
}

inline void ReflectionComponent::SetReflectionTypeSilently(ReflectionProbe::ProbeType t)
{
    probeType = t;
}

inline void ReflectionComponent::SetReflectionType(ReflectionProbe::ProbeType t)
{
    SetReflectionTypeSilently(t);
    GlobalEventSystem::Instance()->Event(this, EventSystem::REFLECTION_COMPONENT_CHANGED);
}

inline ReflectionProbe* ReflectionComponent::GetReflectionProbe() const
{
    return reflectionProbe;
}

inline void ReflectionComponent::SetReflectionProbe(ReflectionProbe* reflectionProbe_)
{
    reflectionProbe = reflectionProbe_;
}

inline bool ReflectionComponent::GetDebugDrawEnabled() const
{
    return debugDraw;
}

inline void ReflectionComponent::SetDebugDrawEnabled(bool enabled)
{
    debugDraw = enabled;
    GlobalEventSystem::Instance()->Event(this, EventSystem::REFLECTION_COMPONENT_CHANGED);
}

inline void ReflectionComponent::DisableDebugDraw()
{
    debugDraw = false;
}

inline void ReflectionComponent::SetCapturePosition(const Vector3& capturePosition_)
{
    capturePosition = capturePosition_;
    GlobalEventSystem::Instance()->Event(this, EventSystem::REFLECTION_COMPONENT_CHANGED);
}

inline const Vector3& ReflectionComponent::GetCapturePosition() const
{
    return capturePosition;
}

inline void ReflectionComponent::SetCaptureSize(const Vector3& captureSize_)
{
    captureSize = captureSize_;
    GlobalEventSystem::Instance()->Event(this, EventSystem::REFLECTION_COMPONENT_CHANGED);
}

inline const Vector3& ReflectionComponent::GetCaptureSize() const
{
    return captureSize;
}

inline const FilePath& ReflectionComponent::GetReflectionsMap() const
{
    return reflectionsMap;
}

inline void ReflectionComponent::SetReflectionsMap(const FilePath& m)
{
    reflectionsMap = m;
    GlobalEventSystem::Instance()->Event(this, EventSystem::REFLECTION_COMPONENT_CHANGED);
}
};
