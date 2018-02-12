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

    ReflectionProbe::eType GetReflectionType() const;
    void SetReflectionType(ReflectionProbe::eType type_);

    void SetCapturePosition(const Vector3& capturePosition_);
    const Vector3& GetCapturePosition() const;

    void SetCaptureSize(const Vector3& captureSize_);
    const Vector3& GetCaptureSize() const;

    ReflectionProbe* GetReflectionProbe() const;
    void SetReflectionProbe(ReflectionProbe* probe);

    bool GetDebugDrawEnabled() const;
    void SetDebugDrawEnabled(bool enabled);

private:
    ReflectionProbe::eType type = ReflectionProbe::TYPE_GLOBAL;
    ReflectionProbe* reflectionProbe = nullptr;
    bool debugDraw = false;
    Vector3 capturePosition = Vector3(0.0f, 0.0f, 0.0f);
    Vector3 captureSize = Vector3(50.0f, 50.0f, 50.0f);

public:
    DAVA_VIRTUAL_REFLECTION(ReflectionComponent, Component);
};

inline ReflectionProbe::eType ReflectionComponent::GetReflectionType() const
{
    return type;
}

inline void ReflectionComponent::SetReflectionType(ReflectionProbe::eType type_)
{
    type = type_;
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
};
