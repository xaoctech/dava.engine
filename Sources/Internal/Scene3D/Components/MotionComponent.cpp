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
#include "Scene3D/SkeletonAnimation/SimpleMotion.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"

namespace DAVA
{
REGISTER_CLASS(MotionComponent)

DAVA_VIRTUAL_REFLECTION_IMPL(MotionComponent)
{
    ReflectionRegistrator<MotionComponent>::Begin()
    .ConstructorByPointer()
    .Field("motionPath", &MotionComponent::GetMotionPath, &MotionComponent::SetMotionPath)[M::DisplayName("Motion File")]
    .Field("playbackRate", &MotionComponent::GetPlaybackRate, &MotionComponent::SetPlaybackRate)[M::DisplayName("Playback Rate"), M::Range(0.f, 1.f, 0.1f)]
    .Field("parameters", &MotionComponent::parameters)[M::DisplayName("Parameters")]
    .Field("motions", &MotionComponent::motions)[M::DisplayName("Motions")]
	.Field("simpleMotionRepeatsCount", &MotionComponent::simpleMotionRepeatsCount)[M::DisplayName("Single animation Repeats")]
    .End();
}

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
    newComponent->SetMotionPath(GetMotionPath());
    return newComponent;
}

void MotionComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    if (!motionPath.IsEmpty())
    {
        String configRelativePath = motionPath.GetRelativePathname(serializationContext->GetScenePath());
        archive->SetString("motion.filepath", configRelativePath);
    }

	archive->SetUInt32("simpleMotion.repeatsCount", simpleMotionRepeatsCount);
	archive->SetFloat("motion.playbackRate", playbackRate);
}

void MotionComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

	String relativePath = archive->GetString("motion.filepath");

	//////////////////////////////////////////////////////////////////////////
	//back compatibility
	if (relativePath.empty())
		relativePath = archive->GetString("motion.configPath");

	if (relativePath.empty())
		relativePath = archive->GetString("simpleMotion.animationPath");

	//////////////////////////////////////////////////////////////////////////

	if (!relativePath.empty())
		SetMotionPath(serializationContext->GetScenePath() + relativePath);

	simpleMotionRepeatsCount = archive->GetUInt32("simpleMotion.repeatsCount");
	playbackRate = archive->GetFloat("motion.playbackRate", 1.f);
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

const FilePath& MotionComponent::GetMotionPath() const
{
return motionPath;
}

void MotionComponent::SetMotionPath(const FilePath& path)
{
	motionPath = path;

	Entity* entity = GetEntity();
	if (entity && entity->GetScene())
	{
		entity->GetScene()->motionSingleComponent->reloadMotion.emplace_back(this);
	}
}

void MotionComponent::ReloadFromFile()
{
	for (Motion*& m : motions)
		SafeDelete(m);

	motions.clear();
	parameters.clear();
	SafeDelete(simpleMotion);

	if (motionPath.IsEmpty())
		return;

	if (motionPath.IsEqualToExtension(".anim"))
	{
		simpleMotion = new SimpleMotion();
		simpleMotion->SetRepeatsCount(simpleMotionRepeatsCount);

		ScopedPtr<AnimationClip> clip(AnimationClip::Load(motionPath));
		simpleMotion->SetAnimation(clip);
	}
	else if (motionPath.IsEqualToExtension(".yaml"))
	{
		YamlParser* parser = YamlParser::Create(motionPath);
		if (parser != nullptr)
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

Vector<FilePath> MotionComponent::GetDependencies() const
{
	Vector<FilePath> result;

	if (!motionPath.IsEmpty())
	{
		result.push_back(motionPath);

		if (motionPath.IsEqualToExtension(".yaml"))
		{
			YamlParser* parser = YamlParser::Create(motionPath);
			if (parser != nullptr)
			{
				Set<FilePath> dependencies;
				GetDependenciesRecursive(parser->GetRootNode(), &dependencies);
				SafeRelease(parser);

				for (const FilePath& fp : dependencies)
					result.push_back(fp);
			}
		}
	}

	return result;
}

void MotionComponent::GetDependenciesRecursive(const YamlNode* node, Set<FilePath>* dependencies) const
{
	if (node != nullptr)
	{
		if (node->GetType() == YamlNode::TYPE_MAP)
		{
			const YamlNode* clipNode = node->Get("clip");
			if (clipNode != nullptr)
				dependencies->insert(FilePath(clipNode->AsString()));
		}

		uint32 childrenCount = node->GetCount();
		for (uint32 c = 0; c < childrenCount; ++c)
			GetDependenciesRecursive(node->Get(c), dependencies);
	}
}

}
