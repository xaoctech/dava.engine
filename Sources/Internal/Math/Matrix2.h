#pragma once

#include <cmath>
#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Math/MathDefines.h"
#include "Math/MathConstants.h"
#include "Math/Vector.h"

namespace DAVA
{
/**	
	\ingroup math
	\brief Class to work with 2 x 2 matrices.
	It basic purpose to handle rotations in 2D space, but it can be used for any 
	2D transformations that can be performed by such matrix.
 */
struct Matrix2
{
    union
    {
        float32 data[4];
        float32 _data[2][2];
        struct
        {
            float32 _00, _01;
            float32 _10, _11;
        };
    };

    Matrix2();
    Matrix2(float32 m00, float32 m01, float32 m10, float32 m11);
    Matrix2(const Matrix2& m) = default;

    float32 Det() const;

    // Helpers
    void SetIdentity();
    void BuildRotation(float32 angle);
    bool GetInverse(Matrix2& out, float32 tolerance = 1e-06) const;

    Matrix2& operator*=(const Matrix2& arg);
    Matrix2 operator*(const Matrix2& arg) const;

    Matrix2& operator-=(const Matrix2& arg);
    Matrix2 operator-(const Matrix2& arg) const;

    Matrix2& operator+=(const Matrix2& arg);
    Matrix2 operator+(const Matrix2& arg) const;

    Vector2 operator*(const Vector2& arg) const;

    //! Comparison operators
    bool operator==(const Matrix2& _m) const;
    bool operator!=(const Matrix2& _m) const;
};

inline Matrix2::Matrix2()
{
    _00 = 1.0f;
    _01 = 0.0f;
    _10 = 0.0f;
    _11 = 1.0f;
}

inline Matrix2::Matrix2(float32 m00, float32 m01, float32 m10, float32 m11)
{
    data[0] = m00;
    data[1] = m01;
    data[2] = m10;
    data[3] = m11;
};

inline float32 Matrix2::Det() const
{
    return data[0] * data[3] - data[1] * data[2];
}

inline void Matrix2::SetIdentity()
{
    _00 = 1.0f;
    _01 = 0.0f;
    _10 = 0.0f;
    _11 = 1.0f;
}

inline void Matrix2::BuildRotation(float32 angle)
{
    float32 cosA = std::cos(angle);
    float32 sinA = std::sin(angle);

    data[0] = cosA;
    data[1] = sinA;
    data[2] = -sinA;
    data[3] = cosA;
}

inline bool Matrix2::GetInverse(Matrix2& out, float32 tolerance) const
{
    float det = Det();
    if (fabsf(det) <= tolerance)
    {
        return false;
    }

    float invDet = 1.f / det;
    out._00 = _11 * invDet;
    out._01 = -_01 * invDet;
    out._10 = -_10 * invDet;
    out._11 = _00 * invDet;
    return true;
}

inline Matrix2 Matrix2::operator*(const Matrix2& m) const
{
    return Matrix2(_00 * m._00 + _01 * m._10,
                   _00 * m._01 + _01 * m._11,

                   _10 * m._00 + _11 * m._10,
                   _10 * m._01 + _11 * m._11);
}

inline Matrix2& Matrix2::operator*=(const Matrix2& m)
{
    return (*this = *this * m);
}

inline Matrix2 Matrix2::operator+(const Matrix2& m) const
{
    return Matrix2(_00 + m._00, _01 + m._01,
                   _10 + m._10, _11 + m._11);
}

inline Matrix2& Matrix2::operator+=(const Matrix2& m)
{
    return (*this = *this + m);
}

inline Matrix2 Matrix2::operator-(const Matrix2& m) const
{
    return Matrix2(_00 - m._00, _01 - m._01,
                   _10 - m._10, _11 - m._11);
}

inline Vector2 Matrix2::operator*(const Vector2& arg) const
{
    return Vector2(arg.x * _00 + arg.y * _01, arg.x * _10 + arg.y * _11);
}

inline Matrix2& Matrix2::operator-=(const Matrix2& m)
{
    return (*this = *this - m);
}

//! Comparison operators
inline bool Matrix2::operator==(const Matrix2& _m) const
{
    for (uint8 k = 0; k < 4; ++k)
    {
        if (!FLOAT_EQUAL(data[k], _m.data[k]))
        {
            return false;
        }
    }
    return true;
}

inline bool Matrix2::operator!=(const Matrix2& _m) const
{
    return !Matrix2::operator==(_m);
}

template <>
bool AnyCompare<Matrix2>::IsEqual(const DAVA::Any& v1, const DAVA::Any& v2);
extern template struct AnyCompare<Matrix2>;

} // end of namespace DAVA
