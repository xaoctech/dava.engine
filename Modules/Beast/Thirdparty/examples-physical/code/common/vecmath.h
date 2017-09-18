/*
Copyright 2012 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * Vector math function for Beast API examples
 * Not intended to be complete, just a set of help function
 * to solve the problems in the samples
 */ 
#ifndef VECMATH_H
#define VECMATH_H

#include <beastapi/beastapitypes.h>
#define _USE_MATH_DEFINES // for M_PI
#include <cmath>
#include <cstdlib>
#include <stdexcept>

namespace bex
{
/**
	 * Wrapper for the 2 vector.
	 * Note, this class relies on the fact that
	 * there are no virtual functions in it.
	 * if you add one, things will go boom
	 */
class Vec2 : public ILBVec2
{
public:
    Vec2(float _x, float _y)
        : ILBVec2(_x, _y)
    {
    }
    Vec2()
    {
    }
};

/**
	 * Wrapper for the 3 vector
	 * Note, this class relies on the fact that
	 * there are no virtual functions in it.
	 * if you add one, things will go boom
	 */
class Vec3 : public ILBVec3
{
public:
    Vec3(float _x, float _y, float _z)
        : ILBVec3(_x, _y, _z)
    {
    }
    Vec3()
        : ILBVec3(0.0f, 0.0f, 0.0f)
    {
    }
    const Vec3& operator+=(const Vec3& v)
    {
        this->x += v.x;
        this->y += v.y;
        this->z += v.z;
        return *this;
    }
};

/**
	 * Cross product for 3 vectors
	 */
inline Vec3 cross(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x);
}

/**
	 * Dot product for 3 vectors
	 */
inline float dot(const Vec3& v1, const Vec3& v2)
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

/**
	 * Add operator for 3 vectors
	 */
inline Vec3 operator+(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

/**
	 * Subtract operator for 3 vectors
	 */
inline Vec3 operator-(const Vec3& v1, const Vec3& v2)
{
    return Vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

/**
	 * Scalar multiply for 3 vectors
	 */
inline Vec3 operator*(const Vec3& v, float f)
{
    return Vec3(v.x * f,
                v.y * f,
                v.z * f);
}

/**
	 * Negation for 3 vectors
	 */
inline Vec3 operator-(const Vec3& v)
{
    return Vec3(-v.x,
                -v.y,
                -v.z);
}
/**
	 * Normalize function for 3 vectors. Note
	 * it returns a normalized version, rather than
	 * normalizing what's sent to it.
	 */
inline Vec3 normalize(const Vec3& v1)
{
    float lenRec = 1.0f / sqrtf(dot(v1, v1));
    return v1 * lenRec;
}

/**
	 * Wrapper for the 4x4 matrix
	 * Note, this class relies on the fact that
	 * there are no virtual functions in it.
	 * if you add one, things will go boom
	 */
class Matrix4x4 : public ILBMatrix4x4
{
public:
    /**
		* Sets the column col to be the vector v in the matrix m
		* note that the 4 component in the column will stay untouched
		*/
    void setColumn(const Vec3& v, int col)
    {
        if (col < 0 || col > 3)
        {
            throw std::runtime_error("Invalid matrix column");
        }
        m[col] = v.x;
        m[col + 4] = v.y;
        m[col + 8] = v.z;
    }

    /**
		* Sets the row row to be the vector v in the matrix m
		* note that the 4 component in the row will stay untouched
		*/
    void setRow(const Vec3& v, int row)
    {
        if (row < 0 || row > 3)
        {
            throw std::runtime_error("Invalid matrix row");
        }
        m[row * 4] = v.x;
        m[row * 4 + 1] = v.y;
        m[row * 4 + 2] = v.z;
    }
};

/**
	 * Sets an identity matrix
	 */
inline Matrix4x4 identity()
{
    Matrix4x4 result;
    for (int i = 0; i < 16; ++i)
    {
        // Check if we are on the diagonal
        if ((i & 3) == (i / 4))
        {
            result.m[i] = 1.0f;
        }
        else
        {
            result.m[i] = 0.0f;
        }
    }
    return result;
}

/**
	 * Sets a translation matrix
	 */
inline Matrix4x4 translation(const Vec3& pos)
{
    Matrix4x4 result = identity();
    result.m[3] = pos.x;
    result.m[7] = pos.y;
    result.m[11] = pos.z;
    return result;
}

/**
	 * Sets a scaling matrix
	 */
inline Matrix4x4 scale(const Vec3& scale)
{
    Matrix4x4 result = identity();
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

/**
	 * Matrix multiplication
	 */
inline Matrix4x4 operator*(const Matrix4x4& m1, const Matrix4x4& m2)
{
    Matrix4x4 result;
    for (int i = 0; i < 4; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            result.m[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                result.m[i * 4 + j] += m1.m[i * 4 + k] * m2.m[k * 4 + j];
            }
        }
    }
    return result;
}

/**
	 * Transforms the point (as opposed to a vector) p using 
	 * the matrix m.
	 * Treats it as M * p where p is a column point
	 */
inline Vec3 mulPoint(const Matrix4x4& m, const Vec3& p)
{
    return Vec3(m.m[0 + 0] * p.x + m.m[0 + 1] * p.y + m.m[0 + 2] * p.z + m.m[0 + 3],
                m.m[4 + 0] * p.x + m.m[4 + 1] * p.y + m.m[4 + 2] * p.z + m.m[4 + 3],
                m.m[8 + 0] * p.x + m.m[8 + 1] * p.y + m.m[8 + 2] * p.z + m.m[8 + 3]);
}

/**
	 * Transforms the vector (as opposed to a point) v using 
	 * the matrix m .
	 * Treats it as M * v where v is a column vector
	 */
inline Vec3 mulVec(const Matrix4x4& m, const Vec3& v)
{
    return Vec3(m.m[0 + 0] * v.x + m.m[0 + 1] * v.y + m.m[0 + 2] * v.z,
                m.m[4 + 0] * v.x + m.m[4 + 1] * v.y + m.m[4 + 2] * v.z,
                m.m[8 + 0] * v.x + m.m[8 + 1] * v.y + m.m[8 + 2] * v.z);
}

/**
	 * Transposes a matrix
	 */
inline Matrix4x4 transpose(const Matrix4x4& m)
{
    Matrix4x4 result;
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            result.m[y * 4 + x] = m.m[x * 4 + y];
        }
    }
    return result;
}

/**
	 * Sets a transform that scales the objects and translates it
	 * note the translation is in world space regardless of what the scale
	 * is set to
	 */
inline Matrix4x4 scaleTranslation(const Vec3& _scale, const Vec3& _trans)
{
    return translation(_trans) * scale(_scale);
}
/**
	 * Returns a vector in the direction of the smallest
	 * component of v. If there are multiple smallest directions
	 * it can choose any
	 */
Vec3 smallestComponent(const Vec3& v)
{
    float x = fabs(v.x);
    float y = fabs(v.y);
    float z = fabs(v.z);
    if (x < y && x < z)
    {
        return Vec3(1.0f, 0.0f, 0.0f);
    }
    else if (y < z)
    {
        return Vec3(0.0f, 1.0f, 0.0f);
    }
    else
    {
        return Vec3(0.0f, 0.0f, 1.0f);
    }
}

/**
	 * Returns a vector that's orthogonal to v
	 */
inline Vec3 findOrtho(const Vec3& v)
{
    return cross(smallestComponent(v), v);
}

/**
	 * Creates an orientation matrix for a directional light (i.e aligning
	 * the direction with negative y)
	 * @param forward the direction in which the directional light should
	 * point in
	 */
inline Matrix4x4 directionalLightOrientation(Vec3 dir, const Vec3& upHint)
{
    float proj = dot(dir, upHint);
    // Epsilon check to avoid having up vectors parallel with the forward vector
    if (fabs(proj * proj - (dot(dir, dir) * dot(upHint, upHint))) < .001f)
    {
        throw std::runtime_error("Up vector and forward vector parallel or close to parallel");
    }

    Vec3 x = normalize(cross(dir, upHint));
    Vec3 z = normalize(cross(dir, x));
    dir = normalize(dir);
    Matrix4x4 result = identity();
    result.setColumn(x, 0);
    result.setColumn(-dir, 1);
    result.setColumn(z, 2);
    return result;
}
inline Matrix4x4 directionalLightOrientation(Vec3 dir)
{
    Vec3 up = findOrtho(dir);
    return directionalLightOrientation(dir, up);
}

/**
	 * Creates an orientation matrix for a camera (i.e aligning
	 * the forward direction with negative z and up with positive y
	 * @param forward the direction the camera should look in
	 * @param upHint a direction to use as hint for up. Will be forced
	 * to be orthogonal to forward if it isn't
	 */
inline Matrix4x4 cameraOrientation(Vec3 forward, const Vec3& upHint)
{
    float proj = dot(forward, upHint);
    // Epsilon check to avoid having up vectors parallel with the forward vector
    if (fabs(proj * proj - (dot(forward, forward) * dot(upHint, upHint))) < .001f)
    {
        throw std::runtime_error("Up vector and forward vector parallel or close to parallel");
    }
    Vec3 right = normalize(cross(upHint, forward));
    Vec3 up = normalize(cross(forward, right));
    forward = normalize(forward);
    Matrix4x4 result = identity();
    result.setColumn(-right, 0);
    result.setColumn(up, 1);
    result.setColumn(-forward, 2);
    return result;
}

/**
	 * Creates a camera transform.
	 * @param pos the position of the camera in world space
	 * @param forward the direction the camera should look in
	 * @param upHint a direction to use as hint for up. Will fail if
	 * it's paralell with forward
	 */
inline Matrix4x4 setCameraMatrix(const Vec3& pos, const Vec3& forward, const Vec3& upHint)
{
    Matrix4x4 trans = translation(pos);
    Matrix4x4 orient = cameraOrientation(forward, upHint);
    return trans * orient;
}

/**
	 * Creates a spotlight transform.
	 * @param pos the position of the spotlight in world space
	 * @param forward the direction the spotlight should look in.
	 * @param upHint a direction to use as hint for up. Will be fail if
	 * it's paralell with forward
	 */
inline Matrix4x4 setSpotlightMatrix(const Vec3& pos, const Vec3& forward, const Vec3& upHint)
{
    Matrix4x4 trans = translation(pos);
    Matrix4x4 orient = directionalLightOrientation(forward, upHint);
    return trans * orient;
}
inline Matrix4x4 setSpotlightMatrix(const Vec3& pos, const Vec3& forward)
{
    Vec3 up = findOrtho(forward);
    return setSpotlightMatrix(pos, forward, up);
}

/**
	 * Creates an area/window light transform
	 * @param pos the position of the light in world space
	 * @param forward the direction the light should look in
	 * @param upHint a direction to use as hint for up. Will be forced
	 * @param scale the scale of the light source, y will be aligned
	 * with the up-direction of the window light, x perpendicular
	 * to be orthogonal to forward if it isn't
	 */
inline Matrix4x4 setAreaLightMatrix(const Vec3& pos, const Vec3& forward, const Vec3& upHint, const Vec2& scaling)
{
    Matrix4x4 scalemtx = scale(Vec3(scaling.x, 1.0f, scaling.y));
    return setSpotlightMatrix(pos, forward, upHint) * scalemtx;
}

/**
	 * Wrapper for RGB colors
	 * Note, this class relies on the fact that
	 * there are no virtual functions in it.
	 * if you add one, things will go boom
	 */
class ColorRGB : public ILBLinearRGB
{
public:
    ColorRGB(float _r, float _g, float _b)
        : ILBLinearRGB(_r, _g, _b)
    {
    }
};

class ColorRGBA : public ILBLinearRGBA
{
public:
    ColorRGBA()
        : ILBLinearRGBA(0.0f, 0.0f, 0.0f, 1.0f)
    {
    }
    ColorRGBA(float _r, float _g, float _b, float _a)
        : ILBLinearRGBA(_r, _g, _b, _a)
    {
    }
    ColorRGB toColorRGB()
    {
        return ColorRGB(r, g, b);
    }
};

/**
	 * Very basic random float
	 */
inline float frand()
{
    unsigned int r = rand();
    return (float)r / (float)RAND_MAX;
}

ColorRGBA randomRGBA(float amp)
{
    return ColorRGBA(frand() * amp, frand() * amp, frand() * amp, frand() * amp);
}
} // namespace bex


#endif // VECMATH_H
