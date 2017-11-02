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

    reachedMarkers.clear();
    animationEndReached = false;

    animationPrevPhaseIndex = animationCurrPhaseIndex;

    float32 animationPhase0 = animationPhase;
    uint32 animationCurrPhaseIndex0 = animationCurrPhaseIndex;

    float32 duration = blendTree->EvaluatePhaseDuration(animationCurrPhaseIndex, boundParams);
    animationPhase += (duration != 0.f) ? (dTime / duration) : 0.f;
    if (animationPhase >= 1.f)
    {
        animationPhase -= 1.f; //TODO: *Skinning* fix phase calculation on change phaseIndex

        if (animationCurrPhaseIndex < uint32(markers.size()) && !markers[animationCurrPhaseIndex].empty())
            reachedMarkers.insert(markers[animationCurrPhaseIndex]);

        ++animationCurrPhaseIndex;
        if (animationCurrPhaseIndex == uint32(blendTree->GetPhasesCount()))
        {
            animationCurrPhaseIndex = 0;
            animationEndReached = true;
        }
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

void MotionState::SyncPhase(const MotionState* other)
{
    DVASSERT(blendTree->GetPhasesCount() == other->blendTree->GetPhasesCount());

    animationPrevPhaseIndex = other->animationPrevPhaseIndex;
    animationCurrPhaseIndex = other->animationCurrPhaseIndex;
    animationPhase = other->animationPhase;
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
        markers.resize(blendTree->GetPhasesCount());
    }

    const YamlNode* markersNode = stateNode->Get("markers");
    if (markersNode != nullptr)
    {
        if (markersNode->GetType() == YamlNode::TYPE_STRING)
        {
            if (!markers.empty())
            {
                markers[0] = markersNode->AsFastName();
            }
        }
        else if (markersNode->GetType() == YamlNode::TYPE_ARRAY)
        {
            uint32 markersCount = Min(markersNode->GetCount(), uint32(markers.size()));
            for (uint32 mi = 0; mi < markersCount; ++mi)
            {
                FastName marker = markersNode->Get(mi)->AsFastName();
                if (!marker.empty() && marker != FastName(""))
                    markers[mi] = marker;
            }
        }
    }
}

} //ns