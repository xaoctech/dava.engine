#pragma once

#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"

namespace DAVA
{
class AnimationClip;
class MotionSystem;
class SimpleMotion;
class SkeletonAnimation;
class SkeletonComponent;
class Motion;
class YamlNode;
class MotionComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(MOTION_COMPONENT);

    MotionComponent() = default;
    ~MotionComponent();

    void TriggerEvent(const FastName& trigger); //TODO: *Skinning* make adequate naming
    void SetParameter(const FastName& parameterID, float32 value);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    uint32 GetMotionsCount() const;
    Motion* GetMotion(uint32 index) const;

    const FilePath& GetMotionPath() const;
    void SetMotionPath(const FilePath& path);

    float32 GetPlaybackRate() const;
    void SetPlaybackRate(float32 rate);

    const Vector3& GetRootOffsetDelta() const;

    Vector<FilePath> GetDependencies() const;

protected:
    void ReloadFromFile();
    void GetDependenciesRecursive(const YamlNode* node, Set<FilePath>* dependencies) const;

    FilePath motionPath;
    Vector<Motion*> motions;

    float32 playbackRate = 1.f;
    UnorderedMap<FastName, float32> parameters;

    Vector3 rootOffsetDelta;

    SimpleMotion* simpleMotion = nullptr;
    uint32 simpleMotionRepeatsCount = 0;

    DAVA_VIRTUAL_REFLECTION(MotionComponent, Component);

    friend class MotionSystem;
};

inline float32 MotionComponent::GetPlaybackRate() const
{
    return playbackRate;
}

inline void MotionComponent::SetPlaybackRate(float32 rate)
{
    playbackRate = rate;
}

inline const Vector3& MotionComponent::GetRootOffsetDelta() const
{
    return rootOffsetDelta;
}

} //ns
