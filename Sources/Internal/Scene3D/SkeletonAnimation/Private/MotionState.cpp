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
    anyPhaseEnd = false;
}

void MotionState::Update(float32 dTime)
{
    animationPrevPhaseIndex = animationCurrPhaseIndex;
    anyPhaseEnd = false;

    float32 animationPhase0 = animationPhase;
    uint32 animationCurrPhaseIndex0 = animationCurrPhaseIndex;

    float32 duration = blendTree->EvaluatePhaseDuration(animationCurrPhaseIndex, boundParams);
    animationPhase += dTime / duration;
    if (animationPhase >= 1.f) //TODO: *Skinning* fix phase calculation on change phaseIndex
    {
        animationPhase -= 1.f;
        ++animationCurrPhaseIndex;
        if (animationCurrPhaseIndex == uint32(blendTree->GetPhasesCount()))
            animationCurrPhaseIndex = 0;

        anyPhaseEnd = true;
    }

    blendTree->EvaluateRootOffset(animationCurrPhaseIndex0, animationPhase0, animationCurrPhaseIndex, animationPhase, boundParams, &rootOffset);
}

void MotionState::EvaluatePose(SkeletonPose* outPose) const
{
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

void MotionState::LoadFromYaml(const YamlNode* stateNode)
{
    DVASSERT(stateNode);

    const YamlNode* stateIDNode = stateNode->Get("state-id");
    if (stateIDNode != nullptr && stateIDNode->GetType() == YamlNode::TYPE_STRING)
    {
        id = stateIDNode->AsFastName(); //temporary for debug
    }

    const YamlNode* blendTreeNode = stateNode->Get("blend-tree");
    if (blendTreeNode != nullptr)
    {
        blendTree = BlendTree::LoadFromYaml(blendTreeNode);
        DVASSERT(blendTree != nullptr);

        size_t paramCount = blendTree->GetParameterIDs().size();
        uint32 phasesCount = blendTree->GetPhasesCount();

        boundParams.resize(paramCount);
        phaseNames.resize(phasesCount);
    }

    const YamlNode* phasesNode = stateNode->Get("phases");
    if (phasesNode != nullptr)
    {
        if (phasesNode->GetType() == YamlNode::TYPE_STRING)
        {
            phaseNames[0] = phasesNode->AsFastName();
        }
        else if (phasesNode->GetType() == YamlNode::TYPE_ARRAY)
        {
            uint32 phasesCount = phasesNode->GetCount();
            DVASSERT(phasesCount <= blendTree->GetPhasesCount());

            for (uint32 sp = 0; sp < phasesCount; ++sp)
            {
                phaseNames[sp] = phasesNode->Get(sp)->AsFastName();
            }
        }
    }
}

} //ns