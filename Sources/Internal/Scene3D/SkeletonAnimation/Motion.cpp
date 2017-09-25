#include "Motion.h"

#include "BlendTree.h"
#include "Base/GlobalEnum.h"
#include "FileSystem/YamlNode.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

#include "Private/MotionStateSequence.h"

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

Motion::Motion()
{
    primaryStateSequence = new MotionStateSequence(this);
}

Motion::~Motion()
{
    for (MotionTransitionInfo* t : transitions)
        SafeDelete(t);

    SafeDelete(primaryStateSequence);
}

bool Motion::RequestState(const FastName& stateUID)
{
    if (GetRequestedState() == stateUID)
        return true;

    bool success = false;

    auto foundState = statesMap.find(stateUID);
    if (foundState != statesMap.end())
    {
        pendingState = foundState->second;
        success = true;
    }

    return success;
}

const FastName& Motion::GetRequestedState() const
{
    static const FastName invalidID = FastName("#invalid-state");

    if (pendingState != nullptr)
        return pendingState->GetID();
    else if (secondaryStateSequence != nullptr && secondaryStateSequence->GetLastState() != nullptr)
        return secondaryStateSequence->GetLastState()->GetID();
    else if (primaryStateSequence->GetLastState() != nullptr)
        return primaryStateSequence->GetLastState()->GetID();
    else
        return invalidID;
}

void Motion::Update(float32 dTime)
{
    DVASSERT(primaryStateSequence != nullptr);

    endedPhases.clear();

    if (pendingState != nullptr && secondaryStateSequence == nullptr)
    {
        MotionState* lastState = primaryStateSequence->GetCurrentState();
        MotionTransitionInfo* pendingTransitionInfo = GetTransition(lastState, pendingState);
        if (pendingTransitionInfo != nullptr && pendingTransitionInfo->type == MotionTransitionInfo::TYPE_STATE)
        {
            if (primaryStateSequence->CanBeInterrupted(pendingTransitionInfo->transitionState))
            {
                primaryStateSequence->Interrupt(pendingTransitionInfo->transitionState);
                primaryStateSequence->Append(pendingState);

                pendingState = nullptr;
            }
            else if (secondaryStateSequence == nullptr)
            {
                secondaryStateSequence = new MotionStateSequence(this);
                secondaryStateSequence->Append(pendingTransitionInfo->transitionState);
                secondaryStateSequence->Append(pendingState);

                pendingState = nullptr;
            }
        }
        else
        {
            if (primaryStateSequence->CanBeInterrupted(pendingState))
            {
                primaryStateSequence->Interrupt(pendingState);

                pendingState = nullptr;
            }
            else if (secondaryStateSequence == nullptr)
            {
                secondaryStateSequence = new MotionStateSequence(this);
                secondaryStateSequence->Append(pendingState);

                pendingState = nullptr;
            }
        }
    }

    if (!transitionIsActive && secondaryStateSequence != nullptr)
    {
        MotionTransitionInfo* transitionInfo = GetTransition(primaryStateSequence->GetCurrentState(), secondaryStateSequence->GetCurrentState());

        if (transitionInfo != nullptr)
        {
            DVASSERT(transitionInfo->type != MotionTransitionInfo::TYPE_STATE);
            currentTransition.Reset(transitionInfo, primaryStateSequence, secondaryStateSequence);
            transitionIsActive = true;
        }
        else
        {
            SafeDelete(primaryStateSequence);
            primaryStateSequence = secondaryStateSequence;
            secondaryStateSequence = nullptr;
        }
    }

    primaryStateSequence->Update(dTime);
    primaryStateSequence->EvaluateRootOffset(&currentRootOffsetDelta);

    if (transitionIsActive)
    {
        DVASSERT(secondaryStateSequence != nullptr);

        secondaryStateSequence->Update(dTime);
        currentTransition.Update(dTime);

        if (currentTransition.IsComplete())
        {
            currentTransition.EvaluateRootOffset(&currentRootOffsetDelta);

            SafeDelete(primaryStateSequence);
            primaryStateSequence = secondaryStateSequence;
            secondaryStateSequence = nullptr;

            transitionIsActive = false;
        }
    }

    //TODO: *Skinning*
    {
        MotionState* currentState = primaryStateSequence->GetCurrentState();
        if (currentState->IsPhaseEnd())
        {
            const FastName& endedPhaseName = currentState->GetLastPhaseName();
            if (endedPhaseName.IsValid())
                endedPhases.emplace_back(currentState->GetID(), endedPhaseName);
        }
    }

    //Evaluate
    currentPose.Reset();
    if (transitionIsActive)
    {
        currentTransition.EvaluatePose(&currentPose);
        currentTransition.EvaluateRootOffset(&currentRootOffsetDelta);
    }
    else
    {
        primaryStateSequence->EvaluatePose(&currentPose);
    }

    //Temp for debug
    currentRootOffsetDelta.z = 0.f;

    Vector3 rootPosition = currentPose.GetJointTransform(0).GetPosition();
    rootPosition.x = rootPosition.y = 0.f;
    currentPose.SetPosition(0, rootPosition);
}

void Motion::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (MotionState& s : states)
        s.BindSkeleton(skeleton);

    if (primaryStateSequence->GetCurrentState() != nullptr)
    {
        currentPose.Reset();

        MotionState* currentState = primaryStateSequence->GetCurrentState();
        currentState->Reset();
        currentState->EvaluatePose(&currentPose);
        currentState->GetRootOffsetDelta(&currentRootOffsetDelta);
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
            motion->primaryStateSequence->Append(motion->states.data());
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
                    DVASSERT(motion->transitions[transitionIndex] == nullptr);
                    motion->transitions[transitionIndex] = MotionTransitionInfo::LoadFromYaml(transitionNode, motion->statesMap);
                }
            }
        }
    }

    motion->parameterIDs.insert(motion->parameterIDs.begin(), statesParameters.begin(), statesParameters.end());

    return motion;
}

} //ns