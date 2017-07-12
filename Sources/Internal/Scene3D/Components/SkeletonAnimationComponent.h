#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"

namespace DAVA
{
class AnimationClip;
class SkeletonComponent;
class SkeletonAnimationSystem;
class SkeletonAnimationComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(SKELETON_ANIMATION_COMPONENT);

    SkeletonAnimationComponent() = default;
    ~SkeletonAnimationComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const FilePath& GetAnimationPath() const;
    void SetAnimationPath(const FilePath& path);

private:
    AnimationClip* animationClip = nullptr;
    Vector<std::pair<const AnimationTrack*, AnimationTrack::State>> animationStates;
    FilePath animationPath;

    bool animationChanged = false;

public:
    INTROSPECTION_EXTEND(SkeletonAnimationComponent, Component,
                         PROPERTY("animation", "animation", GetAnimationPath, SetAnimationPath, I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(SkeletonAnimationComponent, Component);

    friend class SkeletonAnimationSystem;
};

} //ns
