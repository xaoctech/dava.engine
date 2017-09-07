#include "Motion.h"

#include "BlendTree.h"
#include "Base/GlobalEnum.h"
#include "FileSystem/YamlNode.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

ENUM_DECLARE(DAVA::Motion::eMotionBlend)
{
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_OVERRIDE, "Override");
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_ADD, "Add");
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_DIFF, "Diff");
    ENUM_ADD_DESCR(DAVA::Motion::eMotionBlend::BLEND_LERP, "LERP");
};

namespace DAVA
{
DAVA_REFLECTION_IMPL(Motion)
{
    ReflectionRegistrator<Motion>::Begin()
    .Field("name", &Motion::name)[M::ReadOnly()]
    .Field("state", &Motion::GetStateID, &Motion::SetStateID)[M::DisplayName("Motion State")]
    .End();
}

Motion::~Motion()
{
    for (MotionTransition* t : transitions)
        SafeDelete(t);
}

void Motion::Update(float32 dTime, Vector<std::pair<FastName, FastName>>* outEndedPhases)
{
    currentPose.Reset();
    if (!activeTransitions.empty())
    {
        activeTransitions.front()->Update(dTime, &currentPose);
        if (activeTransitions.front()->IsComplete())
        {
            activeTransitions.front()->Reset();
            activeTransitions.pop_front();
        }
    }

    if (activeTransitions.empty() && currentState != nullptr)
    {
        bool statePhaseEnded = currentState->Update(dTime);

        if (statePhaseEnded && outEndedPhases != nullptr)
        {
            const FastName& endedPhaseName = currentState->GetPhaseName(currentState->GetPreviousPhaseIndex());
            if (endedPhaseName.IsValid())
                outEndedPhases->emplace_back(currentState->GetID(), endedPhaseName);
        }

        currentState->EvaluatePose(&currentPose);
    }
}

void Motion::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (MotionState& s : states)
        s.BindSkeleton(skeleton);

    if (currentState != nullptr)
    {
        currentPose.Reset();
        currentState->Reset();
        currentState->EvaluatePose(&currentPose);
    }
}

bool Motion::BindParameter(const FastName& parameterID, const float32* param)
{
    bool success = false;

    for (MotionState& s : states)
        success |= s.BindParameter(parameterID, param);

    return success;
}

bool Motion::UnbindParameter(const FastName& parameterID)
{
    return BindParameter(parameterID, nullptr);
}

void Motion::UnbindParameters()
{
    for (MotionState& s : states)
        s.UnbindParameters();
}

const Vector<FastName>& Motion::GetStateIDs() const
{
    return statesIDs;
}

uint32 Motion::GetTransitionIndex(MotionState* srcState, MotionState* dstState)
{
    size_t srcStateIndex = std::distance(states.data(), srcState);
    size_t dstStateIndex = std::distance(states.data(), dstState);
    return uint32(srcStateIndex * states.size() + dstStateIndex);
}

bool Motion::RequestState(const FastName& stateUID)
{
    bool success = false;

    auto foundState = statesMap.find(stateUID);
    if (foundState != statesMap.end())
    {
        MotionState* nextState = foundState->second;
        if (currentState != nullptr)
        {
            MotionTransition* nextTransition = transitions[GetTransitionIndex(currentState, nextState)];
            if (nextTransition != nullptr)
            {
                if (!activeTransitions.empty())
                {
                    activeTransitions.back()->Interrupt(nextTransition);
                    activeTransitions.back()->Reset();
                    activeTransitions.back() = nextTransition;
                }
                else
                {
                    activeTransitions.push_back(nextTransition);
                }
            }
        }

        currentState = nextState;
        success = true;
    }

    return success;
}

Motion* Motion::LoadFromYaml(const YamlNode* motionNode)
{
    Motion* motion = new Motion();

    int32 enumValue;
    Set<FastName> statesParameters;

    const YamlNode* nameNode = motionNode->Get("name");
    if (nameNode != nullptr && nameNode->GetType() == YamlNode::TYPE_STRING)
    {
        motion->name = nameNode->AsFastName();
    }

    const YamlNode* blendModeNode = motionNode->Get("blend-mode");
    if (blendModeNode != nullptr && blendModeNode->GetType() == YamlNode::TYPE_STRING)
    {
        if (GlobalEnumMap<Motion::eMotionBlend>::Instance()->ToValue(blendModeNode->AsString().c_str(), enumValue))
            motion->blendMode = eMotionBlend(enumValue);
    }

    const YamlNode* statesNode = motionNode->Get("states");
    if (statesNode != nullptr && statesNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        uint32 statesCount = statesNode->GetCount();
        motion->states.resize(statesCount);
        motion->statesIDs.resize(statesCount);
        for (uint32 s = 0; s < statesCount; ++s)
        {
            MotionState& state = motion->states[s];
            state.LoadFromYaml(statesNode->Get(s));

            motion->statesIDs[s] = state.GetID();
            motion->statesMap[state.GetID()] = &state;

            const Vector<FastName>& blendTreeParams = state.GetBlendTreeParameters();
            statesParameters.insert(blendTreeParams.begin(), blendTreeParams.end());
        }

        if (statesCount > 0)
        {
            motion->currentState = motion->states.data();
        }

        motion->transitions.resize(statesCount * statesCount, nullptr);
    }

    const YamlNode* transitionsNode = motionNode->Get("transitions");
    if (transitionsNode != nullptr && transitionsNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        uint32 transitionsCount = transitionsNode->GetCount();
        for (uint32 t = 0; t < transitionsCount; ++t)
        {
            const YamlNode* transitionNode = transitionsNode->Get(t);
            MotionTransition* transition = MotionTransition::LoadFromYaml(transitionNode);

            const YamlNode* srcNode = transitionNode->Get("src-state");
            const YamlNode* dstNode = transitionNode->Get("dst-state");
            if (srcNode != nullptr && srcNode->GetType() == YamlNode::TYPE_STRING &&
                dstNode != nullptr && dstNode->GetType() == YamlNode::TYPE_STRING)
            {
                MotionState *srcState = nullptr, *dstState = nullptr;

                auto foundSrc = motion->statesMap.find(srcNode->AsFastName());
                if (foundSrc != motion->statesMap.end())
                    srcState = foundSrc->second;

                auto foundDst = motion->statesMap.find(dstNode->AsFastName());
                if (foundDst != motion->statesMap.end())
                    dstState = foundDst->second;

                if (srcState != nullptr && dstState != nullptr)
                {
                    transition->SetStates(srcState, dstState);

                    uint32 transitionIndex = motion->GetTransitionIndex(srcState, dstState);
                    motion->transitions[transitionIndex] = transition;
                }
            }
        }
    }

    motion->parameterIDs.insert(motion->parameterIDs.begin(), statesParameters.begin(), statesParameters.end());

    return motion;
}

} //ns