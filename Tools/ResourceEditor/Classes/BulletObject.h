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
#include "Scene3d/UserNode.h"

using namespace DAVA;

class BulletObject : public BaseObject
{
public:
    
    BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *_meshNode, const Matrix4 &pWorldTransform);
	BulletObject(Scene * scene, btCollisionWorld *collisionWorld, UserNode *_userNode, const Matrix4 &pWorldTransform);
	BulletObject(Scene * scene, btCollisionWorld *collisionWorld, Entity * _entity, const Matrix4 &pWorldTransform);
	BulletObject(Scene * scene, btCollisionWorld *collisionWorld, Entity * _entity, const AABBox3 &b, const Matrix4 &pWorldTransform);
    ~BulletObject();
	
	void UpdateCollisionObject(void);
	
	inline btCollisionObject * GetCollisionObject(void)
	{
		return collisionObject;
	}

	void Draw(const Matrix4 & worldTransform, MeshInstanceNode * node);

	inline void SetUpdateFlag(bool flag)
	{
		updateFlag = flag;
	}
	
protected:

	void DeleteCollisionObject();	
	void CreateCollisionObject();

	void CreateLightObject(float32 radius);
	void CreateBoxObject();
	void CreateFromEntity();
	void CreateFromAABox();

	Entity * entity;
	btCollisionWorld *collWorld;
	Matrix4 *collisionPartTransform;
	btCollisionObject *collisionObject;
    btTriangleMesh* trimesh;
	btCollisionShape * shape;
	Matrix4 createdWith;
	MeshInstanceNode * meshNode;
	UserNode * userNode;
	AABBox3 * box;
	bool updateFlag;
};

#endif