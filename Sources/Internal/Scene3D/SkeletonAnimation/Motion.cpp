#include "Motion.h"

#include "Base/GlobalEnum.h"
#include "FileSystem/YamlNode.h"
#include "Scene3D/SkeletonAnimation/BlendNode.h"

ENUM_DECLARE(DAVA::Motion::eBlend)
{
    ENUM_ADD_DESCR(DAVA::Motion::eBlend::BLEND_OVERRIDE, "Override");
    ENUM_ADD_DESCR(DAVA::Motion::eBlend::BLEND_ADD, "Additive");
    ENUM_ADD_DESCR(DAVA::Motion::eBlend::BLEND_SUB, "Subtract");
    ENUM_ADD_DESCR(DAVA::Motion::eBlend::BLEND_LERP, "LERP");
};

namespace DAVA
{
Motion::~Motion()
{
    for (State& s : states)
        SafeDelete(s.treeRoot);
}

Motion* Motion::LoadFromYaml(const YamlNode* motionNode)
{
    Motion* motion = new Motion();

    const YamlNode* nameNode = motionNode->Get("name");
    if (nameNode != nullptr && nameNode->GetType() == YamlNode::TYPE_STRING)
    {
        motion->name = nameNode->AsFastName();
    }

    const YamlNode* blendModeNode = motionNode->Get("blend-mode");
    if (blendModeNode != nullptr && blendModeNode->GetType() == YamlNode::TYPE_STRING)
    {
        int32 enumValue;
        if (GlobalEnumMap<Motion::eBlend>::Instance()->ToValue(blendModeNode->AsString().c_str(), enumValue))
            motion->blendMode = eBlend(enumValue);
    }

    const YamlNode* statesNode = motionNode->Get("states");
    if (statesNode != nullptr && statesNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        uint32 statesCount = statesNode->GetCount();
        motion->states.resize(statesCount);
        for (uint32 s = 0; s < statesCount; ++s)
        {
            State& state = motion->states[s];

            const YamlNode* stateIDNode = statesNode->Get(s)->Get("state-id");
            if (stateIDNode != nullptr && stateIDNode->GetType() == YamlNode::TYPE_STRING)
                state.id = stateIDNode->AsFastName();

            const YamlNode* blendTreeNode = statesNode->Get(s)->Get("blend-tree");
            if (blendTreeNode != nullptr)
            {
                state.treeRoot = motion->LoadBlendNodeFromYaml(blendTreeNode);
            }
        }

        if (statesCount > 0)
        {
            motion->currentState = motion->states.data();
        }
    }

    return motion;
}

BlendNode* Motion::LoadBlendNodeFromYaml(const YamlNode* yamlNode)
{
    BlendNode* blendNode = nullptr;

    const YamlNode* clipNode = yamlNode->Get("clip");
    if (clipNode != nullptr && clipNode->GetType() == YamlNode::TYPE_STRING)
    {
        FilePath animationClipPath(clipNode->AsString());
        ScopedPtr<AnimationClip> animationClip(AnimationClip::Load(animationClipPath));
        if (animationClip)
        {
            blendNode = new BlendNode(animationClip);
            //animationNodes.emplace_back(blendNode, animationClip);
        }
    }
    else
    {
        const YamlNode* operationNode = yamlNode->Get("operation");
        if (operationNode != nullptr && operationNode->GetType() == YamlNode::TYPE_MAP)
        {
            const YamlNode* typeNode = operationNode->Get("type");
            if (typeNode != nullptr && typeNode->GetType() == YamlNode::TYPE_STRING)
            {
                int32 nodeType;
                if (GlobalEnumMap<BlendNode::eType>::Instance()->ToValue(typeNode->AsString().c_str(), nodeType))
                {
                    blendNode = new BlendNode(BlendNode::eType(nodeType));

                    const YamlNode* parameterNode = operationNode->Get("parameter");
                    if (parameterNode != nullptr && parameterNode->GetType() == YamlNode::TYPE_STRING)
                    {
                        FastName parameter = parameterNode->AsFastName();
                        AddParametrizedNode(blendNode, parameter);
                    }

                    const YamlNode* childrenNode = yamlNode->Get("nodes");
                    if (childrenNode != nullptr && childrenNode->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        uint32 childrenCount = childrenNode->GetCount();
                        for (uint32 c = 0; c < childrenCount; ++c)
                        {
                            const YamlNode* childNode = childrenNode->Get(c);

                            Vector2 childParam;
                            const YamlNode* childParamNode = childNode->Get("param-value");
                            if (childParamNode != nullptr)
                            {
                                if (childParamNode->GetType() == YamlNode::TYPE_ARRAY)
                                    childParam = childParamNode->AsVector2();
                                else if (childParamNode->GetType() == YamlNode::TYPE_STRING)
                                    childParam.x = childParamNode->AsFloat();
                            }

                            BlendNode* child = LoadBlendNodeFromYaml(childNode);
                            blendNode->AddChild(child, childParam);
                        }
                    }
                }
            }
        }
    }

    return blendNode;
}

void Motion::AddParametrizedNode(BlendNode* node, const FastName& parameter)
{
    auto found = std::find(parametersIDs.begin(), parametersIDs.end(), parameter);
    size_t paramterIndex = std::distance(parametersIDs.begin(), found);
    if (found == parametersIDs.end())
    {
        parametersIDs.emplace_back(parameter);
        parametrizedNodes.emplace_back();
    }
    parametrizedNodes[paramterIndex].push_back(node);
}

void Motion::SetParameter(uint32 index, Vector2 value)
{
    DVASSERT(index < GetParametersCount());
    for (BlendNode* node : parametrizedNodes[index])
        node->SetParameter(value);
}

void Motion::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (State& state : states)
        state.treeRoot->BindSkeleton(skeleton);

    if (currentState != nullptr)
        currentState->treeRoot->Evaluate(&resultPose, 0.f);
}

void Motion::Update(float32 dTime)
{
    if (currentState != nullptr)
    {
        normalizedTime += dTime * playbackRate;
        if (normalizedTime >= 1.f)
            normalizedTime -= 1.f;

        currentState->treeRoot->Evaluate(&resultPose, normalizedTime);
    }
}

} //ns