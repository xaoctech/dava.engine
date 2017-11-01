#include "Animation/AnimationClip.h"
#include "FileSystem/YamlNode.h"
#include "FileSystem/YamlParser.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SkeletonAnimation/Motion.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
REGISTER_CLASS(MotionComponent)

DAVA_VIRTUAL_REFLECTION_IMPL(MotionComponent)
{
    ReflectionRegistrator<MotionComponent>::Begin()
    .ConstructorByPointer()
    .Field("configPath", &MotionComponent::GetConfigPath, &MotionComponent::SetConfigPath)[M::DisplayName("Motion Config")]
    .Field("playbackRate", &MotionComponent::GetPlaybackRate, &MotionComponent::SetPlaybackRate)[M::DisplayName("Playback Rate"), M::Range(0.f, 1.f, 0.1f)]
    .Field("parameters", &MotionComponent::parameters)[M::DisplayName("parameters")]
    .Field("motions", &MotionComponent::motions)[M::DisplayName("Motions")]
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

void MotionComponent::TriggerEvent(const FastName& trigger)
{
	for (Motion* motion : motions)
		motion->TriggerEvent(trigger);
}

void MotionComponent::SetParameter(const FastName& parameterID, float32 value)
{
	auto found = parameters.find(parameterID);
	if (found != parameters.end())
		found->second = value;
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
    parameters.clear();

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
                    Motion* motion = Motion::LoadFromYaml(motionNode);
                    if (motion != nullptr)
                    {
                        motions.push_back(motion);

                        for (const FastName& p : motion->GetParameterIDs())
                            parameters[p] = 0.f;
                    }
                }

                for (Motion* motion : motions)
                {
                    for (const FastName& p : motion->GetParameterIDs())
                        motion->BindParameter(p, &parameters[p]);
                }
            }
        }
    }

    SafeRelease(parser);
}

}
