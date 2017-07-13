#include "Animation2/AnimationClip.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
REGISTER_CLASS(MotionComponent)

DAVA_VIRTUAL_REFLECTION_IMPL(MotionComponent)
{
    ReflectionRegistrator<MotionComponent>::Begin()
    .ConstructorByPointer()
    .Field("animationPath", &MotionComponent::GetAnimationPath, &MotionComponent::SetAnimationPath)[M::DisplayName("Animation File")]
    .End();
}

MotionComponent::~MotionComponent()
{
    SafeRelease(animationClip);
}

const FilePath& MotionComponent::GetAnimationPath() const
{
    return animationPath;
}

void MotionComponent::SetAnimationPath(const FilePath& path)
{
    animationPath = path;

    SafeRelease(animationClip);
    if (!animationPath.IsEmpty())
    {
        animationClip = AnimationClip::Load(animationPath);
    }

    GlobalEventSystem::Instance()->Event(this, EventSystem::MOTION_CHANGED);
}

Component* MotionComponent::Clone(Entity* toEntity)
{
    MotionComponent* newComponent = new MotionComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetAnimationPath(animationPath);
    return newComponent;
}

void MotionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    String animationRelativePath = animationPath.GetRelativePathname(serializationContext->GetScenePath());
    archive->SetString("animationPath", animationRelativePath);
}

void MotionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    String animationRelativePath = archive->GetString("animationPath");
    SetAnimationPath(serializationContext->GetScenePath() + animationRelativePath);
}
}