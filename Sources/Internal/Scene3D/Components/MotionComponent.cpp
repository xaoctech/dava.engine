#include "Animation/AnimationClip.h"
#include "FileSystem/YamlParser.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SkeletonAnimation/BlendNode.h"
#include "Scene3D/SkeletonAnimation/Motion.h"
#include "Scene3D/SkeletonAnimation/SkeletonAnimation.h"
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
    .Field("configPath", &MotionComponent::GetConfigPath, &MotionComponent::SetConfigPath)[M::DisplayName("Motion Config")]
    .Field("workSpeedParameter", &MotionComponent::workSpeedParameter)[M::DisplayName("Work Speed"), M::Range(0.f, 1.f, 0.01f)]
    .End();
}

const FastName MotionComponent::EVENT_SINGLE_ANIMATION_STARTED = FastName("SingleAnimationStarted");
const FastName MotionComponent::EVENT_SINGLE_ANIMATION_ENDED = FastName("SingleAnimationEnded");

//////////////////////////////////////////////////////////////////////////

MotionComponent::~MotionComponent()
{
    for (Motion*& m : motions)
        SafeDelete(m);
}

Component* MotionComponent::Clone(Entity* toEntity)
{
    MotionComponent* newComponent = new MotionComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetConfigPath(GetConfigPath());
    return newComponent;
}

void MotionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (!configPath.IsEmpty())
    {
        String configRelativePath = configPath.GetRelativePathname(serializationContext->GetScenePath());
        archive->SetString("motion.configPath", configRelativePath);
    }
}

void MotionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    String configRelativePath = archive->GetString("motion.configPath");
    if (!configRelativePath.empty())
    {
        SetConfigPath(serializationContext->GetScenePath() + configRelativePath);
    }
}

uint32 MotionComponent::GetMotionsCount() const
{
    return uint32(motions.size());
}

Motion* MotionComponent::GetMotion(uint32 index) const
{
    DVASSERT(index < GetMotionsCount());
    return motions[index];
}

const FilePath& MotionComponent::GetConfigPath() const
{
    return configPath;
}

void MotionComponent::SetConfigPath(const FilePath& path)
{
    configPath = path;

    Entity* entity = GetEntity();
    if (entity && entity->GetScene())
    {
        entity->GetScene()->motionSingleComponent->reloadConfig.emplace_back(this);
    }
}

void MotionComponent::ReloadFromConfig()
{
    for (Motion*& m : motions)
        SafeDelete(m);
    motions.clear();

    if (configPath.IsEmpty())
        return;

    YamlParser* parser = YamlParser::Create(configPath);
    if (parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if (rootNode)
        {
            const YamlNode* motionsNode = rootNode->Get("Motions");
            if (motionsNode != nullptr && motionsNode->GetType() == YamlNode::TYPE_ARRAY)
            {
                uint32 motionsCount = motionsNode->GetCount();
                for (uint32 m = 0; m < motionsCount; ++m)
                {
                    const YamlNode* motionNode = motionsNode->Get(m);
                    motions.emplace_back(Motion::LoadFromYaml(motionNode));
                }
            }
        }
    }

    SafeRelease(parser);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

DAVA_REFLECTION_IMPL(MotionComponent::SimpleMotion)
{
    ReflectionRegistrator<MotionComponent::SimpleMotion>::Begin()
    .Field("animationPath0", &MotionComponent::SimpleMotion::GetAnimationPath, &MotionComponent::SimpleMotion::SetAnimationPath)[M::DisplayName("Animation")]
    .Field("repeatsCount", &MotionComponent::SimpleMotion::GetRepeatsCount, &MotionComponent::SimpleMotion::SetRepeatsCount)[M::DisplayName("Repeats Count")]
    .End();
}

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
    skeletonAnimation = new SkeletonAnimation(animationClip);
    skeletonAnimation->BindSkeleton(skeleton);

    currentAnimationTime = 0.f;
}

void MotionComponent::SimpleMotion::Start()
{
    isPlaying = true;
    repeatsLeft = repeatsCount;
    if (skeletonAnimation)
        skeletonAnimation->EvaluatePose(&resultPose, 0.f);
}

void MotionComponent::SimpleMotion::Stop()
{
    isPlaying = false;
    currentAnimationTime = 0.f;
    if (skeletonAnimation)
        skeletonAnimation->EvaluatePose(&resultPose, 0.f);
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
            isPlaying = (repeatsLeft > 0 || repeatsCount == 0);
            if (isPlaying)
            {
                currentAnimationTime -= animationClip->GetDuration();

                if (repeatsCount != 0)
                    --repeatsLeft;
            }
            else
            {
                currentAnimationTime = 0.f;
            }
        }

        skeletonAnimation->EvaluatePose(&resultPose, currentAnimationTime);
    }
}

bool MotionComponent::SimpleMotion::IsPlaying() const
{
    return isPlaying;
}

bool MotionComponent::SimpleMotion::IsFinished() const
{
    return (repeatsCount > 0) && (isPlaying == false) && (currentAnimationTime != 0.f);
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

const SkeletonPose& MotionComponent::SimpleMotion::GetSkeletonPose() const
{
    return resultPose;
}

//////////////////////////////////////////////////////////////////////////
}
