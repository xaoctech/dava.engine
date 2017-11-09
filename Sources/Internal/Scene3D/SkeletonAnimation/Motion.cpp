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
    .End();
}

void Motion::TriggerEvent(const FastName& trigger)
{
    const MotionState::TransitionInfo& transitionInfo = (nextState != nullptr && stateTransition.IsStarted()) ? nextState->GetTransitionInfo(trigger) : currentState->GetTransitionInfo(trigger);
    if (transitionInfo.info != nullptr && transitionInfo.state != nullptr)
    {
        pendingTransition = transitionInfo.info;
        pendingState = transitionInfo.state;
    }
}

void Motion::Update(float32 dTime)
{
    if (pendingState != nullptr)
    {
        if (currentState != nullptr)
        {
            if (nextState != nullptr && stateTransition.IsStarted())
            {
                if (pendingTransition != nullptr && stateTransition.CanBeInterrupted(pendingTransition, nextState, pendingState))
                {
                    stateTransition.Interrupt(pendingTransition, nextState, pendingState);
                    currentState = nextState;
                    nextState = pendingState;

                    pendingState = nullptr;
                    pendingTransition = nullptr;
                }
            }
            else
            {
                if (pendingTransition != nullptr)
                {
                    stateTransition.Reset(pendingTransition, currentState, pendingState);
                    nextState = pendingState;
                    nextState->Reset();

                    pendingState = nullptr;
                    pendingTransition = nullptr;
                }
            }
        }
        else
        {
            currentState = pendingState;
            currentState->Reset();
        }
    }

    //////////////////////////////////////////////////////////////////////////

    if (nextState != nullptr) //transition is active
    {
        stateTransition.Update(dTime);
    }
    else
    {
        currentState->Update(dTime);
    }

    //////////////////////////////////////////////////////////////////////////

    reachedMarkers.clear();
    for (const FastName& m : currentState->GetReachedMarkers())
        reachedMarkers.emplace_back(currentState->GetID(), m);

    if (nextState != nullptr && stateTransition.IsStarted())
    {
        for (const FastName& m : nextState->GetReachedMarkers())
            reachedMarkers.emplace_back(currentState->GetID(), m);
    }

    endedStateAnimations.clear();
    if (currentState->IsAnimationEndReached())
        endedStateAnimations.emplace_back(currentState->GetID());

    //////////////////////////////////////////////////////////////////////////

    currentPose.Reset();
    if (nextState != nullptr) //transition is active
    {
        stateTransition.Evaluate(&currentPose, &currentRootOffsetDelta);
    }
    else
    {
        currentState->EvaluatePose(&currentPose);
        currentState->GetRootOffsetDelta(&currentRootOffsetDelta);
    }

    //////////////////////////////////////////////////////////////////////////

    if (nextState != nullptr) //transition is active
    {
        if (stateTransition.IsComplete())
        {
            currentState = nextState;
            nextState = nullptr;
        }
    }

    //////////////////////////////////////////////////////////////////////////

    currentRootOffsetDelta *= rootExtractionMask;

    if (rootNodeJointIndex != SkeletonComponent::INVALID_JOINT_INDEX)
    {
        Vector3 rootPosition = currentPose.GetJointTransform(rootNodeJointIndex).GetPosition();
        rootPosition *= rootResetMask;
        currentPose.SetPosition(rootNodeJointIndex, rootPosition);
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
        currentState->GetRootOffsetDelta(&currentRootOffsetDelta);
    }

    rootNodeJointIndex = skeleton->GetJointIndex(rootNodeID);
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

    FastName defaultStateID;
    const YamlNode* defaultStateNode = motionNode->Get("default-state");
    if (defaultStateNode != nullptr && defaultStateNode->GetType() == YamlNode::TYPE_STRING)
    {
        defaultStateID = defaultStateNode->AsFastName();
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

            const Vector<FastName>& blendTreeParams = state.GetBlendTreeParameters();
            statesParameters.insert(blendTreeParams.begin(), blendTreeParams.end());

            if (defaultStateID == state.GetID())
                motion->currentState = motion->states.data() + s;
        }

        if (motion->currentState == nullptr && statesCount > 0)
            motion->currentState = motion->states.data();
    }

    const YamlNode* transitionsNode = motionNode->Get("transitions");
    if (transitionsNode != nullptr && transitionsNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        uint32 transitionsCount = transitionsNode->GetCount();
        motion->transitions.resize(transitionsCount);
        for (uint32 t = 0; t < transitionsCount; ++t)
        {
            const YamlNode* transitionNode = transitionsNode->Get(t);

            const YamlNode* srcNode = transitionNode->Get("src-state");
            const YamlNode* dstNode = transitionNode->Get("dst-state");
            const YamlNode* triggerNode = transitionNode->Get("trigger");
            if (srcNode != nullptr && srcNode->GetType() == YamlNode::TYPE_STRING &&
                dstNode != nullptr && dstNode->GetType() == YamlNode::TYPE_STRING &&
                triggerNode != nullptr && triggerNode->GetType() == YamlNode::TYPE_STRING)
            {
                uint32 srcPhase = std::numeric_limits<uint32>::max();
                const YamlNode* srcPhaseNode = transitionNode->Get("src-phase");
                if (srcPhaseNode != nullptr && srcPhaseNode->GetType() == YamlNode::TYPE_STRING)
                    srcPhase = srcPhaseNode->AsUInt32();

                FastName srcName = srcNode->AsFastName();
                FastName dstName = dstNode->AsFastName();
                FastName trigger = triggerNode->AsFastName();

                auto foundSrc = std::find_if(motion->states.begin(), motion->states.end(), [&srcName](const MotionState& state) {
                    return state.GetID() == srcName;
                });

                auto foundDst = std::find_if(motion->states.begin(), motion->states.end(), [&dstName](const MotionState& state) {
                    return state.GetID() == dstName;
                });

                if (foundSrc != motion->states.end() && foundDst != motion->states.end())
                {
                    motion->transitions[t] = MotionTransitionInfo::LoadFromYaml(transitionNode);
                    foundSrc->AddTransition(trigger, &motion->transitions[t], &(*foundDst), srcPhase);
                }
            }
        }
    }

    const YamlNode* rootTransformNode = motionNode->Get("root-transform");
    if (rootTransformNode != nullptr && rootTransformNode->GetType() == YamlNode::TYPE_MAP)
    {
        const YamlNode* rootIDNode = rootTransformNode->Get("root-node");
        if (rootIDNode != nullptr && rootIDNode->GetType() == YamlNode::TYPE_STRING)
        {
            FastName rootID = rootIDNode->AsFastName();
            for (MotionState& state : motion->states)
                state.BindRootNode(rootID);

            motion->rootNodeID = rootID;

            const YamlNode* extractPositionNode = nullptr;
            {
                extractPositionNode = rootTransformNode->Get("extract-position-x");
                if (extractPositionNode != nullptr && extractPositionNode->GetType() == YamlNode::TYPE_STRING && extractPositionNode->AsBool() == true)
                    motion->rootExtractionMask.x = 1.f;

                extractPositionNode = rootTransformNode->Get("extract-position-y");
                if (extractPositionNode != nullptr && extractPositionNode->GetType() == YamlNode::TYPE_STRING && extractPositionNode->AsBool() == true)
                    motion->rootExtractionMask.y = 1.f;

                extractPositionNode = rootTransformNode->Get("extract-position-z");
                if (extractPositionNode != nullptr && extractPositionNode->GetType() == YamlNode::TYPE_STRING && extractPositionNode->AsBool() == true)
                    motion->rootExtractionMask.z = 1.f;
            }

            const YamlNode* resetPositionNode = nullptr;
            {
                resetPositionNode = rootTransformNode->Get("reset-position-x");
                if (resetPositionNode != nullptr && resetPositionNode->GetType() == YamlNode::TYPE_STRING && resetPositionNode->AsBool() == true)
                    motion->rootResetMask.x = 0.f;

                resetPositionNode = rootTransformNode->Get("reset-position-y");
                if (resetPositionNode != nullptr && resetPositionNode->GetType() == YamlNode::TYPE_STRING && resetPositionNode->AsBool() == true)
                    motion->rootResetMask.y = 0.f;

                resetPositionNode = rootTransformNode->Get("reset-position-z");
                if (resetPositionNode != nullptr && resetPositionNode->GetType() == YamlNode::TYPE_STRING && resetPositionNode->AsBool() == true)
                    motion->rootResetMask.z = 0.f;
            }
        }
    }

    motion->parameterIDs.insert(motion->parameterIDs.begin(), statesParameters.begin(), statesParameters.end());

    return motion;
}

} //ns