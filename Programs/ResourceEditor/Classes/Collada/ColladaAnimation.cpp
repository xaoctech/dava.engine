#include "stdafx.h"
#include "ColladaAnimation.h"

namespace DAVA
{
ColladaAnimation::ColladaAnimation()
{
}

ColladaAnimation::~ColladaAnimation()
{
}

void ColladaAnimation::Assign()
{
    for (Map<ColladaSceneNode*, SceneNodeAnimation*>::iterator it = animations.begin(); it != animations.end(); ++it)
    {
        ColladaSceneNode* node = it->first;
        SceneNodeAnimation* anim = it->second;
        node->SetAnimation(anim);
    }
}

Matrix4 ColladaAnimation::EvaluateMatrix(FCDTransform* transform, float32 time)
{
    DVASSERT(transform);
    DVASSERT(transform->GetType() == FCDTransform::MATRIX);
    DVASSERT(transform->IsAnimated());

    FCDAnimated* animated = transform->GetAnimated();
    Matrix4 result;
    for (int32 i = 0; i < 4; ++i)
    {
        for (int32 j = 0; j < 4; ++j)
        {
            FCDAnimationCurve* curve = animated->FindCurve(Format("(%i)(%i)", i, j).c_str());
            result._data[i][j] = (curve != nullptr) ? curve->Evaluate(time) : 0.f;
        }
    }

    return result;
}

Vector3 ColladaAnimation::EvaluateTranslation(FCDTransform* transform, float32 time)
{
    DVASSERT(transform);
    DVASSERT(transform->GetType() == FCDTransform::TRANSLATION);
    DVASSERT(transform->IsAnimated());

    FCDAnimated* animated = transform->GetAnimated();
    FCDAnimationCurve* curveX = animated->FindCurve(".X");
    FCDAnimationCurve* curveY = animated->FindCurve(".Y");
    FCDAnimationCurve* curveZ = animated->FindCurve(".Z");

    Vector3 translation;
    translation.x = (curveX != nullptr) ? curveX->Evaluate(time) : 0.f;
    translation.y = (curveX != nullptr) ? curveY->Evaluate(time) : 0.f;
    translation.z = (curveX != nullptr) ? curveZ->Evaluate(time) : 0.f;

    return translation;
}

Quaternion ColladaAnimation::EvaluateRotation(FCDTransform* transform, float32 time)
{
    DVASSERT(transform);
    DVASSERT(transform->GetType() == FCDTransform::ROTATION);
    DVASSERT(transform->IsAnimated());

    FCDAnimated* animated = transform->GetAnimated();
    FCDAnimationCurve* curveX = animated->FindCurve(".X");
    FCDAnimationCurve* curveY = animated->FindCurve(".Y");
    FCDAnimationCurve* curveZ = animated->FindCurve(".Z");
    FCDAnimationCurve* curveAngle = animated->FindCurve(".ANGLE");

    FCDTRotation* rotation = dynamic_cast<FCDTRotation*>(transform);
    Vector3 transformAxis = rotation->GetAxis();
    float32 transformAngle = rotation->GetAngle();

    Vector3 axis;
    float32 angle;

    axis.x = (curveX != nullptr) ? curveX->Evaluate(time) : transformAxis.x;
    axis.y = (curveY != nullptr) ? curveY->Evaluate(time) : transformAxis.y;
    axis.z = (curveZ != nullptr) ? curveZ->Evaluate(time) : transformAxis.z;
    angle = (curveAngle != nullptr) ? curveAngle->Evaluate(time) : transformAngle;

    if (axis.IsZero() && angle == 0.f)
        return Quaternion();

    return Quaternion::MakeRotationFast(axis, DegToRad(angle));
}

Vector3 ColladaAnimation::EvaluateScale(FCDTransform* transform, float32 time)
{
    DVASSERT(transform);
    DVASSERT(transform->GetType() == FCDTransform::SCALE);
    DVASSERT(transform->IsAnimated());

    FCDAnimated* animated = transform->GetAnimated();
    FCDAnimationCurve* curveX = animated->FindCurve(".X");
    FCDAnimationCurve* curveY = animated->FindCurve(".Y");
    FCDAnimationCurve* curveZ = animated->FindCurve(".Z");

    Vector3 scale;
    scale.x = (curveX != nullptr) ? curveX->Evaluate(time) : 1.f;
    scale.y = (curveX != nullptr) ? curveY->Evaluate(time) : 1.f;
    scale.z = (curveX != nullptr) ? curveZ->Evaluate(time) : 1.f;

    return scale;
}

void ColladaAnimation::CollectAnimationKeys(FCDAnimationCurve* curve, TimeStampSet* timeStamps)
{
    if (curve == nullptr)
        return;

    size_t keyCount = curve->GetKeyCount();
    FCDAnimationKey** keys = curve->GetKeys();
    for (size_t k = 0; k < keyCount; ++k)
        timeStamps->insert(keys[k]->input);
}

void ColladaAnimation::CollectAnimationKeys(FCDSceneNode* node, ColladaAnimatinData* data)
{
    TimeStampSet translationKeys;
    TimeStampSet rotationKeys;
    TimeStampSet scaleKeys;

    for (size_t t = 0; t < node->GetTransformCount(); ++t)
    {
        FCDTransform* transform = node->GetTransform(t);
        if (transform->IsAnimated())
        {
            FCDAnimated* animated = transform->GetAnimated();

            if (transform->GetType() == FCDTransform::MATRIX)
            {
                FCDAnimationCurve* mxCurves[4][4] = {};
                for (int32 i = 0; i < 4; ++i)
                {
                    for (int32 j = 0; j < 4; ++j)
                    {
                        mxCurves[i][j] = animated->FindCurve(Format("(%i)(%i)", i, j).c_str());
                        CollectAnimationKeys(mxCurves[i][j], &translationKeys);
                        CollectAnimationKeys(mxCurves[i][j], &rotationKeys);
                        CollectAnimationKeys(mxCurves[i][j], &scaleKeys);
                    }
                }
            }
            else
            {
                FCDAnimationCurve* curveX = animated->FindCurve(".X");
                FCDAnimationCurve* curveY = animated->FindCurve(".Y");
                FCDAnimationCurve* curveZ = animated->FindCurve(".Z");

                if (transform->GetType() == FCDTransform::TRANSLATION)
                {
                    CollectAnimationKeys(curveX, &translationKeys);
                    CollectAnimationKeys(curveY, &translationKeys);
                    CollectAnimationKeys(curveZ, &translationKeys);
                }
                else if (transform->GetType() == FCDTransform::ROTATION)
                {
                    CollectAnimationKeys(curveX, &rotationKeys);
                    CollectAnimationKeys(curveY, &rotationKeys);
                    CollectAnimationKeys(curveZ, &rotationKeys);

                    FCDAnimationCurve* curveAngle = animated->FindCurve(".ANGLE");
                    CollectAnimationKeys(curveAngle, &rotationKeys);
                }
                else if (transform->GetType() == FCDTransform::SCALE)
                {
                    CollectAnimationKeys(curveX, &scaleKeys);
                    CollectAnimationKeys(curveY, &scaleKeys);
                    CollectAnimationKeys(curveZ, &scaleKeys);
                }
            }
        }
    }

    for (float32 key : translationKeys)
        data->translations.emplace_back(key, Vector3());

    for (float32 key : rotationKeys)
        data->rotations.emplace_back(key, Quaternion());

    for (float32 key : scaleKeys)
        data->scales.emplace_back(key, Vector3(1.f, 1.f, 1.f));
}

void ColladaAnimation::ExportAnimationData(FCDSceneNode* originalNode, ColladaAnimatinData* data)
{
    CollectAnimationKeys(originalNode, data);

    for (size_t t = 0; t < originalNode->GetTransformCount(); ++t)
    {
        FCDTransform* transform = originalNode->GetTransform(t);
        if (transform->IsAnimated())
        {
            FCDAnimated* animated = transform->GetAnimated();

            if (transform->GetType() == FCDTransform::MATRIX)
            {
                Matrix4 mxTransform;

                for (auto& t : data->translations)
                    t.second += EvaluateMatrix(transform, t.first).GetTranslationVector();

                for (auto& r : data->rotations)
                    r.second *= EvaluateMatrix(transform, r.first).GetRotation();

                for (auto& s : data->scales)
                    s.second *= EvaluateMatrix(transform, s.first).GetScaleVector();
            }
            else
            {
                if (transform->GetType() == FCDTransform::TRANSLATION)
                {
                    for (auto& t : data->translations)
                        t.second += EvaluateTranslation(transform, t.first);
                }
                else if (transform->GetType() == FCDTransform::ROTATION)
                {
                    for (auto& r : data->rotations)
                        r.second *= EvaluateRotation(transform, r.first);
                }
                else if (transform->GetType() == FCDTransform::SCALE)
                {
                    for (auto& s : data->scales)
                        s.second *= EvaluateScale(transform, s.first);
                }
            }
        }
    }
}
};
