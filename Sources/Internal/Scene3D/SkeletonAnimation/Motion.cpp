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

ENUM_DECLARE(DAVA::Motion::eTransitionType)
{
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionType::TRANSITION_TYPE_SMOOTH, "Smooth");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionType::TRANSITION_TYPE_FROZEN, "Frozen");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionType::TRANSITION_TYPE_BLENDTREE, "BlendTree");
};

ENUM_DECLARE(DAVA::Motion::eTransitionFunc)
{
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionFunc::TRANSITION_FUNC_LERP, "LERP");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionFunc::TRANSITION_FUNC_CURVE, "Curve");
};

ENUM_DECLARE(DAVA::Motion::eTransitionSync)
{
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionSync::TRANSITION_SYNC_IMMIDIATE, "Immidiate");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionSync::TRANSITION_SYNC_WAIT_END, "WaitEnd");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionSync::TRANSITION_SYNC_WAIT_PHASE_END, "WaitPhaseTime");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionSync::TRANSITION_SYNC_PERCENTAGE, "Percent");
    ENUM_ADD_DESCR(DAVA::Motion::eTransitionSync::TRANSITION_SYNC_PERCENTAGE_INVERSE, "PercentInv");
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
    for (State& s : states)
        SafeDelete(s.blendTree);
}

void Motion::Transition::Reset(State* _srcState, State* _dstState)
{
    srcState = _srcState;
    dstState = _dstState;
    transitionPhase = 0.f;
}

bool Motion::Transition::IsComplete() const
{
    return (transitionPhase >= 1.f) || (duration < EPSILON);
}

void Motion::Transition::Update(float32 dTime, SkeletonPose* outPose)
{
    if (IsComplete())
        return;

    DVASSERT(srcState != nullptr && dstState != nullptr);

    SkeletonPose pose1;
    UpdateAndEvaluateStatePose(dTime, srcState, outPose);
    UpdateAndEvaluateStatePose(dTime, dstState, &pose1);

    transitionPhase += dTime / duration;
    outPose->Lerp(pose1, transitionPhase);
}

void Motion::UpdateAndEvaluateStatePose(float32 dTime, State* state, SkeletonPose* pose)
{
    DVASSERT(state != nullptr);
    DVASSERT(pose != nullptr);

    float32& phase = state->animationPhase;
    uint32& phaseIndex = state->animationPhaseIndex;

    float32 duration = state->blendTree->EvaluatePhaseDuration(phaseIndex, state->boundParams);
    phase += dTime / duration;
    if (phase >= 1.f) //TODO: *Skinning* fix phase calculation on change phaseIndex
    {
        phase -= 1.f;
        ++phaseIndex;
        if (phaseIndex == uint32(state->blendTree->GetPhasesCount()))
            phaseIndex = 0;
    }

    state->blendTree->EvaluatePose(phaseIndex, phase, state->boundParams, pose);
}

void Motion::Update(float32 dTime)
{
    currentPose.Reset();
    if (currentTransition != nullptr)
    {
        currentTransition->Update(dTime, &currentPose);
        if (currentTransition->IsComplete())
            currentTransition = nullptr;
    }

    if (currentTransition == nullptr && currentState != nullptr)
    {
        uint32 prevPhase = currentState->animationPhaseIndex;
        UpdateAndEvaluateStatePose(dTime, currentState, &currentPose);

        if (prevPhase != currentState->animationPhaseIndex)
        {
            //TODO: *Skinning* trigger sync-point event
            if (prevPhase < uint32(currentState->phaseNames.size()))
            {
                FastName phaseName = currentState->phaseNames[prevPhase];
            }
        }
    }
}

void Motion::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (State& s : states)
        s.blendTree->BindSkeleton(skeleton);

    if (currentState != nullptr)
    {
        currentState->animationPhaseIndex = 0u;
        currentState->animationPhase = 0.f;
        currentPose.Reset();
        currentState->blendTree->EvaluatePose(0, 0.f, currentState->boundParams, &currentPose);
    }
}

Motion* Motion::LoadFromYaml(const YamlNode* motionNode)
{
    Motion* motion = new Motion();

    motion->transitions.resize(1);
    motion->transitions.back().duration = 0.5f;

    Set<FastName> parameters;
    int32 enumValue;

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
        for (uint32 s = 0; s < statesCount; ++s)
        {
            State& state = motion->states[s];
            const YamlNode* stateNode = statesNode->Get(s);

            const YamlNode* stateIDNode = stateNode->Get("state-id");
            if (stateIDNode != nullptr && stateIDNode->GetType() == YamlNode::TYPE_STRING)
            {
                state.id = stateIDNode->AsFastName(); //temporary for debug
                motion->statesIDs.emplace_back(stateIDNode->AsFastName());
                motion->statesMap[motion->statesIDs.back()] = &state;
            }

            const YamlNode* blendTreeNode = stateNode->Get("blend-tree");
            if (blendTreeNode != nullptr)
            {
                state.blendTree = BlendTree::LoadFromYaml(blendTreeNode);

                const Vector<FastName>& treeParams = state.blendTree->GetParameterIDs();
                state.boundParams.resize(treeParams.size());

                parameters.insert(treeParams.begin(), treeParams.end());
            }

            const YamlNode* phasesNode = stateNode->Get("phases");
            if (phasesNode != nullptr)
            {
                if (phasesNode->GetType() == YamlNode::TYPE_STRING)
                {
                    state.phaseNames.emplace_back(phasesNode->AsFastName());
                }
                else if (phasesNode->GetType() == YamlNode::TYPE_ARRAY)
                {
                    uint32 phasesCount = phasesNode->GetCount();
                    for (uint32 sp = 0; sp < phasesCount; ++sp)
                    {
                        state.phaseNames.emplace_back(phasesNode->Get(sp)->AsFastName());
                    }
                }
            }
        }

        if (statesCount > 0)
        {
            motion->currentState = motion->states.data();
        }
    }

    const YamlNode* transitionsNode = motionNode->Get("transitions");
    if (transitionsNode != nullptr && transitionsNode->GetType() == YamlNode::TYPE_ARRAY)
    {
        uint32 transitionsCount = transitionsNode->GetCount();
        motion->transitions.resize(transitionsCount);
        for (uint32 t = 0; t < transitionsCount; ++t)
        {
            const YamlNode* transitionNode = transitionsNode->Get(t);
            if (transitionsNode != nullptr)
            {
                Transition& transition = motion->transitions[t];

                const YamlNode* durationNode = transitionNode->Get("duration");
                if (durationNode != nullptr && durationNode->GetType() == YamlNode::TYPE_STRING)
                    transition.duration = durationNode->AsFloat();

                const YamlNode* typeNode = transitionNode->Get("type");
                if (typeNode != nullptr && typeNode->GetType() == YamlNode::TYPE_STRING)
                {
                    if (GlobalEnumMap<Motion::eTransitionType>::Instance()->ToValue(typeNode->AsString().c_str(), enumValue))
                        transition.type = eTransitionType(enumValue);
                }

                const YamlNode* funcNode = transitionNode->Get("func");
                if (funcNode != nullptr && funcNode->GetType() == YamlNode::TYPE_STRING)
                {
                    if (GlobalEnumMap<Motion::eTransitionFunc>::Instance()->ToValue(funcNode->AsString().c_str(), enumValue))
                        transition.func = eTransitionFunc(enumValue);
                }

                const YamlNode* syncNode = transitionNode->Get("sync");
                if (syncNode != nullptr && syncNode->GetType() == YamlNode::TYPE_STRING)
                {
                    if (GlobalEnumMap<Motion::eTransitionSync>::Instance()->ToValue(syncNode->AsString().c_str(), enumValue))
                        transition.sync = eTransitionSync(enumValue);
                }

                const YamlNode* srcNode = transitionNode->Get("src-state");
                const YamlNode* dstNode = transitionNode->Get("dst-state");
                if (srcNode != nullptr && srcNode->GetType() == YamlNode::TYPE_STRING &&
                    dstNode != nullptr && dstNode->GetType() == YamlNode::TYPE_STRING)
                {
                    FastName srcStateID = srcNode->AsFastName();
                    FastName dstStateID = dstNode->AsFastName();

                    auto found = motion->statesMap.find(srcStateID);
                    if (found != motion->statesMap.end())
                        found->second->transitions[dstStateID] = &transition;
                }
            }
        }
    }

    motion->parameterIDs.insert(motion->parameterIDs.begin(), parameters.begin(), parameters.end());

    return motion;
}

bool Motion::BindParameter(const FastName& parameterID, const float32* param)
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
        for (const float32*& param : s.boundParams)
            param = nullptr;
    }
}

const Vector<FastName>& Motion::GetStateIDs() const
{
    return statesIDs;
}

bool Motion::RequestState(const FastName& stateUID)
{
    bool success = false;

    auto foundState = statesMap.find(stateUID);
    if (foundState != statesMap.end())
    {
        State* nextState = foundState->second;

        if (currentState != nullptr)
        {
            nextState->animationPhase = currentState->animationPhase;

            const FastNameMap<Transition*>& transitions = currentState->transitions;
            auto foundTransition = transitions.find(stateUID);
            if (foundTransition != transitions.end())
            {
                Transition* nextTransition = foundTransition->second;
                nextTransition->Reset(currentState, nextState);

                if (currentTransition != nullptr)
                {
                    if (nextTransition->sync == TRANSITION_SYNC_PERCENTAGE)
                        nextTransition->transitionPhase = currentTransition->transitionPhase;
                    else if (nextTransition->sync == TRANSITION_SYNC_PERCENTAGE_INVERSE)
                        nextTransition->transitionPhase = 1.f - currentTransition->transitionPhase;
                }

                currentTransition = nextTransition;
            }
        }

        currentState = foundState->second;
        success = true;
    }
    return success;
}

} //ns