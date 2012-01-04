/*
 *  BulletObject.cpp
 *  SceneEditor
 *
 *  Created by Yury Danilov on 14.12.11
 *  Copyright 2011 DAVA. All rights reserved.
 *
 */

#include "BulletObject.h"
//#include "bullet/btBulletCollisionCommon.h"
#include "bullet/BulletCollision/CollisionShapes/btShapeHull.h"

BulletObject::BulletObject(Scene * scene, btCollisionWorld *collisionWorld, MeshInstanceNode *meshNode, const Matrix4 &pWorldTransform)
{
	collWorld = collisionWorld;
    
    btTransform startTransform;
    startTransform.setIdentity();
    float start_x = 0;
    float start_y = 0;
    float start_z = 0;

    startTransform.setOrigin(btVector3(btScalar(start_x),
                                       btScalar(start_y),
                                       btScalar(start_z)));
    
    collisionObject = new btCollisionObject();
    collisionObject->setWorldTransform(startTransform);
	CreateShape(meshNode);
    collisionObject->setCollisionShape(shape);
    collisionWorld->addCollisionObject(collisionObject);
    
    collisionPartTransform = &((Matrix4&)pWorldTransform);
}

BulletObject::~BulletObject()
{
	if (collisionObject) 
	{
		collWorld->removeCollisionObject(collisionObject);
		SafeDelete(collisionObject);
    }
	collWorld = 0;
	collisionPartTransform = 0;
	SafeDelete(shape);
	SafeDelete(trimesh);
}

void BulletObject::CreateShape(MeshInstanceNode *meshNode)
{    
	trimesh = new btTriangleMesh();
	
	const Vector<StaticMesh*> & meshes = meshNode->GetMeshes();
	const Vector<int32> & indexes = meshNode->GetPolygonGroupIndexes();
	
	uint32 meshesSize = (uint32)meshes.size();

	for (uint32 k = 0; k < meshesSize; ++k)
	{
		PolygonGroup * pg = meshes[k]->GetPolygonGroup(indexes[k]);    
	
		int i;
		for (i = 0; i < pg->indexCount / 3; i++)
		{
			int index0 = pg->indexArray[i*3];
			int index1 = pg->indexArray[i*3+1];
			int index2 = pg->indexArray[i*3+2];
			Vector3 v;
			pg->GetCoord(index0, v);
			v = v * meshNode->GetLocalTransform();
			btVector3 vertex0(v.x, v.y, v.z);
			pg->GetCoord(index1, v);
			v = v * meshNode->GetLocalTransform();
			btVector3 vertex1(v.x, v.y, v.z);
			pg->GetCoord(index2, v);
			v = v * meshNode->GetLocalTransform();
			btVector3 vertex2(v.x, v.y, v.z);
			
			trimesh->addTriangle(vertex0,vertex1,vertex2, false);
		}
	}	
	
	shape = new btBvhTriangleMeshShape(trimesh, true, true);    
}

void BulletObject::UpdateCollisionObject()
{
    btTransform btt;
    btt.setIdentity();
    btt.setFromOpenGLMatrix(collisionPartTransform->data);
    collisionObject->setWorldTransform(btt);
}



