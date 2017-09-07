#pragma once

#include "Base/AllocatorFactory.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

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
    enum eInterruption : uint8
    {
        INTERRUPT_PHASE_INVERSE,

        INTERRUPT_COUNT
    };

    static MotionTransition* LoadFromYaml(const YamlNode* transitionNode);

    void SetStates(MotionState* srcState, MotionState* dstState);

    void Reset();
    void Update(float32 dTime, SkeletonPose* outPose);
    bool IsComplete() const;

    bool CanInterrupt(const MotionTransition* other) const;
    void Interrupt(MotionTransition* other);

protected:
    MotionTransition() = default;

    eType type = TYPE_COUNT;
    eFunc func = FUNC_COUNT;
    eSync sync = SYNC_COUNT;
    eInterruption interruption = INTERRUPT_PHASE_INVERSE;
    float32 duration = 0.f;
    FastName waitPhaseID;
    bool syncPhases = false;

    //runtime
    MotionState* srcState = nullptr;
    MotionState* dstState = nullptr;
    float32 transitionPhase = 0.f;
    bool started = false;
};
} //ns