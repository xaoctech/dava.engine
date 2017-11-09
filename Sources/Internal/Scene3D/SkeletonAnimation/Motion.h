#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"
#include "Base/UnordererMap.h"
#include "Reflection/Reflection.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"
#include "Scene3D/Components/SkeletonComponent.h"

#include "Private/MotionState.h"
#include "Private/MotionTransition.h"

namespace DAVA
{
class BlendTree;
class YamlNode;
class Motion;
class MotionState;
struct MotionTransitionInfo;

class Motion
{
    Motion() = default;

public:
    const static FastName STATE_ANIMATION_END_MARKER;

    enum eMotionBlend
    {
        BLEND_OVERRIDE,
        BLEND_ADD,
        BLEND_DIFF,
        BLEND_LERP,

        BLEND_COUNT
    };

    static Motion* LoadFromYaml(const YamlNode* motionNode);

    const FastName& GetName() const;
    eMotionBlend GetBlendMode() const;
    const SkeletonPose& GetCurrentSkeletonPose() const;
    const Vector3& GetCurrentRootOffsetDelta() const;

    void TriggerEvent(const FastName& trigger); //TODO: *Skinning* make adequate naming

    void Update(float32 dTime);

    void BindSkeleton(const SkeletonComponent* skeleton);

    const Vector<FastName>& GetParameterIDs() const;
    bool BindParameter(const FastName& parameterID, const float32* param);
    bool UnbindParameter(const FastName& parameterID);
    void UnbindParameters();

    const Vector<FastName>& GetStateIDs() const;

    const Vector<std::pair<FastName, FastName>> GetEndedPhases() const;

protected:
    FastName name;
    eMotionBlend blendMode = BLEND_COUNT;

    Vector<MotionState> states;
    Vector<MotionTransitionInfo> transitions;

    Vector<FastName> statesIDs;
    Vector<FastName> parameterIDs;

    MotionTransition stateTransition; //transition from 'current' to 'next' state
    MotionState* currentState = nullptr;
    MotionState* nextState = nullptr;
    MotionState* pendingState = nullptr;
    MotionTransitionInfo* pendingTransition = nullptr;

    Vector3 currentRootOffsetDelta;
    Vector3 rootExtractionMask = Vector3(0.f, 0.f, 0.f);
    Vector3 rootResetMask = Vector3(1.f, 1.f, 1.f);

    FastName rootNodeID;
    uint32 rootNodeJointIndex = SkeletonComponent::INVALID_JOINT_INDEX;

    SkeletonPose currentPose;
    Vector<std::pair<FastName, FastName>> endedPhases; /*[state-id, phase-id]*/

    DAVA_REFLECTION(Motion);
};

inline const FastName& Motion::GetName() const
{
    return name;
}

inline Motion::eMotionBlend Motion::GetBlendMode() const
{
    return blendMode;
}

inline const SkeletonPose& Motion::GetCurrentSkeletonPose() const
{
    return currentPose;
}

inline const Vector3& Motion::GetCurrentRootOffsetDelta() const
{
    return currentRootOffsetDelta;
}

inline const Vector<FastName>& Motion::GetParameterIDs() const
{
    return parameterIDs;
}

inline const Vector<std::pair<FastName, FastName>> Motion::GetEndedPhases() const
{
    return endedPhases;
}

} //ns