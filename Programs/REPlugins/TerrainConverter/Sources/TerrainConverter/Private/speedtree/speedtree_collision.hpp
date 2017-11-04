/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SPEEDTREE_COLLISION_HPP
#define SPEEDTREE_COLLISION_HPP

// BW Tech Hearders
#include "../physics2/p2conf.hpp"
#include "../physics2/mathdef.hpp"
#include <memory>
#include <vector>
#include "DAVAEngine.h"

class BSPTree;
class BoundingBox;
class CSpeedTreeRT;
typedef class std::vector<class WorldTriangle> RealWTriangleSet;

typedef std::auto_ptr< BSPTree > BSPTreePtr;

BSPTree* createBSPTree(
    DAVA::Entity * entity,
	const DAVA::String & filename);


//WG: custom speedtree mat kinds
const int SPT_COLLMAT_SOLID  = 71;
const int SPT_COLLMAT_LEAVES = 72;

#endif // SPEEDTREE_COLLISION_HPP
