#include "Animation2/AnimationClip.h"
#include "Animation2/SkeletonAnimation.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
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
    .Field("simpleMotion", &MotionComponent::simpleMotion)[M::DisplayName("Simple Motion")]
    .End();
}

DAVA_REFLECTION_IMPL(MotionComponent::SimpleMotion)
{
    ReflectionRegistrator<MotionComponent::SimpleMotion>::Begin()
    .Field("animationPath", &MotionComponent::SimpleMotion::GetAnimationPath, &MotionComponent::SimpleMotion::SetAnimationPath)[M::DisplayName("Animation")]
    .Field("repeatsCount", &MotionComponent::SimpleMotion::GetRepeatsCount, &MotionComponent::SimpleMotion::SetRepeatsCount)[M::DisplayName("Repeats Count")]
    .End();
}

const FastName MotionComponent::EVENT_SINGLE_ANIMATION_STARTED = FastName("SingleAnimationStarted");
const FastName MotionComponent::EVENT_SINGLE_ANIMATION_ENDED = FastName("SingleAnimationEnded");

//////////////////////////////////////////////////////////////////////////

MotionComponent::SimpleMotion::SimpleMotion(MotionComponent* _component)
    : component(_component)
{
}

MotionComponent::SimpleMotion::~SimpleMotion()
{
    SafeRelease(animationClip);
    SafeDelete(skeletonAnimation);
}

void MotionComponent::SimpleMotion::BindSkeleton(SkeletonComponent* skeleton)
{
    SafeDelete(skeletonAnimation);
    skeletonAnimation = new SkeletonAnimation();
    skeletonAnimation->BindAnimation(animationClip, skeleton);
}

void MotionComponent::SimpleMotion::Start()
{
    isPlaying = true;
    repeatsLeft = repeatsCount;
    if (skeletonAnimation)
        skeletonAnimation->Reset();
}

void MotionComponent::SimpleMotion::Stop()
{
    isPlaying = false;
    currentAnimationTime = 0.f;
    if (skeletonAnimation)
        skeletonAnimation->Reset();
}

void MotionComponent::SimpleMotion::Update(float32 timeElapsed)
{
    if (animationClip == nullptr || skeletonAnimation == nullptr)
        return;

    if (isPlaying)
    {
        currentAnimationTime += timeElapsed;

        if (animationClip->GetDuration() <= currentAnimationTime)
        {
            skeletonAnimation->Reset();

            isPlaying = (repeatsLeft > 0 || repeatsCount == 0);
            if (isPlaying)
            {
                currentAnimationTime -= animationClip->GetDuration();
                skeletonAnimation->Advance(currentAnimationTime);

                if (repeatsCount != 0)
                    --repeatsLeft;
            }
        }
        else
        {
            skeletonAnimation->Advance(timeElapsed);
        }
    }
}

bool MotionComponent::SimpleMotion::IsPlaying() const
{
    return isPlaying;
}

bool MotionComponent::SimpleMotion::IsFinished() const
{
    if (repeatsCount == 0) //infinity-looped motion
        return false;
    else
        return (isPlaying == false) && (currentAnimationTime != 0.f);
}

const FilePath& MotionComponent::SimpleMotion::GetAnimationPath() const
{
    return animationPath;
}

void MotionComponent::SimpleMotion::SetAnimationPath(const FilePath& path)
{
    animationPath = path;

    SafeRelease(animationClip);
    if (!animationPath.IsEmpty())
    {
        animationClip = AnimationClip::Load(animationPath);
    }

    Entity* entity = component->GetEntity();
    if (entity && entity->GetScene())
    {
        entity->GetScene()->motionSingleComponent->rebindAnimation.push_back(component);
    }
}

uint32 MotionComponent::SimpleMotion::GetRepeatsCount() const
{
    return repeatsCount;
}

void MotionComponent::SimpleMotion::SetRepeatsCount(uint32 count)
{
    repeatsCount = count;
}

const SkeletonAnimation* MotionComponent::SimpleMotion::GetAnimation() const
{
    return skeletonAnimation;
}

//////////////////////////////////////////////////////////////////////////

MotionComponent::MotionComponent()
{
    simpleMotion = new SimpleMotion(this);
}

MotionComponent::~MotionComponent()
{
    SafeDelete(simpleMotion);
}

Component* MotionComponent::Clone(Entity* toEntity)
{
    MotionComponent* newComponent = new MotionComponent();
    newComponent->SetEntity(toEntity);
    newComponent->simpleMotion->SetAnimationPath(simpleMotion->GetAnimationPath());
    newComponent->simpleMotion->SetRepeatsCount(simpleMotion->GetRepeatsCount());
    return newComponent;
}

void MotionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    String animationRelativePath = simpleMotion->animationPath.GetRelativePathname(serializationContext->GetScenePath());
    archive->SetString("simpleMotion.animationPath", animationRelativePath);
    archive->SetUInt32("simpleMotion.repeatsCount", simpleMotion->repeatsCount);
}

void MotionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    String animationRelativePath = archive->GetString("simpleMotion.animationPath");
    simpleMotion->SetAnimationPath(serializationContext->GetScenePath() + animationRelativePath);
    simpleMotion->SetRepeatsCount(archive->GetUInt32("simpleMotion.repeatsCount"));
}
}