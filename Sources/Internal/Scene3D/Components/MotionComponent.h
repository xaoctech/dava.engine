#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"

namespace DAVA
{
class AnimationClip;
class MotionSystem;
class SkeletonAnimation;
class SkeletonComponent;
class Motion;
class MotionComponent : public Component
{
public:
    static const FastName EVENT_SINGLE_ANIMATION_STARTED;
    static const FastName EVENT_SINGLE_ANIMATION_ENDED;

    IMPLEMENT_COMPONENT_TYPE(MOTION_COMPONENT);

    MotionComponent() = default;
    ~MotionComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    uint32 GetMotionsCount() const;
    Motion* GetMotion(uint32 index) const;

    const FilePath& GetConfigPath() const;
    void SetConfigPath(const FilePath& path);

protected:
    void ReloadFromConfig();

    FilePath configPath;
    Vector<Motion*> motions;

    DAVA_VIRTUAL_REFLECTION(MotionComponent, Component);

    //temporary for debug
    //////////////////////////////////////////////////////////////////////////
    float32 GetDebugParameterX() const
    {
        return debugParameter.x;
    }
    void SetDebugParameterX(float32 value)
    {
        debugParameter.x = value;
    }
    float32 GetDebugParameterY() const
    {
        return debugParameter.y;
    }
    void SetDebugParameterY(float32 value)
    {
        debugParameter.y = value;
    }
    float32 GetDebugParameterX2() const
    {
        return debugParameter2.x;
    }
    void SetDebugParameterX2(float32 value)
    {
        debugParameter2.x = value;
    }
    float32 GetDebugParameterY2() const
    {
        return debugParameter2.y;
    }
    void SetDebugParameterY2(float32 value)
    {
        debugParameter2.y = value;
    }
    Vector2 debugParameter;
    Vector2 debugParameter2;
    //////////////////////////////////////////////////////////////////////////

public:
    INTROSPECTION_EXTEND(MotionComponent, Component, nullptr);

    friend class MotionSystem;
};

} //ns
