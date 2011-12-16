/*
 *  BulletObject.h
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#ifndef __BULLET_OBJECT_H__
#define __BULLET_OBJECT_H__

#include "DAVAEngine.h"
#include "bullet/btBulletDynamicsCommon.h"
#include "bullet/btBulletCollisionCommon.h"
#include "DebugNode.h"

using namespace DAVA;

class BulletObject : public BaseObject
{
public:
    
    BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *meshNode, const Matrix4 &pWorldTransform);
    ~BulletObject();
	
	void UpdateCollisionObject(void);


	inline DebugNode * GetDebugNode()
	{
		return debugNode;
	}
	
	inline btCollisionObject * GetCollisionObject(void)
	{
		return collisionObject;
	}
	
protected:

	void CreateShape(MeshInstanceNode *meshNode);
	
	btCollisionWorld *collWorld;
	Matrix4 *collisionPartTransform;
	btCollisionObject *collisionObject;
	btConvexHullShape * shape;
	DebugNode * debugNode;
	std::vector<float32> debugShapes;
};

#endif