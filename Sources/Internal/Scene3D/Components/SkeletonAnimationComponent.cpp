#include "Animation2/AnimationClip.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/SkeletonAnimationComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
REGISTER_CLASS(SkeletonAnimationComponent)

DAVA_VIRTUAL_REFLECTION_IMPL(SkeletonAnimationComponent)
{
    ReflectionRegistrator<SkeletonAnimationComponent>::Begin()
    .ConstructorByPointer()
    .Field("animationPath", &SkeletonAnimationComponent::GetAnimationPath, &SkeletonAnimationComponent::SetAnimationPath)[M::DisplayName("Animation File")]
    .End();
}

SkeletonAnimationComponent::~SkeletonAnimationComponent()
{
    SafeDelete(animationClip);
}

const FilePath& SkeletonAnimationComponent::GetAnimationPath() const
{
    return animationPath;
}

void SkeletonAnimationComponent::SetAnimationPath(const FilePath& path)
{
    animationPath = path;

    SafeDelete(animationClip);
    if (!animationPath.IsEmpty())
    {
        animationClip = new AnimationClip();
        if (!animationClip->Load(animationPath))
            SafeDelete(animationClip);
    }

    animationChanged = true;
}

Component* SkeletonAnimationComponent::Clone(Entity* toEntity)
{
    SkeletonAnimationComponent* newComponent = new SkeletonAnimationComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetAnimationPath(animationPath);
    return newComponent;
}

void SkeletonAnimationComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    String animationRelativePath = animationPath.GetRelativePathname(serializationContext->GetScenePath());
    archive->SetString("animationPath", animationRelativePath);
}

void SkeletonAnimationComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    String animationRelativePath = archive->GetString("animationPath");
    SetAnimationPath(serializationContext->GetScenePath() + animationRelativePath);
}
}