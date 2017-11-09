#pragma once

#include "Base/AllocatorFactory.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/Hash.h"
#include "Base/UnordererMap.h"

#include "MotionTransition.h"

namespace DAVA
{
class BlendTree;
class SkeletonComponent;
class SkeletonPose;
class YamlNode;
struct MotionTransitionInfo;

class MotionState
{
public:
    MotionState() = default;
    ~MotionState();

    struct TransitionInfo
    {
        MotionTransitionInfo* info;
        MotionState* state;
    };

    void LoadFromYaml(const YamlNode* stateNode);

    void Reset();
    void Update(float32 dTime);
    void EvaluatePose(SkeletonPose* outPose) const;
    void GetRootOffsetDelta(Vector3* offset) const;
    void SyncPhase(const MotionState* withOther, const MotionTransitionInfo* transitionInfo);

    bool IsAnimationEndReached() const;
    bool IsPhaseEndReached(uint32 phaseIndex) const;
    bool IsMarkerReached(const FastName& marker) const;

    const UnorderedSet<FastName>& GetReachedMarkers() const;

    const FastName& GetID() const;
    const Vector<FastName>& GetBlendTreeParameters() const;

    void BindSkeleton(const SkeletonComponent* skeleton);
    void BindRootNode(const FastName& rootNodeID);

    bool BindParameter(const FastName& parameterID, const float32* param);
    void UnbindParameters();

    void AddTransition(const FastName& trigger, MotionTransitionInfo* transitionInfo, MotionState* dstState, uint32 srcPhase = std::numeric_limits<uint32>::max());
    const TransitionInfo& GetTransitionInfo(const FastName& trigger) const;

protected:
    struct TransitionKey
    {
        TransitionKey(const FastName& _trigger, uint32 _phase = std::numeric_limits<uint32>::max())
            : trigger(_trigger)
            , phase(_phase)
        {
        }

        inline bool operator==(const TransitionKey& other) const
        {
            return trigger == other.trigger && phase == other.phase;
        }

        FastName trigger;
        uint32 phase = std::numeric_limits<uint32>::max();
    };

    struct TransitionKeyHash
    {
        std::size_t operator()(const TransitionKey& key) const
        {
            std::size_t seed = 0;
            HashCombine(seed, key.trigger);
            HashCombine(seed, key.phase);
            return seed;
        }
    };

    FastName id;
    BlendTree* blendTree = nullptr;

    Vector<const float32*> boundParams;

    //TODO: *Skinning* restore markers
    //Vector<FastName> markers;
    UnorderedSet<FastName> reachedMarkers;
    UnorderedMap<TransitionKey, TransitionInfo, TransitionKeyHash> transitions;

    Vector3 rootOffset;
    uint32 animationCurrPhaseIndex = 0u;
    uint32 animationPrevPhaseIndex = 0u;
    float32 animationPhase = 0.f;
    bool animationEndReached = false;
};

inline const FastName& MotionState::GetID() const
{
    return id;
}

inline const UnorderedSet<FastName>& MotionState::GetReachedMarkers() const
{
    return reachedMarkers;
}

inline bool MotionState::IsAnimationEndReached() const
{
    return animationEndReached;
}

inline bool MotionState::IsPhaseEndReached(uint32 phaseIndex) const
{
    return (animationPrevPhaseIndex == phaseIndex) && (animationCurrPhaseIndex != phaseIndex);
}

inline bool MotionState::IsMarkerReached(const FastName& marker) const
{
    return reachedMarkers.count(marker) > 0;
}

} //ns
