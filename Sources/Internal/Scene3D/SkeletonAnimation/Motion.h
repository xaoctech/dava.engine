#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastNameMap.h"
#include "Reflection/Reflection.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

#include "Private/MotionState.h"
#include "Private/MotionTransition.h"

namespace DAVA
{
class BlendTree;
class SkeletonComponent;
class YamlNode;
class MotionState;
class MotionTransition;
class Motion
{
    Motion() = default;

public:
    enum eMotionBlend
    {
        BLEND_OVERRIDE,
        BLEND_ADD,
        BLEND_DIFF,
        BLEND_LERP,

        BLEND_COUNT
    };

    ~Motion();

    static Motion* LoadFromYaml(const YamlNode* motionNode);

    const FastName& GetName() const;
    eMotionBlend GetBlendMode() const;
    const SkeletonPose& GetCurrentSkeletonPose() const;

    void BindSkeleton(const SkeletonComponent* skeleton);
    void Update(float32 dTime, Vector<std::pair<FastName, FastName>>* outEndedPhases = nullptr /*[state-id, phase-id]*/);

    const Vector<FastName>& GetParameterIDs() const;
    bool BindParameter(const FastName& parameterID, const float32* param);
    bool UnbindParameter(const FastName& parameterID);
    void UnbindParameters();

    const Vector<FastName>& GetStateIDs() const;
    bool RequestState(const FastName& stateID);

protected:
    uint32 GetTransitionIndex(MotionState* srcState, MotionState* dstState);

    FastName name;
    eMotionBlend blendMode = BLEND_COUNT;

    Vector<MotionState> states;
    FastNameMap<MotionState*> statesMap;
    MotionState* currentState = nullptr;

    Vector<MotionTransition*> transitions;
    List<MotionTransition*> activeTransitions;

    Vector<FastName> statesIDs;
    Vector<FastName> parameterIDs;

    SkeletonPose currentPose;

    //////////////////////////////////////////////////////////////////////////
    //temporary for debug
    const FastName& GetStateID() const
    {
        static const FastName invalidID = FastName("#invalid-state");
        return (currentState != nullptr) ? currentState->GetID() : invalidID;
    }
    void SetStateID(const FastName& id)
    {
        RequestState(id);
    }
    //////////////////////////////////////////////////////////////////////////

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

inline const Vector<FastName>& Motion::GetParameterIDs() const
{
    return parameterIDs;
}

} //ns