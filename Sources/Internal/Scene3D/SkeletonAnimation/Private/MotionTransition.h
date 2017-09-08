#pragma once

#include "Base/AllocatorFactory.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

namespace DAVA
{
class MotionState;
class SkeletonPose;
class YamlNode;
class MotionTransition
{
public:
    IMPLEMENT_POOL_ALLOCATOR(MotionTransition, 128);

    enum eType : uint8
    {
        TYPE_CROSS_FADE,
        TYPE_FROZEN_FADE,
        TYPE_BLENDTREE,

        TYPE_COUNT
    };
    enum eFunc : uint8
    {
        FUNC_LINEAR,
        FUNC_CURVE,

        FUNC_COUNT
    };
    enum eSync : uint8
    {
        SYNC_IMMIDIATE,
        SYNC_WAIT_END,
        SYNC_WAIT_PHASE_END,

        SYNC_COUNT
    };

    static MotionTransition* LoadFromYaml(const YamlNode* transitionNode);

    void SetStates(MotionState* srcState, MotionState* dstState);
    const MotionState* GetSrcState() const;
    const MotionState* GetDstState() const;

    void Reset();
    void Update(float32 dTime);
    void EvaluatePose(SkeletonPose* outPose) const;
    bool IsComplete() const;
    bool IsStarted() const;

    bool CanInterrupt(const MotionTransition* other) const;
    void Interrupt(const MotionTransition* other);

protected:
    MotionTransition() = default;

    eType type = TYPE_COUNT;
    eFunc func = FUNC_COUNT;
    eSync sync = SYNC_COUNT;
    float32 duration = 0.f;
    FastName waitPhaseID;
    bool syncPhases = false;

    //runtime
    SkeletonPose frozenPose;
    MotionState* srcState = nullptr;
    MotionState* dstState = nullptr;
    float32 transitionPhase = 0.f;
    bool started = false;
    bool srcFrozen = false;
};

inline void MotionTransition::SetStates(MotionState* _srcState, MotionState* _dstState)
{
    srcState = _srcState;
    dstState = _dstState;
}

inline const MotionState* MotionTransition::GetSrcState() const
{
    return srcState;
}

inline const MotionState* MotionTransition::GetDstState() const
{
    return dstState;
}

inline bool MotionTransition::IsComplete() const
{
    return (transitionPhase >= 1.f) || (duration < EPSILON);
}

inline bool MotionTransition::IsStarted() const
{
    return started;
}

} //ns