#include "Math/TransformUtils.h"

namespace DAVA
{
const Transform TransformUtils::IDENTITY;

Vector3 TransformUtils::TransformVector(const Transform& transform, const Vector3& vector)
{
    return transform.GetRotation().ApplyToVectorFast(transform.GetScale() * vector) + transform.GetTranslation();
}

Transform TransformUtils::Inverse(const Transform& transform)
{
    Transform invertedTransform(transform);
    invertedTransform.Inverse();
    return invertedTransform;
}

Matrix4 TransformUtils::ToMatrix(const Transform& transform)
{
    Matrix4 m = Matrix4::MakeScale(transform.GetScale()) * transform.GetRotation().GetMatrix();
    m.SetTranslationVector(transform.GetTranslation());
    return m;
}
}
