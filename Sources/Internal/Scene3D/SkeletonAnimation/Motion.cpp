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

bool Motion::RequestState(const FastName& stateUID)
{
    bool success = false;

    //TODO: *Skinning* rethink state/transition queue ?
    auto foundState = statesMap.find(stateUID);
    if (foundState != statesMap.end())
    {
        MotionState* nextState = foundState->second;
        if (currentState != nullptr)
        {
            if (currentState != nextState)
            {
                if (currentTransiton != nullptr && !currentTransiton->IsStarted())
                {
                    if (currentTransiton->GetSrcState() == nextState)
                    {
                        //discard not started transition
                        currentTransiton = nullptr;
                        currentState = nextState;
                        stateQueue.clear();
                    }
                    else
                    {
                        MotionTransition* nextTransition = GetTransition(currentTransiton->GetSrcState(), nextState);
                        if (nextTransition != nullptr)
                        {
                            //replace not started transition
                            currentTransiton = nextTransition;
                            currentTransiton->Reset();
                            currentState = nextState;
                            stateQueue.clear();
                        }
                    }
                }
                else
                {
                    MotionTransition* nextTransition = GetTransition(currentState, nextState);
                    if (currentTransiton != nullptr && nextTransition->CanInterrupt(currentTransiton))
                    {
                        //interrupt started transition if can
                        nextTransition->Reset();
                        nextTransition->Interrupt(currentTransiton);
                        currentTransiton = nextTransition;

                        currentState = nextState;
                        stateQueue.clear();
                    }
                    else
                    {
                        //search state in queue that we can replace
                        auto found = std::find_if(stateQueue.begin(), stateQueue.end(), [this, &nextState](MotionState* state) {
                            return (this->GetTransition(state, nextState) != nullptr || state == nextState);
                        });

                        if (found != stateQueue.end())
                        {
                            //replace state in queue
                            *found = nextState;
                            stateQueue.erase(++found, stateQueue.end());
                        }
                        else
                        {
                            //add state to queue
                            stateQueue.push_back(nextState);
                        }
                    }
                }
            }
            else
            {
                stateQueue.clear();
            }
        }
        else
        {
            currentState = nextState;
        }

        success = true;
    }

    return success;
}

void Motion::Update(float32 dTime, Vector<std::pair<FastName, FastName>>* outEndedPhases)
{
    currentPose.Reset();
    if (currentTransiton != nullptr)
    {
        currentTransiton->Update(dTime);
        currentTransiton->EvaluatePose(&currentPose);
        if (currentTransiton->IsComplete())
            currentTransiton = nullptr;
    }
    else
    {
        currentState->Update(dTime);

        if (currentState->IsPhaseEnd() && outEndedPhases != nullptr)
        {
            const FastName& endedPhaseName = currentState->GetLastPhaseName();
            if (endedPhaseName.IsValid())
                outEndedPhases->emplace_back(currentState->GetID(), endedPhaseName);
        }

        currentState->EvaluatePose(&currentPose);
    }

    if (currentTransiton == nullptr && currentState != nullptr)
    {
        if (!stateQueue.empty())
        {
            currentTransiton = GetTransition(currentState, stateQueue.front());
            currentTransiton->Reset();

            currentState = stateQueue.front();
            stateQueue.pop_front();
        }
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

MotionTransition* Motion::GetTransition(const MotionState* srcState, const MotionState* dstState) const
{
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