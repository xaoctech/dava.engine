#pragma once

#include "MotionTransition.h"

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"

namespace DAVA
{
class Motion;
class MotionState;
class SkeletonPose;

class MotionStateSequence
{
public:
    MotionStateSequence(Motion* motion);
    ~MotionStateSequence();

    void Append(MotionState* state);
    bool CanBeInterrupted(MotionState* state);
    void Interrupt(MotionState* state);

    void Update(float32 dTime);
    void EvaluatePose(SkeletonPose* pose) const;
    void EvaluateRootOffset(Vector3* offset);

    MotionState* GetCurrentState() const;
    MotionState* GetLastState() const;

protected:
    MotionState* GetNextState() const;

    Motion* motion = nullptr;
    MotionTransition* currentTransition = nullptr;
    Vector3 currentRootOffsetDelta;
    List<MotionState*> states;
    bool transitionIsActive = false;
};
} //ns