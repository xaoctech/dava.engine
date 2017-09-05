#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastNameMap.h"
#include "Reflection/Reflection.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

namespace DAVA
{
class BlendTree;
class SkeletonComponent;
class YamlNode;
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
    enum eTransitionType : uint8
    {
        TRANSITION_TYPE_SMOOTH,
        TRANSITION_TYPE_FROZEN,
        TRANSITION_TYPE_BLENDTREE,

        TRANSITION_TYPE_COUNT
    };
    enum eTransitionFunc : uint8
    {
        TRANSITION_FUNC_LERP,
        TRANSITION_FUNC_CURVE,

        TRANSITION_FUNC_COUNT
    };
    enum eTransitionSync : uint8
    {
        TRANSITION_SYNC_IMMIDIATE,
        TRANSITION_SYNC_WAIT_END,
        TRANSITION_SYNC_WAIT_PHASE_END,
        TRANSITION_SYNC_PERCENTAGE,
        TRANSITION_SYNC_PERCENTAGE_INVERSE,

        TRANSITION_SYNC_COUNT
    };

    ~Motion();

    static Motion* LoadFromYaml(const YamlNode* motionNode);

    const FastName& GetName() const;
    eMotionBlend GetBlendMode() const;
    const SkeletonPose& GetCurrentSkeletonPose() const;

    void BindSkeleton(const SkeletonComponent* skeleton);
    void Update(float32 dTime);

    const Vector<FastName>& GetParameterIDs() const;
    bool BindParameter(const FastName& parameterID, const float32* param);
    bool UnbindParameter(const FastName& parameterID);
    void UnbindParameters();

    const Vector<FastName>& GetStateIDs() const;
    bool RequestState(const FastName& stateID);

protected:
    struct State;
    class Transition
    {
    public:
        void Update(float32 dTime, SkeletonPose* outPose);
        void Reset(State* srcState, State* dstState);

        bool IsComplete() const;

    protected:
        eTransitionType type = TRANSITION_TYPE_COUNT;
        eTransitionFunc func = TRANSITION_FUNC_COUNT;
        eTransitionSync sync = TRANSITION_SYNC_COUNT;
        float32 duration = 0.f;

        //runtime
        State* srcState = nullptr;
        State* dstState = nullptr;
        float32 transitionPhase = 0.f;

        friend class Motion;
    };
    struct State
    {
        FastName id; //temporary for debug
        BlendTree* blendTree = nullptr;
        FastNameMap<Transition*> transitions;

        uint32 animationPhaseIndex = 0u;
        float32 animationPhase = 0.f;
        Vector<const float32*> boundParams;

        //TODO: *Skinning* move it to BlendTree ?
        Vector<FastName> phaseNames;
    };

    static void UpdateAndEvaluateStatePose(float32 dTime, State* state, SkeletonPose* pose);

    FastName name;
    eMotionBlend blendMode = BLEND_COUNT;

    Vector<State> states;
    FastNameMap<State*> statesMap;
    State* currentState = nullptr;

    Vector<Transition> transitions;
    Transition* currentTransition = nullptr;

    Vector<FastName> statesIDs;
    Vector<FastName> parameterIDs;

    SkeletonPose currentPose;

    //////////////////////////////////////////////////////////////////////////
    //temporary for debug
    const FastName& GetStateID() const
    {
        static const FastName invalidID = FastName("#invalid-state");
        return (currentState != nullptr) ? currentState->id : invalidID;
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