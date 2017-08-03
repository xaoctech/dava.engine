#pragma once

#include "SkeletonPose.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/FastName.h"

namespace DAVA
{
class AnimationClip;
class SkeletonAnimation;
class BlendNode
{
public:
    enum eType : uint8
    {
        TYPE_ANIMATION,
        TYPE_LERP_1D,
        TYPE_LERP_2D,
        TYPE_ADD,
        TYPE_SUB,

        TYPE_COUNT
    };

    BlendNode(eType nodeType);
    BlendNode(AnimationClip* animationClip);
    ~BlendNode();

    void AddChild(BlendNode* node, Vector2 point);

    void BindSkeleton(const SkeletonComponent* skeleton);
    void Evaluate(SkeletonPose* outPose, float32 time);

    void SetParameter(Vector2 value);

protected:
    Vector<std::pair<BlendNode*, Vector2>> children;

    SkeletonAnimation* animation = nullptr;
    AnimationClip* animationClip = nullptr;

    Vector2 parameter;
    eType type = TYPE_COUNT;
};

inline void BlendNode::SetParameter(Vector2 value)
{
    parameter = value;
}

} //ns