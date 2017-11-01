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

	void TriggerEvent(const FastName& trigger); //TODO: *Skinning* make adequate naming
	void SetParameter(const FastName& parameterID, float32 value);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    uint32 GetMotionsCount() const;
    Motion* GetMotion(uint32 index) const;

    const FilePath& GetConfigPath() const;
    void SetConfigPath(const FilePath& path);

    float32 GetPlaybackRate() const;
    void SetPlaybackRate(float32 rate);

	const Vector3& GetRootOffsetDelta() const;

protected:
	void ReloadFromConfig();

    FilePath configPath;
	Vector<Motion*> motions;

    float32 playbackRate = 1.f;
	UnorderedMap<FastName, float32> parameters;

	Vector3 rootOffsetDelta;

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
