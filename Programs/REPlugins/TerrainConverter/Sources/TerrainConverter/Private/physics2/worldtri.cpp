/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

/**
*	@file
*/



#include "worldtri.hpp"
#include "worldpoly.hpp"

#ifndef CODE_INLINE
#include "worldtri.ipp"
#endif

#ifdef BW_WORLDTRIANGLE_DEBUG

#include "romp/line_helper.hpp"

bool WorldTriangle::s_debugTriangleDraw_ = false;
Matrix4 WorldTriangle::s_debugTriangleTransform_ = Matrix4::identity;
uint32 WorldTriangle::s_debugTriangleLife_ = 0;
uint32 WorldTriangle::s_debugHitTestFailureLife_ = 0;

// Call this at top of function to set up a transformed triangle. Later macros
// will need this otherwise _dv won't be defined.
#define BW_WT_DEBUG_INIT()\
	Vector3 _dv[3];												\
	if (s_debugTriangleDraw_)									\
{															\
	_dv[0] = s_debugTriangleTransform_.applyPoint(v_[0]);	\
	_dv[1] = s_debugTriangleTransform_.applyPoint(v_[1]);	\
	_dv[2] = s_debugTriangleTransform_.applyPoint(v_[2]);	\
}

// Draw this triangle with a colour
#define BW_WT_DEBUG_DRAW(colour)\
	if (s_debugTriangleDraw_)											\
{																	\
	LineHelper::instance().drawTriangle( \
        _dv[0], _dv[1], _dv[2], (colour), s_debugTriangleLife_);	\
}

// Draw the test prism with a colour
#define BW_WT_DEBUG_DRAW_PRISM(triangle, offset, colour)\
	if (s_debugTriangleDraw_)													\
{																			\
	Vector3 ttA[3], ttB[3];													\
	\
	/* build and draw sides */												\
	for (uint32 i = 0; i < 3; i++)										\
{																		\
	ttA[i] = s_debugTriangleTransform_.applyPoint(triangle.v_[i]);	\
	ttB[i] = ttA[i] + (offset);											\
	LineHelper::instance().drawLine(ttA[i], ttB[i], (colour));		\
}																		\
	\
	/* draw ends */															\
	LineHelper::instance().drawTriangle(ttA[0], ttA[1], ttA[2], (colour));\
	LineHelper::instance().drawTriangle(ttB[0], ttB[1], ttB[2], (colour));\
}

// Draw the test line with a colour
#define BW_WT_DEBUG_DRAW_LINE(start, end, colour)				\
	if (s_debugTriangleDraw_)									\
{															\
	Vector3 a = s_debugTriangleTransform_.applyPoint(start);\
	Vector3 b = s_debugTriangleTransform_.applyPoint(end);\
	LineHelper::instance().drawLine(a, b, (colour), s_debugHitTestFailureLife_);			\
}

#else

// Stub versions of above functions
#define BW_WT_DEBUG_INIT()
#define BW_WT_DEBUG_DRAW(colour)
#define BW_WT_DEBUG_DRAW_PRISM(triangle, offset, colour)
#define BW_WT_DEBUG_DRAW_LINE(start, end, colour)

#endif
#include "mathdef.hpp"

// -----------------------------------------------------------------------------
// Section: WorldTriangle
// -----------------------------------------------------------------------------
/**
*	This method packs collision flags + materialKind into a Flags type.
*/
WorldTriangle::Flags WorldTriangle::packFlags(int collisionFlags, int materialKind)
{
    return collisionFlags |
    (
           (materialKind << TRIANGLE_MATERIALKIND_SHIFT) &
           TRIANGLE_MATERIALKIND_MASK
           );
}

/**
*	This method returns whether the input triangle intersects this triangle.
*/
bool WorldTriangle::intersects(const WorldTriangle& triangle) const
{
    using namespace DAVA;

    // How it works:
    //	Two (non-coplanar) triangles can only intersect in 3D space along an
    //	interval that lies on the line that is the intersection of the two
    //	planes that the triangles lie on. What we do is find the interval of
    //	intersecton each triangle with this line. If these intervals overlap,
    //	the triangles intersect. We only need to test if they overlap in one
    //	coordinate.

    const WorldTriangle& triA = *this;
    const WorldTriangle& triB = triangle;

    PlaneEq planeEqA(triA.v0(), triA.v1(), triA.v2(),
                     PlaneEq::SHOULD_NOT_NORMALISE);

    float dB0 = planeEqA.distanceTo(triB.v0());
    float dB1 = planeEqA.distanceTo(triB.v1());
    float dB2 = planeEqA.distanceTo(triB.v2());

    float dB0dB1 = dB0 * dB1;
    float dB0dB2 = dB0 * dB2;

    // Is triangle B on one side of the plane equation A.
    if (dB0dB1 > 0.f && dB0dB2 > 0.f)
        return false;

    PlaneEq planeEqB(triB.v0(), triB.v1(), triB.v2(),
                     PlaneEq::SHOULD_NOT_NORMALISE);

    float dA0 = planeEqB.distanceTo(triA.v0());
    float dA1 = planeEqB.distanceTo(triA.v1());
    float dA2 = planeEqB.distanceTo(triA.v2());

    float dA0dA1 = dA0 * dA1;
    float dA0dA2 = dA0 * dA2;

    // Is triangle A on one side of the plane equation B.
    if (dA0dA1 > 0.f && dA0dA2 > 0.f)
        return false;

    Vector3 D(planeEqA.normal().CrossProduct(planeEqB.normal()));

    float max = fabsf(D.x);
    int index = X_AXIS;

    float temp = fabsf(D.y);

    if (temp > max)
    {
        max = temp;
        index = Y_AXIS;
    }

    temp = fabsf(D.z);

    if (temp > max)
    {
        index = Z_AXIS;
    }

    Vector3 projA(
    triA.v0().data[index],
    triA.v1().data[index],
    triA.v2().data[index]);

    Vector3 projB(
    triB.v0().data[index],
    triB.v1().data[index],
    triB.v2().data[index]);

    float isect1[2], isect2[2];

    /* compute interval for triangle A */
    COMPUTE_INTERVALS(projA.x, projA.y, projA.z,
                      dA0, dA1, dA2,
                      dA0dA1, dA0dA2,
                      isect1[0], isect1[1]);

    /* compute interval for triangle B */
    COMPUTE_INTERVALS(projB.x, projB.y, projB.z,
                      dB0, dB1, dB2,
                      dB0dB1, dB0dB2,
                      isect2[0], isect2[1]);

    sort(isect1[0], isect1[1]);
    sort(isect2[0], isect2[1]);

    return (isect1[1] >= isect2[0] &&
            isect2[1] >= isect1[0]);
}

/// This constant stores that value that if two floats are within this value,
///	they are considered equal.
const float BW_EPSILON = 0.000001f;

/**
*	This method tests whether the input interval intersects this triangle.
*	The interval is from start to (start + length * dir). If it intersects, the
*	input float is set to the value such that (start + length * dir) is the
*	intersection point.
*/
bool WorldTriangle::intersects(const DAVA::Vector3& start,
                               const DAVA::Vector3& dir,
                               float& length) const
{
    using namespace DAVA;

    BW_WT_DEBUG_INIT();
    BW_WT_DEBUG_DRAW(0xFFFFFFFF);

    // Find the vectors for the edges sharing v0.
    const Vector3 edge1(v1() - v0());
    const Vector3 edge2(v2() - v0());

    // begin calculating determinant - also used to calculate U parameter
    const Vector3 p(dir.CrossProduct(edge2));

    // if determinant is near zero, ray lies in plane of triangle
    float det = edge1.DotProduct(p);

    if (almostZero(det, BW_EPSILON))
    {
        //BW_WT_DEBUG_DRAW_LINE( start, start + dir, 0xFFFF0000)
        return false;
    }

    float inv_det = 1.f / det;

    // calculate distance from vert0 to ray origin
    const Vector3 t(start - v0());

    // calculate U parameter and test bounds
    float u = t.DotProduct(p) * inv_det;

    if (u < 0.f || 1.f < u)
    {
        //BW_WT_DEBUG_DRAW_LINE( start, start + dir, 0xFFFF0000)
        return false;
    }

    // prepare to test V parameter
    const Vector3 q(t.CrossProduct(edge1));

    // calculate V parameter and test bounds
    float v = dir.DotProduct(q) * inv_det;

    if (v < 0.f || 1.f < u + v)
    {
        //BW_WT_DEBUG_DRAW_LINE( start, start + dir, 0xFFFF0000)
        return false;
    }

    // calculate intersection point = start + dist * dir
    float distanceToPlane = edge2.DotProduct(q) * inv_det;

    if (0.f < distanceToPlane && distanceToPlane < length)
    {
        length = distanceToPlane;
        return true;
    }
    else
    {
        BW_WT_DEBUG_DRAW_LINE(start, start + dir, 0xFFFF0000);
        return false;
    }
}

/**
*	Helper function to determine which side a given vector is
*	of another vector. Returns true if the second vector is in
*	the half-plane on the anticlockwise side of the first vector.
*	Colinear vectors are considered to be on that side (returns true)
*/
inline bool inside(const DAVA::Vector2& va, const DAVA::Vector2& vb)
{
    return (va.x * vb.y - va.y * vb.x) >= 0;
}

// WG: helper function for intersection tests
inline bool separatingAxisTest(const DAVA::Vector3& axis, const DAVA::Vector3* a, int anum, const DAVA::Vector3* b, int bnum)
{
    float amin = axis.DotProduct(a[0]);
    float amax = amin;

    for (int i = 1; i < anum; i++)
    {
        float val = axis.DotProduct(a[i]);
        if (val > amax)
            amax = val;
        else if (val < amin)
            amin = val;
    }

    float bmin = axis.DotProduct(b[0]);
    float bmax = bmin;

    for (int i = 1; i < bnum; i++)
    {
        float val = axis.DotProduct(b[i]);
        if (val > bmax)
            bmax = val;
        else if (val < bmin)
            bmin = val;
    }

    return amin <= bmax && amax >= bmin;
}

/**
*	This method returns whether a triangular prism intersects this triangle.
*	The prism is described by a triangle and an offset.
*
*	@param	triangle	The triangle to test against, assumed to be in the same
*						space as this triangle.
*	@param	offset		The offset of end triangle in volume.
*/
// WG implementation
bool WorldTriangle::intersects(const WorldTriangle& triangle,
                               const DAVA::Vector3& offset) const
{
    using namespace DAVA;

    // TODO: optimize
    // first, it is bossible to compute prism projection using triangle and offset projections
    // this require only 4 dots instead of 6
    // then, many of the tests can be customized in more efficient way

    Vector3 prismPoints[6];
    memcpy(prismPoints, triangle.v_, 3 * sizeof(Vector3));

    for (int i = 0; i < 3; i++)
    {
        prismPoints[i + 3] = triangle.v_[i] + offset;
    }

    Vector3 prismTriEdges[3] = { prismPoints[1] - prismPoints[0],
                                 prismPoints[2] - prismPoints[0],
                                 prismPoints[2] - prismPoints[1] };

    if (!separatingAxisTest(prismTriEdges[0].CrossProduct(prismTriEdges[1]), v_, 3, prismPoints, 6))
        return false;

    Vector3 triEdges[3] = { v_[1] - v_[0],
                            v_[2] - v_[0],
                            v_[2] - v_[1] };

    if (!separatingAxisTest(triEdges[0].CrossProduct(triEdges[1]), v_, 3, prismPoints, 6))
        return false;

    for (int i = 0; i < 3; i++)
    {
        if (!separatingAxisTest(offset.CrossProduct(prismTriEdges[i]), v_, 3, prismPoints, 6))
            return false;
    }

    for (int i = 0; i < 3; i++)
    {
        if (!separatingAxisTest(offset.CrossProduct(triEdges[i]), v_, 3, prismPoints, 6))
            return false;

        for (int j = 0; j < 3; j++)
        {
            if (!separatingAxisTest(prismTriEdges[i].CrossProduct(triEdges[j]), v_, 3, prismPoints, 6))
                return false;
        }
    }

    return true;
}

void WorldTriangle::bounce(DAVA::Vector3& v,
                           float elasticity) const
{
    using namespace DAVA;

    Vector3 normal = this->normal();
    normal.Normalize();

    float proj = normal.DotProduct(v);
    v -= (1 + elasticity) * proj * normal;
}

/**
*	This method project the given 3D point to the basis defined by
*	the sides of this triangle. (0,0) is at v0, (1,0) is at v1,
*	(0,1) is at v2, etc.
*
*	Currently it only uses a vertical projection.
*/
DAVA::Vector2 WorldTriangle::project(const DAVA::Vector3& onTri) const
{
    using namespace DAVA;

    // always use a vertical projection
    Vector2 vs(v_[1].x - v_[0].x, v_[1].z - v_[0].z);
    Vector2 vt(v_[2].x - v_[0].x, v_[2].z - v_[0].z);
    Vector2 vp(onTri.x - v_[0].x, onTri.z - v_[0].z);

    // do that funky linear interpolation
    float sXt = vs.x * vt.y - vs.y * vt.x;
    float ls = (vp.x * vt.y - vp.y * vt.x) / sXt;
    float lt = (vp.y * vs.x - vp.x * vs.y) / sXt;

    return Vector2(ls, lt);
}
// worldtri.cpp

#ifdef BW_WORLDTRIANGLE_DEBUG

void WorldTriangle::BeginDraw(const Matrix4& transform,
                              uint32 lifeTime /* = 0  */,
                              uint32 hitFailureLife /* = 0 */)
{
    s_debugTriangleDraw_ = true;
    s_debugTriangleTransform_ = transform;
    s_debugTriangleLife_ = lifeTime;
    s_debugHitTestFailureLife_ = hitFailureLife;
}

void WorldTriangle::EndDraw()
{
    s_debugTriangleDraw_ = false;
}

#endif
