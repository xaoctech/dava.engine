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
    .Field("state", &Motion::GetRequestedState, &Motion::SetStateID)[M::DisplayName("Motion State")]
    .End();
}

Motion::~Motion()
{
    for (MotionTransitionInfo* t : transitions)
        SafeDelete(t);
}

bool Motion::RequestState(const FastName& stateUID)
{
    if (GetRequestedState() == stateUID)
        return true;

    bool success = false;

    auto foundState = statesMap.find(stateUID);
    if (foundState != statesMap.end())
    {
        pendingState = (currentState != foundState->second) ? (foundState->second) : nullptr;
        success = true;
    }

    return success;
}

void Motion::Update(float32 dTime)
{
    endedPhases.clear();

    //Update
    if (pendingState != nullptr)
    {
        bool transitionInterruption = false;
        if (currentState != nullptr)
        {
            MotionTransitionInfo* nextTransition = GetTransition(currentState, pendingState);
            if (nextTransition != nullptr)
            {
                if (nextTransition->type == MotionTransitionInfo::TYPE_STATE)
                {
                    DVASSERT(nextTransition->transitionState != nullptr);
                    afterTransitionState = pendingState;
                    currentState = nextTransition->transitionState;
                    currentState->Reset();
                }
                else
                {
                    if (transitionIsActive)
                    {
                        if (!currentTransition.IsStarted())
                        {
                            if (nextTransition != nullptr)
                                currentTransition.Reset(nextTransition, currentState, pendingState);
                            else
                                transitionIsActive = false;
                        }
                        else
                        {
                            if (currentTransition.CanBeInterrupted(nextTransition, pendingState))
                            {
                                currentTransition.Interrupt(nextTransition, pendingState);
                                transitionInterruption = true;
                            }
                        }
                    }
                    else
                    {
                        currentTransition.Reset(nextTransition, currentState, pendingState);
                        transitionIsActive = true;
                    }
                }
            }
            else
            {
                currentState = pendingState;
            }
        }
        else
        {
            currentState = pendingState;
        }

        if (!transitionInterruption)
            pendingState->Reset();

        pendingState = nullptr;
    }

    currentState->Update(dTime);

    if (afterTransitionState != nullptr && currentState->IsAnimationEnd())
    {
        currentState = afterTransitionState;
        afterTransitionState = nullptr;

        if (transitionIsActive)
            currentTransition.SetSrcState(currentState);
    }

    if (currentState->IsPhaseEnd())
    {
        const FastName& endedPhaseName = currentState->GetLastPhaseName();
        if (endedPhaseName.IsValid())
            endedPhases.emplace_back(currentState->GetID(), endedPhaseName);
    }

    if (transitionIsActive)
    {
        currentTransition.GetDstState()->Update(dTime);
        currentTransition.Update(dTime);

        if (currentTransition.IsComplete())
        {
            currentState = currentTransition.GetDstState();
            afterTransitionState = nullptr;
            transitionIsActive = false;
        }
    }

    //Evaluate
    currentPose.Reset();
    if (transitionIsActive)
    {
        currentTransition.EvaluatePose(&currentPose);
    }
    else
    {
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

uint32 Motion::GetTransitionIndex(const MotionState* srcState, const MotionState* dstState) const
{
    size_t srcStateIndex = std::distance(states.data(), srcState);
    size_t dstStateIndex = std::distance(states.data(), dstState);
    return uint32(srcStateIndex * states.size() + dstStateIndex);
}

MotionTransitionInfo* Motion::GetTransition(const MotionState* srcState, const MotionState* dstState) const
{
    if (srcState == nullptr || dstState == nullptr)
        return nullptr;

    return transitions[GetTransitionIndex(srcState, dstState)];
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

            const YamlNode* srcNode = transitionNode->Get("src-state");
            const YamlNode* dstNode = transitionNode->Get("dst-state");
            if (srcNode != nullptr && srcNode->GetType() == YamlNode::TYPE_STRING &&
                dstNode != nullptr && dstNode->GetType() == YamlNode::TYPE_STRING)
            {
                auto foundSrc = motion->statesMap.find(srcNode->AsFastName());
                auto foundDst = motion->statesMap.find(dstNode->AsFastName());

                if (foundSrc != motion->statesMap.end() && foundDst != motion->statesMap.end())
                {
                    uint32 transitionIndex = motion->GetTransitionIndex(foundSrc->second, foundDst->second);
                    motion->transitions[transitionIndex] = MotionTransitionInfo::LoadFromYaml(transitionNode, motion->statesMap);
                    ;
                }
            }
        }
    }

    motion->parameterIDs.insert(motion->parameterIDs.begin(), statesParameters.begin(), statesParameters.end());

    return motion;
}

} //ns