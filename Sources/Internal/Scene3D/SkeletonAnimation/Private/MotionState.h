#pragma once

#include "Base/AllocatorFactory.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"

#include "MotionTransition.h"

namespace DAVA
{
class BlendTree;
class MotionTransition;
class SkeletonComponent;
class SkeletonPose;
class YamlNode;
class MotionState
{
public:
    MotionState() = default;
    ~MotionState();

    void LoadFromYaml(const YamlNode* stateNode);

    void Reset();
    bool Update(float32 dTime); //return 'true' if current phase of state is ended
    void EvaluatePose(SkeletonPose* outPose) const;
    void SyncPhase(const MotionState* other);

    const FastName& GetID() const;
    const Vector<FastName>& GetBlendTreeParameters() const;

    //TODO: *Skinning* think about interface or usage
    uint32 GetCurrentPhaseIndex() const;
    uint32 GetPreviousPhaseIndex() const;
    const FastName& GetPhaseName(uint32 phaseIndex) const;

    void BindSkeleton(const SkeletonComponent* skeleton);

    bool BindParameter(const FastName& parameterID, const float32* param);
    void UnbindParameters();

protected:
    FastName id;
    BlendTree* blendTree = nullptr;

    Vector<const float32*> boundParams;
    Vector<FastName> phaseNames;

    uint32 animationCurrPhaseIndex = 0u;
    uint32 animationPrevPhaseIndex = 0u;
    float32 animationPhase = 0.f;
};

inline const FastName& MotionState::GetID() const
{
    return id;
}

inline uint32 MotionState::GetCurrentPhaseIndex() const
{
    return animationCurrPhaseIndex;
}

inline uint32 MotionState::GetPreviousPhaseIndex() const
{
    return animationPrevPhaseIndex;
}

inline const FastName& MotionState::GetPhaseName(uint32 phaseIndex) const
{
    DVASSERT(phaseIndex < uint32(phaseNames.size()));
    return phaseNames[phaseIndex];
}

} //ns
