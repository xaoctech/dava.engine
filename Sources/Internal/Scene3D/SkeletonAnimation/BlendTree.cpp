#include "BlendTree.h"
#include "SkeletonPose.h"
#include "SkeletonAnimation.h"

#include "Animation/AnimationClip.h"
#include "Base/BaseMath.h"
#include "Base/BaseTypes.h"
#include "Base/GlobalEnum.h"
#include "FileSystem/YamlNode.h"

ENUM_DECLARE(DAVA::BlendTree::eNodeType)
{
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_ANIMATION, "Animation");
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_LERP_1D, "LERP1D");
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_LERP_2D, "LERP2D");
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_ADD, "Additive");
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_SUB, "Subtract");
};

namespace DAVA
{
BlendTree::~BlendTree()
{
    for (Animation& a : animations)
        SafeDelete(a.skeletonAnimation);
}

void BlendTree::BindSkeleton(const SkeletonComponent* skeleton)
{
    for (const Animation& a : animations)
        a.skeletonAnimation->BindSkeleton(skeleton);
}

void BlendTree::EvaluatePose(float32 phase, const Vector<const Vector2*>& parameters, SkeletonPose* outPose) const
{
    EvaluateRecursive(phase, nodes.front(), parameters, outPose, nullptr);
}

float32 BlendTree::EvaluatePhaseDuration(const Vector<const Vector2*>& parameters) const
{
    float32 duration = 1.f;
    EvaluateRecursive(0.f, nodes.front(), parameters, nullptr, &duration);
    return duration;
}

void BlendTree::EvaluateRecursive(float32 phase, const BlendNode& node, const Vector<const Vector2*>& parameters, SkeletonPose* outPose, float32* outPhaseDuration) const
{
    switch (node.type)
    {
    case TYPE_ANIMATION:
    {
        int32 animationIndex = node.animData.animationIndex;
        if (animationIndex != -1)
        {
            SkeletonAnimation* skeletonAnimation = animations[node.animData.animationIndex].skeletonAnimation;

            if (outPose != nullptr)
                skeletonAnimation->EvaluatePose(phase, outPose);

            if (outPhaseDuration != nullptr)
                *outPhaseDuration = skeletonAnimation->GetPhaseDuration();
        }
    }
    break;
    case TYPE_LERP_1D:
    {
        const BlendNode::BlendData& blendData = node.blendData;
        int32 childrenCount = blendData.endChildIndex - blendData.beginChildIndex;
        if (childrenCount == 1)
        {
            EvaluateRecursive(phase, nodes[blendData.beginChildIndex], parameters, outPose, outPhaseDuration);
        }
        else
        {
            float32 parameter = (parameters[blendData.parameterIndex] != nullptr) ? parameters[blendData.parameterIndex]->x : 0.f;

            int32 c = blendData.beginChildIndex;
            for (; c < blendData.endChildIndex; ++c)
            {
                if (nodes[c].coord.x >= parameter)
                    break;
            }

            if (c == blendData.beginChildIndex)
            {
                EvaluateRecursive(phase, nodes[blendData.beginChildIndex], parameters, outPose, outPhaseDuration);
            }
            else if (c == blendData.endChildIndex)
            {
                EvaluateRecursive(phase, nodes[blendData.endChildIndex - 1], parameters, outPose, outPhaseDuration);
            }
            else
            {
                const BlendNode& child0 = nodes[c - 1];
                const BlendNode& child1 = nodes[c];
                float32 coord0 = child0.coord.x;
                float32 coord1 = child1.coord.x;

                float32 factor = (parameter - coord0) / (coord1 - coord0);
                if (outPose != nullptr)
                {
                    SkeletonPose pose0, pose1;
                    EvaluateRecursive(phase, child0, parameters, &pose0, nullptr);
                    EvaluateRecursive(phase, child1, parameters, &pose1, nullptr);
                    *outPose = SkeletonPose::Lerp(pose0, pose1, factor);
                }

                if (outPhaseDuration != nullptr)
                {
                    float32 dur0, dur1;
                    EvaluateRecursive(phase, child0, parameters, nullptr, &dur0);
                    EvaluateRecursive(phase, child1, parameters, nullptr, &dur1);
                    *outPhaseDuration = Lerp(dur0, dur1, factor);
                }
            }
        }
    }
    break;
    case TYPE_LERP_2D:
    case TYPE_ADD:
    case TYPE_SUB:
    default:
        break;
    }
}

//////////////////////////////////////////////////////////////////////////

BlendTree* BlendTree::LoadFromYaml(const YamlNode* yamlNode)
{
    BlendTree* blendTree = new BlendTree();
    blendTree->nodes.emplace_back();
    blendTree->LoadBlendNodeRecursive(yamlNode, blendTree, 0);

    return blendTree;
}

void BlendTree::LoadBlendNodeRecursive(const YamlNode* yamlNode, BlendTree* blendTree, uint32 nodeIndex)
{
    Vector<BlendNode>& nodes = blendTree->nodes;
    BlendNode& node = nodes[nodeIndex];

    const YamlNode* clipNode = yamlNode->Get("clip");
    if (clipNode != nullptr && clipNode->GetType() == YamlNode::TYPE_STRING)
    {
        node.type = eNodeType::TYPE_ANIMATION;
        node.animData.animationIndex = -1;

        FilePath animationClipPath(clipNode->AsString());
        ScopedPtr<AnimationClip> animationClip(AnimationClip::Load(animationClipPath));
        if (animationClip)
        {
            node.animData.animationIndex = int32(animations.size());

            animations.emplace_back();
            animations.back().skeletonAnimation = new SkeletonAnimation(animationClip);
        }
    }
    else
    {
        BlendNode::BlendData& blendData = node.blendData;
        blendData.parameterIndex = -1;

        blendData.beginChildIndex = -1;
        blendData.endChildIndex = -1;

        const YamlNode* operationNode = yamlNode->Get("operation");
        if (operationNode != nullptr && operationNode->GetType() == YamlNode::TYPE_MAP)
        {
            const YamlNode* typeNode = operationNode->Get("type");
            if (typeNode != nullptr && typeNode->GetType() == YamlNode::TYPE_STRING)
            {
                int32 nodeType;
                if (GlobalEnumMap<eNodeType>::Instance()->ToValue(typeNode->AsString().c_str(), nodeType))
                {
                    node.type = eNodeType(nodeType);

                    const YamlNode* parameterNode = operationNode->Get("parameter");
                    if (parameterNode != nullptr && parameterNode->GetType() == YamlNode::TYPE_STRING)
                    {
                        FastName parameterID = parameterNode->AsFastName();
                        auto found = std::find(parameterIDs.begin(), parameterIDs.end(), parameterID);

                        blendData.parameterIndex = int32(std::distance(parameterIDs.begin(), found));
                        if (found == parameterIDs.end())
                            parameterIDs.emplace_back(parameterID);
                    }

                    const YamlNode* childrenNode = yamlNode->Get("nodes");
                    if (childrenNode != nullptr && childrenNode->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        uint32 childrenCount = childrenNode->GetCount();
                        uint32 child0 = uint32(nodes.size());

                        blendData.beginChildIndex = child0;
                        blendData.endChildIndex = child0 + int32(childrenCount);

                        //Hint: 'blendData' should be filled at this point. After 'nodes.resize()' reference is invalid.
                        nodes.resize(nodes.size() + childrenCount);
                        for (uint32 c = 0; c < childrenCount; ++c)
                        {
                            const YamlNode* childNode = childrenNode->Get(c);
                            LoadBlendNodeRecursive(childNode, blendTree, child0 + c);
                        }
                    }
                }
            }
        }
    }

    const YamlNode* paramNode = yamlNode->Get("param-value");
    if (paramNode != nullptr)
    {
        if (paramNode->GetType() == YamlNode::TYPE_ARRAY)
            node.coord = paramNode->AsVector2();
        else if (paramNode->GetType() == YamlNode::TYPE_STRING)
            node.coord.x = paramNode->AsFloat();
    }
}

} //ns