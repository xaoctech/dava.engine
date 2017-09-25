#include "MotionStateSequence.h"

namespace DAVA
{
MotionStateSequence::MotionStateSequence(Motion* _motion)
    : motion(_motion)
{
    DVASSERT(motion);
    currentTransition = new MotionTransition();
}

MotionStateSequence::~MotionStateSequence()
{
    SafeDelete(currentTransition);
}

void MotionStateSequence::Append(MotionState* state)
{
    state->Reset();
    states.push_back(state);
}

bool MotionStateSequence::CanBeInterrupted(MotionState* state)
{
    if (states.size() < 2)
        return true;

    if (transitionIsActive)
    {
        MotionState* currentState = GetCurrentState();
        MotionTransitionInfo* nextTransitionInfo0 = motion->GetTransition(currentState, state);

        MotionState* nextState = GetNextState();
        MotionTransitionInfo* nextTransitionInfo1 = motion->GetTransition(nextState, state);

        return (nextTransitionInfo0 != nullptr && currentTransition->CanBeInterrupted(nextTransitionInfo0, currentState, state))
        || (nextTransitionInfo1 != nullptr && currentTransition->CanBeInterrupted(nextTransitionInfo1, nextState, state));
    }

    if (states.front() == state)
        return true;

    return false;
}

void MotionStateSequence::Interrupt(MotionState* state)
{
    DVASSERT(CanBeInterrupted(state));

    if (states.size() < 2)
    {
        Append(state);
    }
    else if (transitionIsActive)
    {
        MotionState* currentState = GetCurrentState();
        MotionTransitionInfo* nextTransitionInfo0 = motion->GetTransition(currentState, state);

        MotionState* nextState = GetNextState();
        MotionTransitionInfo* nextTransitionInfo1 = motion->GetTransition(nextState, state);

        if (nextTransitionInfo0 != nullptr && currentTransition->CanBeInterrupted(nextTransitionInfo0, currentState, state))
        {
            currentTransition->Interrupt(nextTransitionInfo0, currentState, state);
            states = { currentState, state };
            state->Reset();
        }
        else if (nextTransitionInfo1 != nullptr && currentTransition->CanBeInterrupted(nextTransitionInfo1, nextState, state))
        {
            currentTransition->Interrupt(nextTransitionInfo1, nextState, state);
            states = { nextState, state };
            state->Reset();
        }
    }
    else
    {
        DVASSERT(states.front() == state);
        states.erase(++states.begin(), states.end());
    }
}

void MotionStateSequence::Update(float32 dTime)
{
    MotionState* currentState = GetCurrentState();

    currentState->Update(dTime);
    currentState->GetRootOffsetDelta(&currentRootOffsetDelta);

    MotionState* nextState = GetNextState();
    MotionTransitionInfo* transitionInfo = motion->GetTransition(currentState, nextState);

    if (states.size() > 1 && !transitionIsActive)
    {
        if (transitionInfo != nullptr)
        {
            DVASSERT(transitionInfo->type != MotionTransitionInfo::TYPE_STATE);

            currentTransition->Reset(transitionInfo, currentState, nextState);
            transitionIsActive = true;
        }
        else
        {
            if (currentState->IsAnimationEnd())
            {
                states.pop_front();
            }
        }
    }

    if (transitionIsActive)
    {
        currentTransition->GetDstState()->Update(dTime);
        currentTransition->Update(dTime);

        if (currentTransition->IsComplete())
        {
            currentTransition->EvaluateRootOffset(&currentRootOffsetDelta);
            states.pop_front();
            transitionIsActive = false;
        }
    }
}

void MotionStateSequence::EvaluatePose(SkeletonPose* pose) const
{
    if (transitionIsActive)
    {
        currentTransition->EvaluatePose(pose);
    }
    else
    {
        GetCurrentState()->EvaluatePose(pose);
    }
}

void MotionStateSequence::EvaluateRootOffset(Vector3* offset)
{
    if (transitionIsActive)
    {
        currentTransition->EvaluateRootOffset(offset);
    }
    else
    {
        *offset = currentRootOffsetDelta;
    }
}

MotionState* MotionStateSequence::GetCurrentState() const
{
    return (states.empty()) ? nullptr : states.front();
}

MotionState* MotionStateSequence::GetLastState() const
{
    return (states.empty()) ? nullptr : states.back();
}

MotionState* MotionStateSequence::GetNextState() const
{
    return (states.size() > 1) ? *(++states.begin()) : states.front();
}

} //ns