#include "MotionState.h"
#include "MotionTransition.h"

#include "FileSystem/YamlNode.h"
#include "Scene3D/SkeletonAnimation/BlendTree.h"

namespace DAVA
{
MotionState::~MotionState()
{
    SafeDelete(blendTree);
}

void MotionState::Reset()
{
    rootOffset = Vector3();
    animationPrevPhaseIndex = 0;
    animationCurrPhaseIndex = 0;
    animationPhase = 0.f;
}

void MotionState::Update(float32 dTime)
{
    if (blendTree == nullptr)
        return;

    animationEndReached = false;

    animationPrevPhaseIndex = animationCurrPhaseIndex;

    float32 animationPhase0 = animationPhase;
    uint32 animationCurrPhaseIndex0 = animationCurrPhaseIndex;

    float32 duration = blendTree->EvaluatePhaseDuration(animationCurrPhaseIndex, boundParams);
    animationPhase += (duration != 0.f) ? (dTime / duration) : 0.f;
    if (animationPhase >= 1.f)
    {
        animationPhase -= 1.f;

        ++animationCurrPhaseIndex;
        if (animationCurrPhaseIndex == blendTree->GetPhasesCount())
        {
            animationCurrPhaseIndex = 0;
            animationEndReached = true;
        }

        float32 nextPhaseDuration = blendTree->EvaluatePhaseDuration(animationCurrPhaseIndex, boundParams);
        animationPhase = (nextPhaseDuration != 0.f) ? (animationPhase * duration / nextPhaseDuration) : animationPhase;
        animationPhase = Clamp(animationPhase, 0.f, 1.f);
    }

    blendTree->EvaluateRootOffset(animationCurrPhaseIndex0, animationPhase0, animationCurrPhaseIndex, animationPhase, boundParams, &rootOffset);
}

void MotionState::EvaluatePose(SkeletonPose* outPose) const
{
    if (blendTree != nullptr)
        blendTree->EvaluatePose(animationCurrPhaseIndex, animationPhase, boundParams, outPose);
}

void MotionState::GetRootOffsetDelta(Vector3* offset) const
{
    *offset = rootOffset;
}

void MotionState::SyncPhase(const MotionState* other, const MotionTransitionInfo* transitionInfo)
{
    DVASSERT(transitionInfo != nullptr);

    const Vector<uint32>& phaseMap = transitionInfo->phaseMap;
    if (!phaseMap.empty())
    {
        animationPrevPhaseIndex = (other->animationPrevPhaseIndex < phaseMap.size()) ? phaseMap[other->animationPrevPhaseIndex] : animationPrevPhaseIndex;
        animationCurrPhaseIndex = (other->animationCurrPhaseIndex < phaseMap.size()) ? phaseMap[other->animationCurrPhaseIndex] : animationCurrPhaseIndex;

        DVASSERT(animationPrevPhaseIndex < blendTree->GetPhasesCount());
        DVASSERT(animationCurrPhaseIndex < blendTree->GetPhasesCount());
    }

    if (transitionInfo->syncPhase)
    {
        animationPhase = other->animationPhase;
    }

    if (transitionInfo->inversePhase)
    {
        animationPhase = 1.f - animationPhase;
    }
}

const Vector<FastName>& MotionState::GetBlendTreeParameters() const
{
    static Vector<FastName> empty;
    return (blendTree != nullptr) ? blendTree->GetParameterIDs() : empty;
}

void MotionState::BindSkeleton(const SkeletonComponent* skeleton)
{
    if (blendTree != nullptr)
        blendTree->BindSkeleton(skeleton);
}

void MotionState::BindRootNode(const FastName& rootNodeID)
{
    if (blendTree != nullptr)
        blendTree->BindRootNode(rootNodeID);
}

bool MotionState::BindParameter(const FastName& parameterID, const float32* param)
{
    bool success = false;
    if (blendTree != nullptr)
    {
        const Vector<FastName>& params = blendTree->GetParameterIDs();
        auto found = std::find(params.begin(), params.end(), parameterID);
        if (found != params.end())
        {
            size_t paramIndex = std::distance(params.begin(), found);
            boundParams[paramIndex] = param;
            success = true;
        }
    }

    return success;
}

void MotionState::UnbindParameters()
{
    std::fill(boundParams.begin(), boundParams.end(), nullptr);
}

void MotionState::AddTransitionState(const FastName& trigger, MotionState* dstState)
{
    transitions[trigger] = dstState;
}

MotionState* MotionState::GetTransitionState(const FastName& trigger) const
{
    auto found = transitions.find(trigger);
    if (found != transitions.end())
        return found->second;

    return nullptr;
}

void MotionState::LoadFromYaml(const YamlNode* stateNode)
{
    DVASSERT(stateNode);

    const YamlNode* stateIDNode = stateNode->Get("state-id");
    if (stateIDNode != nullptr && stateIDNode->GetType() == YamlNode::TYPE_STRING)
    {
        id = stateIDNode->AsFastName();
    }

    const YamlNode* blendTreeNode = stateNode->Get("blend-tree");
    if (blendTreeNode != nullptr)
    {
        blendTree = BlendTree::LoadFromYaml(blendTreeNode);
        DVASSERT(blendTree != nullptr);

        boundParams.resize(blendTree->GetParameterIDs().size());
    }
}

} //ns