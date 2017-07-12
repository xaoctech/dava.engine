#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"

namespace DAVA
{
class AnimationClip;
class MotionSystem;
class MotionComponent : public Component
{
public:
    IMPLEMENT_COMPONENT_TYPE(MOTION_COMPONENT);

    MotionComponent() = default;
    ~MotionComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    const FilePath& GetAnimationPath() const;
    void SetAnimationPath(const FilePath& path);

private:
    FilePath animationPath;

    AnimationClip* animationClip = nullptr;

public:
    INTROSPECTION_EXTEND(MotionComponent, Component,
                         PROPERTY("animation", "animation", GetAnimationPath, SetAnimationPath, I_SAVE | I_VIEW | I_EDIT)
                         );

    DAVA_VIRTUAL_REFLECTION(MotionComponent, Component);

    friend class MotionSystem;
};

} //ns
