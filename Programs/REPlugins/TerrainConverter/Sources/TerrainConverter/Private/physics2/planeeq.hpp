/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef PLANEEQ_HPP
#define PLANEEQ_HPP

#include "p2conf.hpp"
#include "mathdef.hpp"
#include "lineeq.hpp"

namespace wg
{
	class Archive;
}

/**
 *	This class is used to represent a plane in 3 dimensional space.
 *
 *	@ingroup Math
 */
class PlaneEq
{
public:
	enum ShouldNormalise
	{
		SHOULD_NORMALISE,
		SHOULD_NOT_NORMALISE
	};

	PlaneEq();
	PlaneEq( const DAVA::Vector3 & normal, const float d );
	PlaneEq( const DAVA::Vector3 & point, const DAVA::Vector3 & normal );
	PlaneEq( const DAVA::Vector3 & v0,
		const DAVA::Vector3 & v1,
		const DAVA::Vector3 & v2,
		ShouldNormalise Normalize = SHOULD_NORMALISE );

	void Serialize(wg::Archive& ar);

	INLINE void init( const DAVA::Vector3 & p0,
		const DAVA::Vector3 & p1,
		const DAVA::Vector3 & p2,
		ShouldNormalise Normalize = SHOULD_NORMALISE );
	INLINE void init( const DAVA::Vector3 & point,
				 const DAVA::Vector3 & normal );

	float distanceTo( const DAVA::Vector3 & point ) const;
	bool isInFrontOf( const DAVA::Vector3 & point ) const;
	bool isInFrontOfExact( const DAVA::Vector3 & point ) const;
	float y( float x, float z ) const;
	
	bool intersectSegment(DAVA::Vector3& point, const DAVA::Vector3& a, const DAVA::Vector3& b) const;

    DAVA::Vector3 intersectRay( const DAVA::Vector3 & source, const DAVA::Vector3 & dir ) const;
	INLINE float
		intersectRayHalf( const DAVA::Vector3 & source, float normalDotDir ) const;
	INLINE float
		intersectRayHalfNoCheck( const DAVA::Vector3 & source, float oneOverNormalDotDir ) const;

	LineEq intersect( const PlaneEq & slice ) const;

	void basis(DAVA::Vector3 & xdir, DAVA::Vector3 & ydir ) const;
    DAVA::Vector3 param( const DAVA::Vector2 & param ) const;
    DAVA::Vector2 project( const DAVA::Vector3 & point ) const;

	const DAVA::Vector3 & normal() const;
	float d() const;

	void setHidden();
	void setAlwaysVisible();

private:
    DAVA::Vector3 normal_;
	float d_;
};

#ifdef CODE_INLINE
	#include "planeeq.ipp"
#endif

#endif // PLANEEQ_HPP
