#pragma once

#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Scene3D/SkeletonAnimation/SkeletonPose.h"

namespace DAVA
{
class BlendTree;
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

protected:
    struct State
    {
        FastName id;
        BlendTree* blendTree = nullptr;
        float32 animationPhase = 0.f;

        Vector<const float32*> boundParams;

        //TODO: *Skinning* transitions ?
    };

    FastName name;
    eMotionBlend blendMode = BLEND_COUNT;

    Vector<State> states;
    State* currentState = nullptr;

    Vector<FastName> parameterIDs;

    SkeletonPose currentPose;
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