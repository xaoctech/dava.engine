#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

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
    class SimpleMotion;

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

    //temporary for debug
    float32 workSpeedParameter = 0.f;

protected:
    void ReloadFromConfig();

    FilePath configPath;
    Vector<Motion*> motions;

    DAVA_VIRTUAL_REFLECTION(MotionComponent, Component);

public:
    INTROSPECTION_EXTEND(MotionComponent, Component, nullptr);

    friend class MotionSystem;
};

//////////////////////////////////////////////////////////////////////////

class MotionComponent::SimpleMotion
{
    SimpleMotion(MotionComponent* component);

public:
    ~SimpleMotion();

    void BindSkeleton(SkeletonComponent* skeleton);
    void Start();
    void Stop();
    void Update(float32 dTime);

    bool IsPlaying() const;
    bool IsFinished() const;

    const SkeletonPose& GetSkeletonPose() const;

    const FilePath& GetAnimationPath() const;
    void SetAnimationPath(const FilePath& path);

    uint32 GetRepeatsCount() const;
    void SetRepeatsCount(uint32 count);

private:
    //Serializable
    FilePath animationPath;
    uint32 repeatsCount = 0;

    //Runtime
    AnimationClip* animationClip = nullptr;
    SkeletonAnimation* skeletonAnimation = nullptr;

    SkeletonPose resultPose;

    bool isPlaying = false;
    uint32 repeatsLeft = 0;
    float32 currentAnimationTime = 0.f;

    MotionComponent* component = nullptr; //weak-pointer

    friend class MotionComponent;

    DAVA_REFLECTION(SimpleMotion);
};

} //ns
