#include "BlendNode.h"
#include "SkeletonPose.h"
#include "SkeletonAnimation.h"

#include "Animation/AnimationClip.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/GlobalEnum.h"

ENUM_DECLARE(DAVA::BlendNode::eType)
{
    ENUM_ADD_DESCR(DAVA::BlendNode::eType::TYPE_ANIMATION, "Animation");
    ENUM_ADD_DESCR(DAVA::BlendNode::eType::TYPE_LERP_1D, "LERP1D");
    ENUM_ADD_DESCR(DAVA::BlendNode::eType::TYPE_LERP_2D, "LERP2D");
    ENUM_ADD_DESCR(DAVA::BlendNode::eType::TYPE_ADD, "Additive");
    ENUM_ADD_DESCR(DAVA::BlendNode::eType::TYPE_SUB, "Subtract");
};

namespace DAVA
{
BlendNode::BlendNode(eType _type)
    : type(_type)
{
    DVASSERT(type != TYPE_ANIMATION);
}

BlendNode::~BlendNode()
{
    if (type == TYPE_ANIMATION)
    {
        SafeDelete(animation);
        SafeRelease(animationClip);
    }
    else
    {
        for (auto& c : children)
            SafeDelete(c.first);
    }
}

BlendNode::BlendNode(AnimationClip* _animationClip)
    : animationClip(SafeRetain(_animationClip))
{
    DVASSERT(animationClip != nullptr);

    type = TYPE_ANIMATION;
    animation = new SkeletonAnimation();
}

void BlendNode::AddChild(BlendNode* node, Vector2 point)
{
    DVASSERT(type != TYPE_ANIMATION);
    children.emplace_back(std::make_pair(node, point));
}

void BlendNode::BindSkeleton(const SkeletonComponent* skeleton)
{
    if (type == TYPE_ANIMATION && animation)
    {
        animation->BindAnimation(animationClip, skeleton);
    }
    else
    {
        for (auto& child : children)
            child.first->BindSkeleton(skeleton);
    }
}

void BlendNode::Evaluate(SkeletonPose* outPose, float32 nTime)
{
    switch (type)
    {
    case TYPE_ANIMATION:
    {
        animation->EvaluatePose(outPose, nTime * animationClip->GetDuration());
    }
    break;
    case TYPE_LERP_1D:
    {
        if (children.size() == 2)
        {
            float32 factor = parameter.x / (children[1].second.x - children[0].second.x);

            SkeletonPose pose0, pose1;
            children[0].first->Evaluate(&pose0, nTime);
            children[1].first->Evaluate(&pose1, nTime);

            *outPose = SkeletonPose::Lerp(pose0, pose1, factor);
        }
    }
    break;
    case TYPE_LERP_2D:
    {
    }
    break;
    case TYPE_ADD:
    {
    }
    break;
    case TYPE_SUB:
    {
    }
    break;
    default:
        break;
    }
}

} //ns