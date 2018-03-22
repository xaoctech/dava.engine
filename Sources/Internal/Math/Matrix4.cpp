#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Math/Matrix4.h"
#include "Math/Quaternion.h"
#include "Logger/Logger.h"

namespace DAVA
{
Matrix4 Matrix4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
                          0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f,
                          0.0f, 0.0f, 0.0f, 1.0f);

Matrix4::Matrix4(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
    rotation.GetMatrix(this);

    _00 *= scale.x;
    _01 *= scale.x;
    _02 *= scale.x;

    _10 *= scale.y;
    _11 *= scale.y;
    _12 *= scale.y;

    _20 *= scale.z;
    _21 *= scale.z;
    _22 *= scale.z;

    _30 = position.x;
    _31 = position.y;
    _32 = position.z;
}

void Matrix4::Dump()
{
    Logger::FrameworkDebug("%5.5f %5.5f %5.5f %5.5f ", _00, _01, _02, _03);
    Logger::FrameworkDebug("%5.5f %5.5f %5.5f %5.5f ", _10, _11, _12, _13);
    Logger::FrameworkDebug("%5.5f %5.5f %5.5f %5.5f ", _20, _21, _22, _23);
    Logger::FrameworkDebug("%5.5f %5.5f %5.5f %5.5f ", _30, _31, _32, _33);
}

void Matrix4::Decomposition(Vector3& position, Vector3& scale, Quaternion& rot) const
{
    scale = GetScaleVector();
    position = GetTranslationVector();

    Matrix4 unscaled(*this);
    for (int32 i = 0; i < 3; ++i)
    {
        unscaled._data[0][i] /= scale.x;
        unscaled._data[1][i] /= scale.y;
        unscaled._data[2][i] /= scale.z;
    }
    rot.Construct(unscaled);
}

Quaternion Matrix4::GetRotation() const
{
    Vector3 scale = GetScaleVector();
    Quaternion rot;

    Matrix4 unscaled(*this);
    for (int32 i = 0; i < 3; ++i)
    {
        unscaled._data[0][i] /= scale.x;
        unscaled._data[1][i] /= scale.y;
        unscaled._data[2][i] /= scale.z;
    }
    rot.Construct(unscaled);

    return rot;
}

void Matrix4::BuildProjection(float32 l, float32 r, float32 b, float32 t, float32 n, float32 f, uint32 flags)
{
    bool orthographic = (flags & ProjectionFlags::Orthographic) == ProjectionFlags::Orthographic;
    bool reverseProjection = (flags & ProjectionFlags::ReverseProjection) == ProjectionFlags::ReverseProjection;
    bool infiniteFarPlane = (flags & ProjectionFlags::InfiniteFarPlane) == ProjectionFlags::InfiniteFarPlane;
    bool zeroBasedClip = (flags & ProjectionFlags::ZeroBasedClipRange) == ProjectionFlags::ZeroBasedClipRange;

    DVASSERT((reverseProjection == false) || ((reverseProjection == true) && zeroBasedClip));
    DVASSERT((infiniteFarPlane == false) || ((infiniteFarPlane == true) && reverseProjection && zeroBasedClip));

    float xSize = r - l;
    float ySize = t - b;
    float zSize = f - n;

    Zero();

    if (orthographic)
    {
        data[0] = 2.0f / xSize;
        data[5] = 2.0f / ySize;
        data[12] = -(r + l) / xSize;
        data[13] = -(t + b) / ySize;
        data[15] = 1.0f;

        if (zeroBasedClip)
        {
            if (reverseProjection)
            {
                data[10] = 1.0f / zSize;
                data[14] = f / zSize;
            }
            else
            {
                data[10] = -1.0f / zSize;
                data[14] = -n / zSize;
            }
        }
        else
        {
            data[10] = -2.0f / zSize;
            data[14] = -(f + n) / zSize;
        }
    }
    else
    {
        data[0] = 2.0f * n / xSize;
        data[5] = 2.0f * n / ySize;
        data[8] = (r + l) / xSize;
        data[9] = (t + b) / ySize;
        data[11] = -1.0f;

        if (zeroBasedClip)
        {
            if (reverseProjection)
            {
                data[10] = infiniteFarPlane ? 0 : (n / zSize);
                data[14] = infiniteFarPlane ? n : (f * n / zSize);
            }
            else
            {
                data[10] = -f / zSize;
                data[14] = -f * n / zSize;
            }
        }
        else
        {
            data[10] = -(f + n) / zSize;
            data[14] = -2.0f * f * n / zSize;
        }
    }
}

template <>
bool AnyCompare<Matrix4>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2)
{
    return v1.Get<Matrix4>() == v2.Get<Matrix4>();
}
}
