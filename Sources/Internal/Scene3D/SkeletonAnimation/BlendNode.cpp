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
    }
    else
    {
        for (auto& c : children)
            SafeDelete(c.first);
    }
}

BlendNode::BlendNode(AnimationClip* animationClip)
{
    DVASSERT(animationClip != nullptr);

    type = TYPE_ANIMATION;
    animation = new SkeletonAnimation(animationClip);
}

void BlendNode::AddChild(BlendNode* node, Vector2 point)
{
    DVASSERT(type != TYPE_ANIMATION);
    children.emplace_back(std::make_pair(node, point));
}

void BlendNode::BindSkeleton(const SkeletonComponent* skeleton)
{
    if (type == TYPE_ANIMATION)
    {
        animation->BindSkeleton(skeleton);
    }
    else
    {
        for (auto& child : children)
            child.first->BindSkeleton(skeleton);
    }
}

void BlendNode::EvaluatePose(SkeletonPose* outPose, float32 phase) const
{
    switch (type)
    {
    case TYPE_ANIMATION:
    {
        animation->EvaluatePose(outPose, phase);
    }
    break;
    case TYPE_LERP_1D:
    {
        if (children.size() == 2)
        {
            BlendNode* child0 = children[0].first;
            BlendNode* child1 = children[1].first;
            float32 param0 = children[0].second.x;
            float32 param1 = children[1].second.x;

            SkeletonPose pose0, pose1;
            child0->EvaluatePose(&pose0, phase);
            child1->EvaluatePose(&pose1, phase);

            float32 factor = parameter.x / (param1 - param0);
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

float32 BlendNode::EvaluatePhaseDuration() const
{
    switch (type)
    {
    case TYPE_ANIMATION:
    {
        return animation->GetPhaseDuration();
    }
    break;
    case TYPE_LERP_1D:
    {
        if (children.size() == 2)
        {
            BlendNode* child0 = children[0].first;
            BlendNode* child1 = children[1].first;
            float32 param0 = children[0].second.x;
            float32 param1 = children[1].second.x;

            float32 dur0 = child0->EvaluatePhaseDuration();
            float32 dur1 = child1->EvaluatePhaseDuration();

            float32 factor = parameter.x / (param1 - param0);
            return Lerp(dur0, dur1, factor);
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

    return 1.f;
}

} //ns