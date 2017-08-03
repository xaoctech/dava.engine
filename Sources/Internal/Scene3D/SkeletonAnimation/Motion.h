#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

namespace DAVA
{
class YamlNode;
class BlendNode;
class AnimationClip;
class Motion
{
    Motion() = default;

public:
    enum eBlend
    {
        BLEND_OVERRIDE,
        BLEND_ADD,
        BLEND_SUB,
        BLEND_LERP,

        BLEND_COUNT
    };

    ~Motion();

    static Motion* LoadFromYaml(const YamlNode* motionNode);

    const FastName& GetName() const;
    eBlend GetBlendMode() const;

    uint32 GetParametersCount() const;
    const FastName& GetParameterID(uint32 index) const;
    uint32 GetParameterIndex(const FastName& parameterID) const;

    void SetParameter(uint32 index, float32 value);
    void SetParameter(uint32 index, Vector2 value);

    void BindSkeleton(const SkeletonComponent* skeleton);

    void Update(float32 dTime);

    const SkeletonPose& GetCurrentSkeletonPose() const;

protected:
    BlendNode* LoadBlendNodeFromYaml(const YamlNode* yamlNode);
    void AddParametrizedNode(BlendNode* node, const FastName& parameter);

    struct State
    {
        FastName id;
        BlendNode* treeRoot = nullptr;

        //transitions ?
    };

    FastName name;
    eBlend blendMode = BLEND_COUNT;

    Vector<State> states;
    State* currentState = nullptr;

    Vector<FastName> parametersIDs;
    Vector<Vector<BlendNode*>> parametrizedNodes;
    //Vector<std::pair<BlendNode*, AnimationClip*>> animationNodes;

    SkeletonPose resultPose;

    float32 animationPhase = 0.f;
};

inline const FastName& Motion::GetName() const
{
    return name;
}

inline Motion::eBlend Motion::GetBlendMode() const
{
    return blendMode;
}

inline uint32 Motion::GetParametersCount() const
{
    return uint32(parametersIDs.size());
}

inline const FastName& Motion::GetParameterID(uint32 index) const
{
    DVASSERT(index < GetParametersCount());
    return parametersIDs[index];
}

inline uint32 Motion::GetParameterIndex(const FastName& parameterID) const
{
    auto found = std::find(parametersIDs.begin(), parametersIDs.end(), parameterID);
    return (found != parametersIDs.end()) ? uint32(std::distance(parametersIDs.begin(), found)) : std::numeric_limits<uint32>::max();
}

inline void Motion::SetParameter(uint32 index, float32 value)
{
    SetParameter(index, Vector2(value, 0.f));
}

inline const SkeletonPose& Motion::GetCurrentSkeletonPose() const
{
    return resultPose;
}

} //ns