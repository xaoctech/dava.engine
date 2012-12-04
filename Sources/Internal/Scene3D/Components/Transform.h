#ifndef __DAVAENGINE_TRANSFORM_H__
#define __DAVAENGINE_TRANSFORM_H__

#include "Base/BaseTypes.h"

namespace DAVA 
{

class SceneNode;

class Transform
{
public:

	Matrix4 * matrix;
	SceneNode * parent;
};

};

#endif //__DAVAENGINE_TRANSFORM_H__
