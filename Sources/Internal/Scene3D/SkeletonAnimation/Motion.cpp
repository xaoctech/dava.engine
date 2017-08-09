#include "Motion.h"

#include "BlendTree.h"
#include "Base/GlobalEnum.h"
#include "FileSystem/YamlNode.h"

ENUM_DECLARE(DAVA::Motion::eMotionBlend)
{
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_OVERRIDE, "Override");
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_ADD, "Additive");
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_SUB, "Subtract");
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_LERP, "LERP");
};

namespace DAVA
{
Motion::~Motion()
{
    for (State& s : states)
        SafeDelete(s.blendTree);
}

void Motion::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (State& s : states)
        s.blendTree->BindSkeleton(skeleton);

    if (currentState != nullptr)
    {
        currentState->animationPhase = 0.f;
        currentPose = skeleton->GetDefaultPose();
        currentState->blendTree->EvaluatePose(0.f, currentState->boundParams, &currentPose);
    }
}

void Motion::Update(float32 dTime)
{
    if (currentState != nullptr)
    {
        float32 duration = currentState->blendTree->EvaluatePhaseDuration(currentState->boundParams);

        float32& phase = currentState->animationPhase;
        phase += dTime / duration;
        if (phase >= 1.f)
            phase -= 1.f;

        currentState->blendTree->EvaluatePose(phase, currentState->boundParams, &currentPose);
    }
}

Motion* Motion::LoadFromYaml(const YamlNode* motionNode)
{
    Motion* motion = new Motion();

    Set<FastName> parameters;

    const YamlNode* nameNode = motionNode->Get("name");
    if (nameNode != nullptr && nameNode->GetType() == YamlNode::TYPE_STRING)
    {
        motion->name = nameNode->AsFastName();
    }

    const YamlNode* blendModeNode = motionNode->Get("blend-mode");
    if (blendModeNode != nullptr && blendModeNode->GetType() == YamlNode::TYPE_STRING)
    {
        int32 enumValue;
        if (GlobalEnumMap<Motion::eMotionBlend>::Instance()->ToValue(blendModeNode->AsString().c_str(), enumValue))
            motion->blendMode = eMotionBlend(enumValue);
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
                state.blendTree = BlendTree::LoadFromYaml(blendTreeNode);

                const Vector<FastName>& treeParams = state.blendTree->GetParameterIDs();
                state.boundParams.resize(treeParams.size());

                parameters.insert(treeParams.begin(), treeParams.end());
            }
        }

        if (statesCount > 0)
        {
            motion->currentState = motion->states.data();
        }
    }

    motion->parameterIDs.insert(motion->parameterIDs.begin(), parameters.begin(), parameters.end());

    return motion;
}

bool Motion::BindParameter(const FastName& parameterID, const Vector2* param)
{
    bool success = false;

    for (State& s : states)
    {
        const Vector<FastName>& params = s.blendTree->GetParameterIDs();
        auto found = std::find(params.begin(), params.end(), parameterID);
        if (found != params.end())
        {
            size_t paramIndex = std::distance(params.begin(), found);
            s.boundParams[paramIndex] = param;

            success = true;
        }
    }

    return success;
}

bool Motion::UnbindParameter(const FastName& parameterID)
{
    return BindParameter(parameterID, nullptr);
}

void Motion::UnbindParameters()
{
    for (State& s : states)
    {
        for (const Vector2*& param : s.boundParams)
            param = nullptr;
    }
}

} //ns