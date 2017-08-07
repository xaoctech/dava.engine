#ifndef __DAVAENGINE_COLLADAANIMATION_H__
#define __DAVAENGINE_COLLADAANIMATION_H__

#include "ColladaIncludes.h"
#include "ColladaSceneNode.h"

namespace DAVA
{
class ColladaAnimation
{
public:
    struct ColladaAnimatinData
    {
        Vector<std::pair<float32, Vector3>> translations; // [time, translation]
        Vector<std::pair<float32, Quaternion>> rotations; // [time, rotation]
        Vector<std::pair<float32, Vector3>> scales; // [time, scale]
    };

    ColladaAnimation();
    ~ColladaAnimation();

    std::string name;
    float32 duration;
    Map<ColladaSceneNode*, SceneNodeAnimation*> animations;
    Map<ColladaSceneNode*, ColladaAnimatinData> animationsData;

    void Assign();

    static void ExportAnimationData(FCDSceneNode* originalNode, ColladaAnimatinData* data);

protected:
    struct ColladaAnimatinTimeCmp
    {
        bool operator()(float32 l, float32 r)
        {
            return (!FLOAT_EQUAL(l, r)) ? (l < r) : false;
        }
    };
    using TimeStampSet = Set<float32, ColladaAnimatinTimeCmp>;

    static Matrix4 EvaluateMatrix(FCDTransform* transform, float32 time);
    static Vector3 EvaluateScale(FCDTransform* transform, float32 time);
    static Vector3 EvaluateTranslation(FCDTransform* transform, float32 time);
    static Quaternion EvaluateRotation(FCDTransform* transform, float32 time);

    static void CollectAnimationKeys(FCDAnimationCurve* curve, TimeStampSet* timeStamps);
    static void CollectAnimationKeys(FCDSceneNode* node, ColladaAnimatinData* data);
};
};
					 
#endif // __DAVAENGINE_COLLADAANIMATION_H__
