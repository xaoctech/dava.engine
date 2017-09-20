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

    float32 GetPlaybackRate() const;
    void SetPlaybackRate(float32 rate);

    void ReloadFromConfig();

    Vector3 rootOffsetDelta;

protected:
    FilePath configPath;
    Vector<Motion*> motions;

    DAVA_VIRTUAL_REFLECTION(MotionComponent, Component);

    //temporary for debug
    //////////////////////////////////////////////////////////////////////////
    float32 playbackRate = 1.f;
    Map<FastName, float32> parameters;
    //////////////////////////////////////////////////////////////////////////

    friend class MotionSystem;
};

} //ns
