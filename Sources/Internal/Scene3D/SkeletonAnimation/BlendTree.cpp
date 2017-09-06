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
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_ADD, "Add");
    ENUM_ADD_DESCR(DAVA::BlendTree::eNodeType::TYPE_DIFF, "Diff");
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

void BlendTree::EvaluatePose(uint32 phaseIndex, float32 phase, const Vector<const float32*>& parameters, SkeletonPose* outPose) const
{
    DVASSERT(phaseIndex < phasesCount);

    EvaluateRecursive(phaseIndex, phase, nodes.front(), parameters, outPose, nullptr);
}

float32 BlendTree::EvaluatePhaseDuration(uint32 phaseIndex, const Vector<const float32*>& parameters) const
{
    DVASSERT(phaseIndex < phasesCount);

    float32 duration = 1.f;
    EvaluateRecursive(phaseIndex, 0.f, nodes.front(), parameters, nullptr, &duration);
    return duration;
}

void BlendTree::EvaluateRecursive(uint32 phaseIndex, float32 phase, const BlendNode& node, const Vector<const float32*>& parameters, SkeletonPose* outPose, float32* outPhaseDuration) const
{
    switch (node.type)
    {
    case TYPE_ANIMATION:
    {
        int32 animationIndex = node.animData.animationIndex;
        if (animationIndex != -1)
        {
            const BlendTree::Animation& animation = animations[node.animData.animationIndex];

            float32 phaseStart = (phaseIndex > 0) ? animation.phaseEnds[phaseIndex - 1] : 0.f;
            float32 phaseEnd = animation.phaseEnds[phaseIndex];

            if (outPose != nullptr)
            {
                float32 animationLocalTime = (phaseStart + phase * (phaseEnd - phaseStart)) * animation.skeletonAnimation->GetDuration();
                animation.skeletonAnimation->EvaluatePose(animationLocalTime, outPose);
            }

            if (outPhaseDuration != nullptr)
            {
                *outPhaseDuration = (phaseEnd - phaseStart) * animation.skeletonAnimation->GetDuration();
            }
        }
    }
    break;
    case TYPE_LERP_1D:
    {
        const BlendNode::BlendData& blendData = node.blendData;
        int32 childrenCount = blendData.endChildIndex - blendData.beginChildIndex;
        if (childrenCount == 1)
        {
            EvaluateRecursive(phaseIndex, phase, nodes[blendData.beginChildIndex], parameters, outPose, outPhaseDuration);
        }
        else
        {
            float32 parameter = (parameters[blendData.parameterIndex] != nullptr) ? *parameters[blendData.parameterIndex] : 0.f;

            int32 c = blendData.beginChildIndex;
            for (; c < blendData.endChildIndex; ++c)
            {
                if (nodes[c].coord.x >= parameter)
                    break;
            }

            if (c == blendData.beginChildIndex)
            {
                EvaluateRecursive(phaseIndex, phase, nodes[blendData.beginChildIndex], parameters, outPose, outPhaseDuration);
            }
            else if (c == blendData.endChildIndex)
            {
                EvaluateRecursive(phaseIndex, phase, nodes[blendData.endChildIndex - 1], parameters, outPose, outPhaseDuration);
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
                    SkeletonPose pose1;
                    EvaluateRecursive(phaseIndex, phase, child0, parameters, outPose, nullptr);
                    EvaluateRecursive(phaseIndex, phase, child1, parameters, &pose1, nullptr);
                    outPose->Lerp(pose1, factor);
                }

                if (outPhaseDuration != nullptr)
                {
                    float32 dur0, dur1;
                    EvaluateRecursive(phaseIndex, phase, child0, parameters, nullptr, &dur0);
                    EvaluateRecursive(phaseIndex, phase, child1, parameters, nullptr, &dur1);
                    *outPhaseDuration = Lerp(dur0, dur1, factor);
                }
            }
        }
    }
    break;
    case TYPE_LERP_2D:
    {
        //TODO: *Skinning*
        DVASSERT(false);
    }
    break;
    case TYPE_ADD:
    {
        const BlendNode::BlendData& blendData = node.blendData;
        DVASSERT((blendData.endChildIndex - blendData.beginChildIndex) == 2);
        if (outPose != nullptr)
        {
            SkeletonPose pose1;
            EvaluateRecursive(phaseIndex, phase, nodes[blendData.beginChildIndex], parameters, outPose, nullptr);
            EvaluateRecursive(phaseIndex, phase, nodes[blendData.beginChildIndex + 1], parameters, &pose1, nullptr);
            outPose->Add(pose1);
        }
    }
    break;
    case TYPE_DIFF:
    {
        const BlendNode::BlendData& blendData = node.blendData;
        DVASSERT((blendData.endChildIndex - blendData.beginChildIndex) == 2);
        if (outPose != nullptr)
        {
            SkeletonPose pose1;
            EvaluateRecursive(phaseIndex, phase, nodes[blendData.beginChildIndex], parameters, outPose, nullptr);
            EvaluateRecursive(phaseIndex, phase, nodes[blendData.beginChildIndex + 1], parameters, &pose1, nullptr);
            outPose->Diff(pose1);
        }
    }
    break;
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

    const YamlNode* paramNode = yamlNode->Get("param-value");
    if (paramNode != nullptr)
    {
        if (paramNode->GetType() == YamlNode::TYPE_ARRAY)
            node.coord = paramNode->AsVector2();
        else if (paramNode->GetType() == YamlNode::TYPE_STRING)
            node.coord.x = paramNode->AsFloat();
    }

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
            BlendTree::Animation& animation = animations.back();

            animation.skeletonAnimation = new SkeletonAnimation(animationClip);

            uint32 markerCount = animationClip->GetMarkerCount();
            if (markerCount != 0)
            {
                for (uint32 m = 0; m < markerCount; ++m)
                {
                    DVASSERT(animation.phaseEnds.empty() || animation.phaseEnds.back() < animationClip->GetMarkerTime(m));
                    animation.phaseEnds.push_back(animationClip->GetMarkerTime(m));
                }
            }
            else
            {
                const YamlNode* syncPointsNode = yamlNode->Get("sync-points");
                if (syncPointsNode != nullptr)
                {
                    if (syncPointsNode->GetType() == YamlNode::TYPE_STRING)
                    {
                        animation.phaseEnds.push_back(syncPointsNode->AsFloat());
                    }
                    else if (syncPointsNode->GetType() == YamlNode::TYPE_ARRAY)
                    {
                        uint32 pointsCount = syncPointsNode->GetCount();
                        for (uint32 sp = 0; sp < pointsCount; ++sp)
                        {
                            const YamlNode* syncPointNode = syncPointsNode->Get(sp);
                            if (syncPointNode->GetType() == YamlNode::TYPE_STRING)
                            {
                                DVASSERT(animation.phaseEnds.empty() || animation.phaseEnds.back() < syncPointNode->AsFloat());
                                animation.phaseEnds.push_back(syncPointNode->AsFloat());
                            }
                        }
                    }
                }
            }

            if (animation.phaseEnds.empty() || (!animation.phaseEnds.empty() && !FLOAT_EQUAL(animation.phaseEnds.back(), 1.f)))
                animation.phaseEnds.push_back(1.f);

            DVASSERT(animation.phaseEnds.front() > EPSILON);

            //All animation should have the same phases count
            //TODO: *Skinning* return error, not assert
            DVASSERT(phasesCount == 0 || phasesCount == uint32(animation.phaseEnds.size()));
            phasesCount = uint32(animation.phaseEnds.size());
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
                        uint32 childBegin = uint32(nodes.size());
                        uint32 childEnd = childBegin + childrenCount;

                        blendData.beginChildIndex = int32(childBegin);
                        blendData.endChildIndex = int32(childEnd);

                        //Hint: 'blendData' and 'node' should be filled at this point. After 'nodes.resize()' references is invalid.
                        nodes.resize(childEnd);
                        for (uint32 c = 0; c < childrenCount; ++c)
                        {
                            const YamlNode* childNode = childrenNode->Get(c);
                            LoadBlendNodeRecursive(childNode, blendTree, childBegin + c);
                        }

                        if (nodes[nodeIndex].type == TYPE_LERP_1D)
                        {
                            std::sort(nodes.data() + childBegin, nodes.data() + childEnd, [](const BlendNode& l, const BlendNode& r) {
                                return l.coord.x < r.coord.x;
                            });
                        }
                    }
                }
            }
        }
    }
}

} //ns