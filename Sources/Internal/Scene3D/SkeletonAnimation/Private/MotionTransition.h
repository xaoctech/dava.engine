#pragma once

#include "Base/AllocatorFactory.h"
#include "Animation/Interpolation.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

namespace DAVA
{
class MotionState;
class SkeletonPose;
class YamlNode;

struct MotionTransitionInfo
{
public:
    IMPLEMENT_POOL_ALLOCATOR(MotionTransitionInfo, 128);

    enum eType : uint8
    {
        TYPE_REPLACE,
        TYPE_CROSS_FADE,
        TYPE_FROZEN_FADE,
        TYPE_STATE,

        TYPE_COUNT
    };
    enum eSync : uint8
    {
        SYNC_IMMIDIATE,
        SYNC_WAIT_END,
        SYNC_WAIT_PHASE_END,

        SYNC_COUNT
    };

    static MotionTransitionInfo* LoadFromYaml(const YamlNode* transitionNode, const FastNameMap<MotionState*>& states);

    eType type = TYPE_COUNT;
    eSync sync = SYNC_COUNT;

    Interpolation::Func func;
    float32 duration = 0.f;
    FastName waitPhaseID;
    MotionState* transitionState = nullptr;
    bool syncPhases = false;
};

class MotionTransition
{
public:
    MotionTransition() = default;

    void Reset(const MotionTransitionInfo* transitionInfo, MotionState* srcState, MotionState* dstState);

    void Update(float32 dTime);
    void EvaluatePose(SkeletonPose* outPose) const;

    bool IsComplete() const;
    bool IsStarted() const;

    bool CanBeInterrupted(const MotionTransitionInfo* other, const MotionState* dstState) const;
    void Interrupt(const MotionTransitionInfo* other, MotionState* dstState);

    MotionState* GetDstState() const;
    MotionState* GetSrcState() const;

    void SetSrcState(MotionState* state);

protected:
    const MotionTransitionInfo* transitionInfo = nullptr;
    MotionState* srcState = nullptr;
    MotionState* dstState = nullptr;

    SkeletonPose frozenPose;
    float32 transitionPhase = 0.f;
    bool started = false;
    bool srcFrozen = false;
    bool inversed = false;
};

inline bool MotionTransition::IsComplete() const
{
    DVASSERT(transitionInfo != nullptr);
    return IsStarted() && ((transitionPhase >= 1.f) || (transitionInfo->duration < EPSILON) || (transitionInfo->type == MotionTransitionInfo::TYPE_REPLACE));
}

inline bool MotionTransition::IsStarted() const
{
    return started;
}

} //ns