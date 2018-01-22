/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef BOUNDBOX_HPP
#define BOUNDBOX_HPP

#include "p2conf.hpp"

#include "mathdef.hpp"

/**
 *	BoundingBox, implementation of an axis aligned boundingbox
 */
class BoundingBox
{
public:
	BoundingBox();
	BoundingBox( const DAVA::Vector3 & min, const DAVA::Vector3 & max );

	bool operator==( const BoundingBox& bb ) const;
	bool operator!=( const BoundingBox& bb ) const;

	const DAVA::Vector3 & minBounds() const;
	const DAVA::Vector3 & maxBounds() const;
	void setBounds( const DAVA::Vector3 & min, const DAVA::Vector3 & max );

    float width() const;
    float height() const;
    float depth() const;

	void addYBounds( float y );
	void addBounds( const DAVA::Vector3 & v );
	void addBounds( const BoundingBox & bb );
    void expandSymmetrically( float dx, float dy, float dz );
    void expandSymmetrically( const DAVA::Vector3 & v );
//	void calculateOutcode( const Matrix4 & m ) const;    

	Outcode outcode() const;
	Outcode combinedOutcode() const;
	void outcode( Outcode oc );
	void combinedOutcode( Outcode oc );

//	void transformBy( const Matrix4 & transform );

	bool intersects( const BoundingBox & box ) const;
	bool intersects( const DAVA::Vector3 & v ) const;
	bool intersects( const DAVA::Vector3 & v, float bias ) const;
	bool intersectsRay( const DAVA::Vector3 & origin, const DAVA::Vector3 & dir ) const;
	bool intersectsLine( const DAVA::Vector3 & origin, const DAVA::Vector3 & dest ) const;

	bool clip(DAVA::Vector3 & start, DAVA::Vector3 & extent, float bloat = 0.f ) const;

	float distance( const DAVA::Vector3& point ) const;

	INLINE DAVA::Vector3 centre() const;

	INLINE bool insideOut() const;

private:
    DAVA::Vector3 min_;
    DAVA::Vector3 max_;

	mutable Outcode oc_;
	mutable Outcode combinedOc_;

public:
	static const BoundingBox s_insideOut_;
};

#ifdef CODE_INLINE
#include "boundbox.ipp"
#endif




#endif
/*BoundBox.hpp*/
