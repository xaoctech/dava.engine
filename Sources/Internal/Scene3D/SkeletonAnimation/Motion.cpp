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

    float32 duration = state->blendTree->EvaluatePhaseDuration(state->boundParams);

    float32& phase = state->animationPhase;
    phase += dTime / duration;
    if (phase >= 1.f)
        phase -= 1.f;

    state->blendTree->EvaluatePose(phase, state->boundParams, pose);
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
        UpdateAndEvaluateStatePose(dTime, currentState, &currentPose);
    }
}

void Motion::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (State& s : states)
        s.blendTree->BindSkeleton(skeleton);

    if (currentState != nullptr)
    {
        currentState->animationPhase = 0.f;
        currentPose.Reset();
        currentState->blendTree->EvaluatePose(0.f, currentState->boundParams, &currentPose);
    }
}

Motion* Motion::LoadFromYaml(const YamlNode* motionNode)
{
    Motion* motion = new Motion();

    motion->transitions.resize(1);
    motion->transitions.back().duration = 0.5f;

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
            {
                state.id = stateIDNode->AsFastName(); //temporary for debug
                motion->statesIDs.emplace_back(stateIDNode->AsFastName());
                motion->statesMap[motion->statesIDs.back()] = &state;
            }

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
            auto fountTransition = transitions.find(stateUID);
            if (fountTransition != transitions.end())
            {
                Transition* nextTransition = fountTransition->second;
                nextTransition->Reset(currentState, nextState);

                if (currentTransition != nullptr)
                    nextTransition->transitionPhase = 1.f - currentTransition->transitionPhase;

                currentTransition = nextTransition;
            }
        }

        currentState = foundState->second;
        success = true;
    }
    return success;
}

} //ns